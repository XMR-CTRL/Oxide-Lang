#include "Driver.h"
#include "Lexer.h"
#include "Parser.h"
#include "Sema.h"
#include "IRGen.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <array>
#include <cctype>
#include <set>
#include <algorithm>

#ifdef _WIN32
  #define popen  _popen
  #define pclose _pclose
#endif

namespace {
  std::string readFile(const std::string& path, bool& ok) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { ok = false; return {}; }
    std::stringstream ss;
    ss << f.rdbuf();
    ok = true;
    return ss.str();
  }

  std::string stripExt(const std::string& path) {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) return path;
    return path.substr(0, pos);
  }

  bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f << content;
    return (bool)f;
  }


  std::string pathDir(const std::string& p) {
    auto bs = p.find_last_of('\\');
    auto fs = p.find_last_of('/');
    size_t pos = std::string::npos;
    if (bs != std::string::npos && fs != std::string::npos) pos = std::max(bs, fs);
    else if (bs != std::string::npos) pos = bs;
    else if (fs != std::string::npos) pos = fs;
    if (pos == std::string::npos) return ".";
    return p.substr(0, pos);
  }

  std::string joinPath(const std::string& dir, const std::string& rel) {
    std::string r = rel;
    while (r.size() >= 2 && r[0] == '.' && (r[1] == '/' || r[1] == '\\')) r = r.substr(2);
    if (dir.empty() || dir == ".") return r;
    char sep = (dir.find('\\') != std::string::npos) ? '\\' : '/';
    return dir + sep + r;
  }


}

std::string runtimeC() {
  return std::string(
      "#ifdef _MSC_VER\n"
      "#define _CRT_SECURE_NO_WARNINGS\n"
      "#endif\n"
      "#include <stdio.h>\n"
      "#include <stdlib.h>\n"
      "#include <string.h>\n"
      "#include <math.h>\n"
      "#include <stdint.h>\n"
      "\n"


      "static char* ox_arena_base = 0;\n"
      "static size_t ox_arena_end = 0, ox_arena_cap = 0;\n"
      "static char* ox_arena_alloc(size_t n){\n"
      "  if(!ox_arena_base){ ox_arena_cap = 1<<20; ox_arena_base = (char*)malloc(ox_arena_cap); ox_arena_end = 0; }\n"
      "  size_t need = (n + 8u) & ~(size_t)7u;          /* keep 8-byte alignment */\n"
      "  if(ox_arena_end + need > ox_arena_cap){\n"
      "    while(ox_arena_end + need > ox_arena_cap) ox_arena_cap <<= 1;\n"
      "    char* nb = (char*)malloc(ox_arena_cap);\n"
      "    if(!nb) return ox_arena_base;                 /* oom: reuse old buffer front */\n"
      "    memcpy(nb, ox_arena_base, ox_arena_end);\n"
      "    free(ox_arena_base); ox_arena_base = nb;\n"
      "  }\n"
      "  char* p = ox_arena_base + ox_arena_end; ox_arena_end += need; p[0] = 0; return p;\n"
      "}\n"
      "// Note: ox_arena_alloc doubles scratch room already; callers further NUL-tag the real length.\n"
      "\n"
      "// Every string-returning runtime function never returns NULL: on any\n"


      "static char* ox_str_short(size_t n){ char* p = ox_arena_alloc(n); p[n] = 0; return p; }\n"
      "\n"
      "int ox_puts(const char* s){ if(s) fputs(s, stdout); return 0; }\n"
      "int ox_puti(long long v){ printf(\"%lld\", v); return 0; }\n"
      "int ox_putf(double v){ printf(\"%g\", v); return 0; }\n"
      "int ox_newline(void){ putchar('\\n'); return 0; }\n"
      "int ox_putc(long long c){ putchar((int)(unsigned char)c); return 0; }\n"
      "\n"
      "long long ox_abs_i64(long long v){ return v < 0 ? -v : v; }\n"
      "double ox_sqrt(double v){ return sqrt(v); }\n"
      "long long ox_imin(long long a, long long b){ return a < b ? a : b; }\n"
      "long long ox_imax(long long a, long long b){ return a > b ? a : b; }\n"
      "double ox_fmin2(double a, double b){ return a < b ? a : b; }\n"
      "double ox_fmax2(double a, double b){ return a > b ? a : b; }\n"
      "\n"
      "char* ox_itos(long long v){ char* p = ox_str_short(24); sprintf(p, \"%lld\", v); return p; }\n"
      "long long ox_stoi(const char* s){ return s ? strtoll(s, 0, 10) : 0; }\n"
      "double ox_stod(const char* s){ return s ? strtod(s, 0) : 0.0; }\n"
      "\n"
      "// strconv: double -> str (arena-owned, never null). \"%g\" matches print of f64.\n"
      "char* ox_ftos(double v){ char* p = ox_str_short(32); sprintf(p, \"%g\", v); return p; }\n"
      "// a single char lifted to a 1-byte NUL-terminated string (arena-owned).\n"
      "char* ox_char_str(long long c){ char* p = ox_str_short(1); p[0] = (char)(unsigned char)c; p[1] = 0; return p; }\n"
      "\n"
      "// string comparison: <0 / 0 / >0, like strcmp. Null-safe (treats null as \"\").\n"
      "long long ox_strcmp(const char* a, const char* b){\n"
      "  if(!a) a = \"\"; if(!b) b = \"\";\n"
      "  return (long long)strcmp(a, b);\n"
      "}\n"
      "// length of a NUL-terminated string in bytes (excludes the terminator).\n"
      "long long ox_strlen(const char* s){ return s ? (long long)strlen(s) : 0; }\n"
      "// find first occurrence of char c in s; returns the byte index or -1 if absent.\n"
      "long long ox_strchr(const char* s, long long c){\n"
      "  if(!s) return -1;\n"
      "  const char* p = strchr(s, (int)(unsigned char)c);\n"
      "  return p ? (long long)(p - s) : -1;\n"
      "}\n"
      "// substring s[start..start+len) into a fresh arena string. Out-of-range\n"
      "// start/len are clamped (start past the end yields \"\", negative start\n"
      "// starts at 0, len past the end runs to the terminator). Never returns null.\n"
      "char* ox_substr(const char* s, long long start, long long len){\n"
      "  if(!s) return ox_str_short(0);\n"
      "  long long n = (long long)strlen(s);\n"
      "  if(start < 0) start = 0;\n"
      "  if(start > n) start = n;\n"
      "  if(len < 0) len = 0;\n"
      "  if(start + len > n) len = n - start;\n"
      "  char* p = ox_str_short((size_t)len);\n"
      "  if(len) memcpy(p, s + start, (size_t)len);\n"
      "  p[len] = 0; return p;\n"
      "}\n"
      "\n"
      "// String builder: ox_sb_new() returns an opaque growable buffer; ox_sb_puts\n"
      "// appends a NUL-terminated string; ox_sb_finish() returns an arena-owned\n"
      "// NUL-terminated string and frees the scratch.\n"
      "struct ox_sb { char* data; long long len, cap; };\n"
      "struct ox_sb* ox_sb_new(void){\n"
      "  struct ox_sb* sb = (struct ox_sb*)malloc(sizeof(struct ox_sb));\n"
      "  sb->cap = 16; sb->len = 0; sb->data = (char*)malloc((size_t)sb->cap); sb->data[0] = 0; return sb;\n"
      "}\n"
      "void ox_sb_puts(struct ox_sb* sb, const char* s){\n"
      "  if(!sb || !s) return;\n"
      "  long long n = (long long)strlen(s);\n"
      "  if(sb->len + n + 1 > sb->cap){\n"
      "    while(sb->len + n + 1 > sb->cap) sb->cap <<= 1;\n"
      "    sb->data = (char*)realloc(sb->data, (size_t)sb->cap);\n"
      "  }\n"
      "  memcpy(sb->data + sb->len, s, (size_t)n); sb->len += n; sb->data[sb->len] = 0;\n"
      "}\n"
      "char* ox_sb_finish(struct ox_sb* sb){\n"
      "  if(!sb) return ox_str_short(0);\n"
      "  char* out = ox_str_short((size_t)sb->len);\n"
      "  if(sb->len) memcpy(out, sb->data, (size_t)sb->len);\n"
      "  out[sb->len] = 0;\n"
      "  free(sb->data); free(sb);\n"
      "  return out;\n"
      "}\n"
      "\n"
      "// Bounds-check failure: printed then aborted. The IR emits a call to this\n"
      "// when an array index is out of range (negative or >= length).\n"
      "void ox_bounds_fail(long long idx, long long len){\n"
      "  fprintf(stderr, \"oxide: array index out of bounds (index %lld, length %lld)\\n\", idx, len);\n"
      "  abort();\n"
      "}\n"
      "\n"
      "// io: console read and whole-file reads return NON-null arena buffers.\n"
      "char* ox_read_line(void){\n"
      "  /* Read into a growing scratch buffer, then copy the exact bytes into a\n"
      "     fresh arena string so every returned buffer is compact and NUL-safe. */\n"
      "  size_t cap = 256, len = 0;\n"
      "  char* scratch = (char*)malloc(cap);\n"
      "  int c;\n"
      "  while((c = getchar()) != EOF && c != '\\n'){\n"
      "    if(len == cap){ cap *= 2; char* ns = (char*)realloc(scratch, cap); if(!ns) break; scratch = ns; }\n"
      "    scratch[len++] = (char)c;\n"
      "  }\n"
      "  char* buf = ox_str_short(len);\n"
      "  if(len) memcpy(buf, scratch, len);\n"
      "  buf[len] = 0;\n"
      "  free(scratch);\n"
      "  return buf;\n"
      "}\n"
      "char* ox_read_file(const char* path){\n"
      "  if(!path) return ox_str_short(0);\n"
      "  FILE* f = fopen(path, \"rb\"); if(!f) return ox_str_short(0);\n"
      "  fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);\n"
      "  if(n < 0){ fclose(f); return ox_str_short(0); }\n"
      "  char* buf = ox_str_short((size_t)n); size_t rd = fread(buf, 1, (size_t)n, f); fclose(f);\n"
      "  buf[rd] = 0; return buf;\n"
      "}\n"
      "// file handles are just FILE* stored as a pointer-width integer.\n"
      "long long ox_file_open(const char* path, const char* mode){\n"
      "  if(!path || !mode) return -1; FILE* f = fopen(path, mode); \n"
      "  if(!f) return -1; return (long long)(intptr_t)f;\n"
      "}\n"
      "long long ox_file_close(long long h){ if(h < 0) return -1; return (long long)fclose((FILE*)(intptr_t)h); }\n"
      "char* ox_file_read(long long h){\n"
      "  if(h < 0) return ox_str_short(0); FILE* f = (FILE*)(intptr_t)h;\n"
      "  fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);\n"
      "  if(n < 0) return ox_str_short(0); char* buf = ox_str_short((size_t)n); size_t rd = fread(buf,1,(size_t)n,f); buf[rd]=0; return buf;\n"
      "}\n"
      "long long ox_file_write(long long h, const char* s){ if(h < 0 || !s) return -1; return (long long)fputs(s, (FILE*)(intptr_t)h); }\n"
      "int ox_file_exists(const char* path){ if(!path) return 0; FILE* f = fopen(path,\"rb\"); if(!f) return 0; fclose(f); return 1; }\n"
      "\n"
      "// ----------------------------------------------------------------\n"
      "// Dynamic (growable) array runtime. A handle is a pointer to a\n"
      "// header holding { len, cap, data }; elements are typed in host C, so the\n"
      "// name suffix selects the concrete element type (i64 f64 i1 str).\n"
      "// ----------------------------------------------------------------\n"
      "struct ox_vec { long long len, cap; void* data; };\n"
      "static struct ox_vec* ox_vec_check(void* h, const char* who){\n"
      "  if(!h){ fprintf(stderr, \"oxide: %s on a null vec\\n\", who); abort(); }\n"
      "  return (struct ox_vec*)h;\n"
      "}\n"
      "long long ox_vec_len(void* h){ return ox_vec_check(h,\"len\")->len; }\n"
      "static void ox_vec_grow(struct ox_vec* v, size_t esz){\n"
      "  if(v->len < v->cap) return;\n"
      "  long long nc = v->cap ? v->cap*2 : 8;\n"
      "  v->data = realloc(v->data, (size_t)nc * esz);\n"
      "  if(!v->data){ fprintf(stderr, \"oxide: vec out of memory\\n\"); abort(); }\n"
      "  v->cap = nc;\n"
      "}\n"
      "// The four element-kind typos define new/push/get/set/print. As the\n"
      "// compiler only references the suffixes the program uses, we always\n"
      "// provide all four to simplify the C side; linkers keep the unused ones.\n"
      "#define OX_VEC_KIND(SUF, ETYPE, FMT) \\\n"
      "  void* ox_vec_new_##SUF(void){ struct ox_vec* v = (struct ox_vec*)calloc(1,sizeof(*v)); return v; } \\\n"
      "  void ox_vec_push_##SUF(void* h, ETYPE x){ struct ox_vec* v = ox_vec_check(h,\"push\"); ox_vec_grow(v, sizeof(ETYPE)); ((ETYPE*)v->data)[v->len++] = x; } \\\n"
      "  ETYPE ox_vec_get_##SUF(void* h, long long i){ struct ox_vec* v = ox_vec_check(h,\"get\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } return ((ETYPE*)v->data)[i]; } \\\n"
      "  void ox_vec_set_##SUF(void* h, long long i, ETYPE x){ struct ox_vec* v = ox_vec_check(h,\"set\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } ((ETYPE*)v->data)[i] = x; } \\\n"
      "  void ox_vec_print_##SUF(void* h){ struct ox_vec* v = ox_vec_check(h,\"print\"); putchar('['); for(long long i=0;i<v->len;i++){ if(i) fputs(\", \", stdout); printf(FMT, ((ETYPE*)v->data)[i]); } putchar(']'); }\n"
      "OX_VEC_KIND(i64, long long, \"%lld\")\n"
      "OX_VEC_KIND(f64, double, \"%g\")\n"
      "OX_VEC_KIND(i1, int, \"%d\")\n"
      "// char elements are i8 (printed as a character, not a number).\n"
      "void* ox_vec_new_i8(void){ struct ox_vec* v = (struct ox_vec*)calloc(1,sizeof(*v)); return v; }\n"
      "void ox_vec_push_i8(void* h, char x){ struct ox_vec* v = ox_vec_check(h,\"push\"); ox_vec_grow(v, sizeof(char)); ((char*)v->data)[v->len++] = x; }\n"
      "char ox_vec_get_i8(void* h, long long i){ struct ox_vec* v = ox_vec_check(h,\"get\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } return ((char*)v->data)[i]; }\n"
      "void ox_vec_set_i8(void* h, long long i, char x){ struct ox_vec* v = ox_vec_check(h,\"set\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } ((char*)v->data)[i] = x; }\n"
      "void ox_vec_print_i8(void* h){ struct ox_vec* v = ox_vec_check(h,\"print\"); putchar('['); for(long long i=0;i<v->len;i++){ if(i) fputs(\", \", stdout); putchar((int)((unsigned char)((char*)v->data)[i])); } putchar(']'); }\n"
      "// str elements are i8* (NUL-terminated), printed with fputs.\n"
      "void* ox_vec_new_str(void){ struct ox_vec* v = (struct ox_vec*)calloc(1,sizeof(*v)); return v; }\n"
      "void ox_vec_push_str(void* h, char* x){ struct ox_vec* v = ox_vec_check(h,\"push\"); ox_vec_grow(v, sizeof(char*)); ((char**)v->data)[v->len++] = x; }\n"
      "char* ox_vec_get_str(void* h, long long i){ struct ox_vec* v = ox_vec_check(h,\"get\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } return ((char**)v->data)[i]; }\n"
      "void ox_vec_set_str(void* h, long long i, char* x){ struct ox_vec* v = ox_vec_check(h,\"set\"); if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); } ((char**)v->data)[i] = x; }\n"
      "void ox_vec_print_str(void* h){ struct ox_vec* v = ox_vec_check(h,\"print\"); putchar('['); for(long long i=0;i<v->len;i++){ if(i) fputs(\", \", stdout); fputs(((char**)v->data)[i] ? ((char**)v->data)[i] : \"\", stdout); } putchar(']'); }\n"
      "\n"
      "// ----------------------------------------------------------------\n"
      "// sort: in-place ascending sort of a vec's elements. Two entry points:\n"
      "//  ox_sort_<suffix> for the typed fast-path vecs (i64/f64/i1/i8/str), and a\n"
      "//  generic ox_sort_blob(h, esz, kind) for blob vecs keyed on the element\n"
      "//  byte width + a category tag (0 signed int, 1 unsigned int, 2 float,\n"
      "//  3 pointer/str). Struct/aggregate elements can't be sorted this way.\n"
      "// ----------------------------------------------------------------\n"
      "static int ox_cmp_i64(const void* a, const void* b){ long long x=*(const long long*)a, y=*(const long long*)b; return (x>y)-(x<y); }\n"
      "static int ox_cmp_f64(const void* a, const void* b){ double x=*(const double*)a, y=*(const double*)b; return (x>y)-(x<y); }\n"
      "static int ox_cmp_i1 (const void* a, const void* b){ int x=*(const int*)a, y=*(const int*)b; return (x>y)-(x<y); }\n"
      "static int ox_cmp_i8 (const void* a, const void* b){ unsigned char x=*(const unsigned char*)a, y=*(const unsigned char*)b; return (int)x-(int)y; }\n"
      "static int ox_cmp_str(const void* a, const void* b){ const char* x=*(const char* const*)a; const char* y=*(const char* const*)b; if(!x) x=\"\"; if(!y) y=\"\"; return strcmp(x,y); }\n"
      "void ox_sort_i64(void* h){ struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len>1) qsort(v->data, (size_t)v->len, sizeof(long long), ox_cmp_i64); }\n"
      "void ox_sort_f64(void* h){ struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len>1) qsort(v->data, (size_t)v->len, sizeof(double), ox_cmp_f64); }\n"
      "void ox_sort_i1 (void* h){ struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len>1) qsort(v->data, (size_t)v->len, sizeof(int), ox_cmp_i1); }\n"
      "void ox_sort_i8 (void* h){ struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len>1) qsort(v->data, (size_t)v->len, sizeof(char), ox_cmp_i8); }\n"
      "void ox_sort_str(void* h){ struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len>1) qsort(v->data, (size_t)v->len, sizeof(char*), ox_cmp_str); }\n"
      "// generic blob sort: kind selects a signed/unsigned/float/ptr comparator;\n"
      "// the element width selects the exact C type read at each slot so small\n"
      "// ints and f32 widen correctly (a 4-byte read into an 8-byte value would\n"
      "// misread f32 bit patterns, so dispatch on esz).\n"
      "static int ox_sort_blob_kind; static size_t ox_sort_blob_esz;\n"
      "static int ox_cmp_blob(const void* a, const void* b){\n"
      "  if(ox_sort_blob_kind==3){ const char* x=*(const char* const*)a; const char* y=*(const char* const*)b; if(!x) x=\"\"; if(!y) y=\"\"; return strcmp(x,y); }\n"
      "  if(ox_sort_blob_kind==2){\n"
      "    if(ox_sort_blob_esz==4){ float x=*(const float*)a, y=*(const float*)b; return (x>y)-(x<y); }\n"
      "    double x=*(const double*)a, y=*(const double*)b; return (x>y)-(x<y);\n"
      "  }\n"
      "  // integer kinds: read exactly esz bytes (zero-init, then memcpy) and, for\n"
      "  // signed, sign-extend from the top read bit so i8/i16/i32 compare right.\n"
      "  unsigned long long ua=0, ub=0;\n"
      "  memcpy(&ua, a, ox_sort_blob_esz); memcpy(&ub, b, ox_sort_blob_esz);\n"
      "  if(ox_sort_blob_kind==0){\n"
      "    long long sa, sb;\n"
      "    if(ox_sort_blob_esz==1){ sa=(signed char)ua; sb=(signed char)ub; }\n"
      "    else if(ox_sort_blob_esz==2){ sa=(short)ua; sb=(short)ub; }\n"
      "    else if(ox_sort_blob_esz==4){ sa=(int)ua; sb=(int)ub; }\n"
      "    else { sa=(long long)ua; sb=(long long)ub; }\n"
      "    return (sa>sb)-(sa<sb);\n"
      "  }\n"
      "  return (ua>ub)-(ua<ub);\n"
      "}\n"
      "void ox_sort_blob(void* h, long long esz, long long kind){\n"
      "  struct ox_vec* v = ox_vec_check(h,\"sort\"); if(v->len<2) return;\n"
      "  ox_sort_blob_esz = (size_t)esz; ox_sort_blob_kind = (int)kind;\n"
      "  qsort(v->data, (size_t)v->len, (size_t)esz, ox_cmp_blob);\n"
      "}\n"
      "\n"
      "// (structs, fixed arrays, nested vecs) via memcpy in/out. The element\n"
      "// byte size `esz` is fixed at construction and passed to each operation.\n"
      "// `ox_vec_blob_ptr` yields a pointer to slot i so the compiler can read or\n"
      "// write a struct/aggregate element in place (GEP into the data buffer).\n"
      "// For pointer-shaped elements (the fast i8* path) the compiler uses str/\n"
      "// i64 accessors; blobs only kick in for non-pointer-sized aggregate slots.\n"
      "// ----------------------------------------------------------------\n"
      "static void ox_vec_blob_grow(struct ox_vec* v, size_t esz){\n"
      "  if(v->len < v->cap) return;\n"
      "  long long nc = v->cap ? v->cap*2 : 8;\n"
      "  v->data = realloc(v->data, (size_t)nc * esz);\n"
      "  if(!v->data){ fprintf(stderr, \"oxide: vec out of memory\\n\"); abort(); }\n"
      "  v->cap = nc;\n"
      "}\n"
      "void* ox_vec_blob_new(long long esz){\n"
      "  struct ox_vec* v = (struct ox_vec*)calloc(1,sizeof(*v));\n"
      "  /* stash esz in the otherwise-unused high half? no: keep esz explicit on\n"
      "     every call so the header stays the same as the typed vec kinds. */\n"
      "  (void)esz; return v;\n"
      "}\n"
      "void ox_vec_blob_push(void* h, long long esz, void* src){\n"
      "  struct ox_vec* v = ox_vec_check(h,\"blob_push\");\n"
      "  ox_vec_blob_grow(v, (size_t)esz);\n"
      "  memcpy((char*)v->data + v->len*(long long)esz, src, (size_t)esz);\n"
      "  v->len++;\n"
      "}\n"
      "void ox_vec_blob_get(void* h, long long i, long long esz, void* dst){\n"
      "  struct ox_vec* v = ox_vec_check(h,\"blob_get\");\n"
      "  if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); }\n"
      "  memcpy(dst, (char*)v->data + i*(long long)esz, (size_t)esz);\n"
      "}\n"
      "void ox_vec_blob_set(void* h, long long i, long long esz, void* src){\n"
      "  struct ox_vec* v = ox_vec_check(h,\"blob_set\");\n"
      "  if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); }\n"
      "  memcpy((char*)v->data + i*(long long)esz, src, (size_t)esz);\n"
      "}\n"
      "// in-place pointer to slot i (for field access / nested indexing into a\n"
      "// blob element without a round-trip copy).\n"
      "void* ox_vec_blob_ptr(void* h, long long i, long long esz){\n"
      "  struct ox_vec* v = ox_vec_check(h,\"blob_ptr\");\n"
      "  if(i<0||i>=v->len){ fprintf(stderr, \"oxide: vec index out of bounds (%lld, len %lld)\\n\", i, v->len); abort(); }\n"
      "  return (char*)v->data + i*(long long)esz;\n"
      "}\n"
      "\n"
      "// ----------------------------------------------------------------\n"
      "// map[K,V]: an ordered associative array (sorted ascending by key) with\n"
      "// O(log n) lookup and O(n) insert/delete. One generic family, keyed on\n"
      "// the key byte width (kw), value byte width (vw), and a key category tag\n"
      "// (kk: 0 signed int, 1 unsigned, 2 float, 3 str/ptr) — the same category\n"
      "// scheme as sort. Keys are stored sorted; values live in a parallel array.\n"
      "// ----------------------------------------------------------------\n"
      "struct ox_map { long long len, cap; char* keys; char* vals; long long kw, vw, kk; };\n"
      "static struct ox_map* ox_map_check(void* h, const char* who){\n"
      "  if(!h){ fprintf(stderr, \"oxide: %s on a null map\\n\", who); abort(); }\n"
      "  return (struct ox_map*)h;\n"
      "}\n"
      "void* ox_map_new(long long kw, long long vw, long long kk){\n"
      "  struct ox_map* m = (struct ox_map*)calloc(1,sizeof(*m));\n"
      "  m->kw = kw; m->vw = vw; m->kk = kk; m->cap = 0; return m;\n"
      "}\n"
      "long long ox_map_len(void* h){ return ox_map_check(h,\"len\")->len; }\n"
      "// compare a key buffer against a stored key slot, using the category kind\n"
      "// so signed/unsigned/float order correctly across all int widths.\n"
      "static int ox_map_kcmp(const struct ox_map* m, const char* a, const char* b){\n"
      "  if(m->kk==3){ const char* x=*(const char* const*)a; const char* y=*(const char* const*)b; if(!x) x=\"\"; if(!y) y=\"\"; return strcmp(x,y); }\n"
      "  if(m->kk==2){\n"
      "    if(m->kw==4){ float x=*(const float*)a, y=*(const float*)b; return (x>y)-(x<y); }\n"
      "    double x=*(const double*)a, y=*(const double*)b; return (x>y)-(x<y);\n"
      "  }\n"
      "  unsigned long long ua=0, ub=0; memcpy(&ua,a,(size_t)m->kw); memcpy(&ub,b,(size_t)m->kw);\n"
      "  if(m->kk==0){\n"
      "    long long sa, sb;\n"
      "    if(m->kw==1){ sa=(signed char)ua; sb=(signed char)ub; }\n"
      "    else if(m->kw==2){ sa=(short)ua; sb=(short)ub; }\n"
      "    else if(m->kw==4){ sa=(int)ua; sb=(int)ub; }\n"
      "    else { sa=(long long)ua; sb=(long long)ub; }\n"
      "    return (sa>sb)-(sa<sb);\n"
      "  }\n"
      "  return (ua>ub)-(ua<ub);\n"
      "}\n"
      "// lower_bound: the first index whose key is >= q. Returns len if all are < q.\n"
      "static long long ox_map_lb(struct ox_map* m, const char* q){\n"
      "  long long lo=0, hi=m->len;\n"
      "  while(lo<hi){ long long mid=lo+(hi-lo)/2; if(ox_map_kcmp(m, m->keys + mid*m->kw, q) < 0) lo=mid+1; else hi=mid; }\n"
      "  return lo;\n"
      "}\n"
      "static void ox_map_grow(struct ox_map* m){\n"
      "  if(m->len < m->cap) return;\n"
      "  long long nc = m->cap ? m->cap*2 : 8;\n"
      "  m->keys = (char*)realloc(m->keys, (size_t)nc * (size_t)m->kw);\n"
      "  m->vals = (char*)realloc(m->vals, (size_t)nc * (size_t)m->vw);\n"
      "  m->cap = nc;\n"
      "}\n"
      "// set(m, kptr, vptr): insert-or-replace. The key/value live in caller\n"
      "// scratch (bytewise copies; widened by kw/vw on the Oxide side first).\n"
      "void ox_map_set(void* h, const void* kp, const void* vp){\n"
      "  struct ox_map* m = ox_map_check(h,\"set\");\n"
      "  const char* k = (const char*)kp;\n"
      "  long long i = ox_map_lb(m, k);\n"
      "  if(i < m->len && ox_map_kcmp(m, m->keys + i*m->kw, k) == 0){\n"
      "    memcpy(m->vals + i*m->vw, vp, (size_t)m->vw); return;   // replace value\n"
      "  }\n"
      "  ox_map_grow(m);\n"
      "  memmove(m->keys + (i+1)*m->kw, m->keys + i*m->kw, (size_t)(m->len - i) * (size_t)m->kw);\n"
      "  memmove(m->vals + (i+1)*m->vw, m->vals + i*m->vw, (size_t)(m->len - i) * (size_t)m->vw);\n"
      "  memcpy(m->keys + i*m->kw, k, (size_t)m->kw);\n"
      "  memcpy(m->vals + i*m->vw, vp, (size_t)m->vw);\n"
      "  m->len++;\n"
      "}\n"
      "// get(m, kptr, vptr): if present, copy the value into *vptr and return 1;\n"
      "// else zero-fill *vptr and return 0 (so a missing key reads as zero).\n"
      "long long ox_map_get(void* h, const void* kp, void* vp){\n"
      "  struct ox_map* m = ox_map_check(h,\"get\");\n"
      "  long long i = ox_map_lb(m, (const char*)kp);\n"
      "  if(i < m->len && ox_map_kcmp(m, m->keys + i*m->kw, (const char*)kp) == 0){\n"
      "    memcpy(vp, m->vals + i*m->vw, (size_t)m->vw); return 1;\n"
      "  }\n"
      "  memset(vp, 0, (size_t)m->vw); return 0;\n"
      "}\n"
      "// contains(m, kptr): 1 if the key is present, else 0.\n"
      "long long ox_map_contains(void* h, const void* kp){\n"
      "  struct ox_map* m = ox_map_check(h,\"contains\");\n"
      "  long long i = ox_map_lb(m, (const char*)kp);\n"
      "  return (i < m->len && ox_map_kcmp(m, m->keys + i*m->kw, (const char*)kp) == 0) ? 1 : 0;\n"
      "}\n"
      "// key pointer at sorted index i (for `map_keys`, which copies out a vec).\n"
      "void* ox_map_key_ptr(void* h, long long i){\n"
      "  struct ox_map* m = ox_map_check(h,\"key_ptr\");\n"
      "  if(i<0||i>=m->len){ fprintf(stderr, \"oxide: map key index out of bounds (%lld, len %lld)\\n\", i, m->len); abort(); }\n"
      "  return m->keys + i*(long long)m->kw;\n"
      "}\n"
      "\n"
      "// ----------------------------------------------------------------\n"
      "// set[T] (std::set): a sorted-unique array of keys (no values). Same\n"
      "// comparator + sorted-insert machinery as ox_map, minus the value side.\n"
      "// ----------------------------------------------------------------\n"
      "struct ox_set { long long len, cap; char* data; long long kw, kk; };\n"
      "static struct ox_set* ox_set_check(void* h, const char* who){\n"
      "  if(!h){ fprintf(stderr, \"oxide: %s on a null set\\n\", who); abort(); }\n"
      "  return (struct ox_set*)h;\n"
      "}\n"
      "void* ox_set_new(long long kw, long long kk){\n"
      "  struct ox_set* s = (struct ox_set*)calloc(1,sizeof(*s));\n"
      "  s->kw = kw; s->kk = kk; return s;\n"
      "}\n"
      "long long ox_set_len(void* h){ return ox_set_check(h,\"len\")->len; }\n"
      "static int ox_set_kcmp(const struct ox_set* s, const char* a, const char* b){\n"
      "  if(s->kk==3){ const char* x=*(const char* const*)a; const char* y=*(const char* const*)b; if(!x) x=\"\"; if(!y) y=\"\"; return strcmp(x,y); }\n"
      "  if(s->kk==2){\n"
      "    if(s->kw==4){ float x=*(const float*)a, y=*(const float*)b; return (x>y)-(x<y); }\n"
      "    double x=*(const double*)a, y=*(const double*)b; return (x>y)-(x<y);\n"
      "  }\n"
      "  unsigned long long ua=0, ub=0; memcpy(&ua,a,(size_t)s->kw); memcpy(&ub,b,(size_t)s->kw);\n"
      "  if(s->kk==0){\n"
      "    long long sa, sb;\n"
      "    if(s->kw==1){ sa=(signed char)ua; sb=(signed char)ub; }\n"
      "    else if(s->kw==2){ sa=(short)ua; sb=(short)ub; }\n"
      "    else if(s->kw==4){ sa=(int)ua; sb=(int)ub; }\n"
      "    else { sa=(long long)ua; sb=(long long)ub; }\n"
      "    return (sa>sb)-(sa<sb);\n"
      "  }\n"
      "  return (ua>ub)-(ua<ub);\n"
      "}\n"
      "static long long ox_set_lb(struct ox_set* s, const char* q){\n"
      "  long long lo=0, hi=s->len;\n"
      "  while(lo<hi){ long long mid=lo+(hi-lo)/2; if(ox_set_kcmp(s, s->data + mid*s->kw, q) < 0) lo=mid+1; else hi=mid; }\n"
      "  return lo;\n"
      "}\n"
      "static void ox_set_grow(struct ox_set* s){\n"
      "  if(s->len < s->cap) return;\n"
      "  long long nc = s->cap ? s->cap*2 : 8;\n"
      "  s->data = (char*)realloc(s->data, (size_t)nc * (size_t)s->kw);\n"
      "  s->cap = nc;\n"
      "}\n"
      "// insert(elem): add if absent (std::set::insert). Idempotent: inserting an\n"
      "// already-present element is a no-op.\n"
      "void ox_set_insert(void* h, const void* ep){\n"
      "  struct ox_set* s = ox_set_check(h,\"insert\");\n"
      "  const char* k = (const char*)ep;\n"
      "  long long i = ox_set_lb(s, k);\n"
      "  if(i < s->len && ox_set_kcmp(s, s->data + i*s->kw, k) == 0) return;\n"
      "  ox_set_grow(s);\n"
      "  memmove(s->data + (i+1)*s->kw, s->data + i*s->kw, (size_t)(s->len - i) * (size_t)s->kw);\n"
      "  memcpy(s->data + i*s->kw, k, (size_t)s->kw);\n"
      "  s->len++;\n"
      "}\n"
      "// remove(elem): erase if present (std::set::erase). Idempotent.\n"
      "void ox_set_remove(void* h, const void* ep){\n"
      "  struct ox_set* s = ox_set_check(h,\"remove\");\n"
      "  const char* k = (const char*)ep;\n"
      "  long long i = ox_set_lb(s, k);\n"
      "  if(i < s->len && ox_set_kcmp(s, s->data + i*s->kw, k) == 0){\n"
      "    memmove(s->data + i*s->kw, s->data + (i+1)*s->kw, (size_t)(s->len - i - 1) * (size_t)s->kw);\n"
      "    s->len--;\n"
      "  }\n"
      "}\n"
      "// contains(elem): 1 if present, else 0.\n"
      "long long ox_set_contains(void* h, const void* ep){\n"
      "  struct ox_set* s = ox_set_check(h,\"contains\");\n"
      "  long long i = ox_set_lb(s, (const char*)ep);\n"
      "  return (i < s->len && ox_set_kcmp(s, s->data + i*s->kw, (const char*)ep) == 0) ? 1 : 0;\n"
      "}\n"
      "// element pointer at sorted index i (for `set_to_vec` / iteration).\n"
      "void* ox_set_ptr(void* h, long long i){\n"
      "  struct ox_set* s = ox_set_check(h,\"ptr\");\n"
      "  if(i<0||i>=s->len){ fprintf(stderr, \"oxide: set index out of bounds (%lld, len %lld)\\n\", i, s->len); abort(); }\n"
      "  return s->data + i*(long long)s->kw;\n"
      "}\n"
      "\n"
      "long long oxide_main(void);\n"
      "int main(void){ int r = (int)oxide_main(); if(ox_arena_base) free(ox_arena_base); return r; }\n"
    );
}


std::vector<Token> Driver::resolveImports(std::vector<Token>& toks, const std::string& dir,
                                          std::vector<CompileError>& cerrs) {
  std::vector<Token> out;
  out.reserve(toks.size());
  for (size_t i = 0; i < toks.size(); i++) {

    if (i + 2 < toks.size() &&
        toks[i].kind == Tok::ident && toks[i].text == "import" &&
        toks[i + 1].kind == Tok::str_lit && toks[i + 2].kind == Tok::semicolon) {
      std::string rel = toks[i + 1].text;
      std::string path = joinPath(dir, rel);
      std::string canon = path;

      for (auto& c : canon) if (c == '\\') c = '/';
      if (importVisited_.count(canon)) { i += 2; continue; }
      importVisited_.insert(canon);

      bool ok = false;
      std::string src = readFile(path, ok);
      if (!ok) {
        cerrs.push_back({"io", "cannot import '" + rel + "' (file not found: " + path + ")",
                          toks[i].line, toks[i].col});
        i += 2; continue;
      }
      std::vector<Token> itoks;
      std::vector<LexError> lerr;
      Lexer lex(src);
      lex.lex(itoks, lerr);
      for (auto& e : lerr) cerrs.push_back({"lex", e.msg, e.line, e.col});
      if (!lerr.empty()) { i += 2; continue; }

      itoks = resolveImports(itoks, pathDir(path), cerrs);
      for (auto& t : itoks) out.push_back(std::move(t));
      i += 2;
      continue;
    }
    out.push_back(std::move(toks[i]));
  }
  return out;
}


static std::string linkFlagsFor(const std::string& clang, const Options& opt) {
  std::string out;
  bool msvc = (clang == "clang-cl" || clang == "cl");
  for (const auto& lib : opt.linkLibs) {
    if (lib.empty()) continue;
    if (msvc) {

      std::string s = lib;
      if (s.size() < 4 || s.substr(s.size() - 4) != ".lib") s += ".lib";
      out += " " + s;
    } else {
      out += " -l" + lib;
    }
  }
  for (const auto& f : opt.linkFlags) {


    out += " " + f;
  }
  return out;
}

std::string Driver::findTool(const std::vector<const char*>& names) {
  for (const char* n : names) {
    std::string probe;
#ifdef _WIN32
    probe = std::string("where ") + n + " >nul 2>nul";
#else
    probe = std::string("command -v ") + n + " >/dev/null 2>&1";
#endif
    FILE* p = popen(probe.c_str(), "r");
    if (!p) continue;
    int rc = pclose(p);
    if (rc == 0) return n;
  }
  return "";
}

int Driver::runCmd(const std::string& cmd) {
  return std::system(cmd.c_str());
}

std::string Driver::renameMain(const std::string& ir) {
  std::string out;
  out.reserve(ir.size());
  size_t i = 0;
  while (i < ir.size()) {
    if (ir.compare(i, 5, "@main") == 0) {
      char nx = (i + 5 < ir.size()) ? ir[i + 5] : '(';
      if (!(isalnum((unsigned char)nx) || nx == '_')) {
        out += "@oxide_main"; i += 5; continue;
      }
    }
    out += ir[i++];
  }
  return out;
}

bool Driver::run(const Options& opt) {
  bool ok = false;
  std::string src = readFile(opt.input, ok);
  if (!ok) {
    errs.push_back({"io", "cannot open input file '" + opt.input + "'", 0, 0});
    return false;
  }

  srcLines_.clear();
  {
    std::string cur;
    for (char ch : src) {
      if (ch == '\n') { srcLines_.push_back(cur); cur.clear(); }
      else cur += ch;
    }
    srcLines_.push_back(cur);
  }

  std::vector<Token> toks;
  std::vector<LexError> lexErrs;
  Lexer lex(src);
  lex.lex(toks, lexErrs);
  for (auto& e : lexErrs) errs.push_back({"lex", e.msg, e.line, e.col});
  if (!lexErrs.empty()) return false;


  importVisited_.clear();
  std::string baseDir = pathDir(opt.input);
  {
    std::vector<CompileError> impErrs;
    toks = resolveImports(toks, baseDir, impErrs);
    for (auto& e : impErrs) errs.push_back(e);
    if (!impErrs.empty()) return false;
  }

  std::vector<ParseError> parseErrs;
  Parser parser(std::move(toks), parseErrs);
  auto prog = parser.parseProgram();
  for (auto& e : parseErrs) errs.push_back({"parse", e.msg, e.line, e.col});
  if (!parseErrs.empty()) return false;

  Sema sema;


  sema.requireMain = (opt.action == Action::run || opt.action == Action::exe)
                     && !opt.freestanding;
  sema.freestanding = opt.freestanding;
  sema.check(*prog);
  for (auto& e : sema.errs) errs.push_back({"sema", e.msg, e.line, e.col, e.hint});
  if (!sema.errs.empty()) return false;

  IRGen irgen(sema);
  irgen.setTargetTriple(opt.targetTriple);
  irgen.generate(*prog);
  ir_ = irgen.takeIR();

  switch (opt.action) {
    case Action::run: return doRun(opt);
    case Action::emit: return doEmit();
    case Action::build: return doBuild(opt, opt.output.empty() ? stripExt(opt.input) + ".o" : opt.output);
    case Action::exe: return doExe(opt, opt.output.empty() ? stripExt(opt.input) + ".exe" : opt.output);
  }
  return false;
}


void Driver::printErrors(const std::string& file) const {
  for (const auto& e : errs) {
    std::printf("error[%s]: %s\n", e.stage.c_str(), e.msg.c_str());
    if (e.line >= 1 && (size_t)e.line <= srcLines_.size()) {
      const std::string& line = srcLines_[e.line - 1];
      char numbuf[32];
      std::snprintf(numbuf, sizeof(numbuf), "%d", e.line);
      std::printf(" %s | %s\n", numbuf, line.c_str());
      std::string pad(std::string(numbuf).size(), ' ');

      int col = e.col > 0 ? e.col : (int)(line.find_first_not_of(" \t")) + 1;
      if (col <= 0) col = 1;
      std::string caret(pad.size() + 3 + (size_t)(col - 1), ' ');
      caret += "^";
      std::printf("%s\n", caret.c_str());
      if (!e.hint.empty()) std::printf("      = note: %s\n", e.hint.c_str());
    } else {
      std::printf("  (no source location)\n");
    }
    if (!e.hint.empty() && e.line < 1) std::printf("  hint: %s\n", e.hint.c_str());
  }
  std::printf("  --> %s\n\n", file.c_str());
}

bool Driver::doEmit() {
  std::fputs(ir_.c_str(), stdout);
  return true;
}


static std::string tempDir() {
  const char* t = std::getenv("TEMP");
  if (!t) t = std::getenv("TMP");
  if (!t) t = ".";
  return t;
}

bool Driver::doBuild(const Options& opt, const std::string& outPath) {


  std::string dir = tempDir();
  std::string ll = dir + "\\oxide_out.ll";
  std::string irOut = opt.freestanding ? ir_ : renameMain(ir_);
  if (!writeFile(ll, irOut)) {
    errs.push_back({"emit", "cannot write temp ir", 0, 0});
    return false;
  }

  std::string clang = findTool({"clang", "clang-cl"});
  if (clang.empty()) clang = "clang";
  std::string cmd;


  std::string extra = opt.freestanding ? " -ffreestanding" : "";

  std::string nw = " -Wno-override-module";
  if (clang == "clang-cl") {
    cmd = clang + " -c -fuse-ld=lld" + extra + nw + " \"" + ll + "\" -o \"" + outPath + "\"";
  } else {
    cmd = clang + " -c" + extra + nw + " \"" + ll + "\" -o \"" + outPath + "\"";
  }
  int rc = runCmd(cmd);
  std::remove(ll.c_str());
  if (rc != 0) {
    errs.push_back({"build", "clang failed to assemble the generated ir", 0, 0});
    return false;
  }
  return true;
}

bool Driver::doExe(const Options& opt, const std::string& outPath) {
  std::string dir = tempDir();
  std::string ll = dir + "\\oxide_out.ll";
  std::string rt = dir + "\\oxide_rt.c";


  std::string irFix = opt.freestanding ? ir_ : renameMain(ir_);
  if (!writeFile(ll, irFix)) {
    errs.push_back({"emit", "cannot write temp ir", 0, 0});
    return false;
  }
  std::string rtSrc;
  if (!opt.freestanding) {
    rtSrc = runtimeC();
    if (!writeFile(rt, rtSrc)) {
      errs.push_back({"emit", "cannot write runtime", 0, 0});
      return false;
    }
  }

  std::string clang = findTool({"clang", "clang-cl", "gcc", "cl"});
  if (clang.empty()) clang = "clang";
  std::string optflag = optFlag(opt.optimize);


  std::string nw = "-Wno-override-module";

  std::string lflags = linkFlagsFor(clang, opt);
  std::string cmd;
  if (clang == "cl") {
    cmd = clang + " /O2 /Fe:\"" + outPath + "\" \"" + ll + "\"";
    if (!opt.freestanding) cmd += " \"" + rt + "\"";
    cmd += lflags;
  } else if (clang == "clang-cl") {
    cmd = clang + " -fuse-ld=lld " + optflag + " " + nw + " -o \"" + outPath + "\" \"" + ll + "\"";
    if (!opt.freestanding) cmd += " \"" + rt + "\"";
    cmd += lflags;
  } else {
    cmd = clang + " " + optflag + " " + nw + " -o \"" + outPath + "\" \"" + ll + "\"";
    if (!opt.freestanding) cmd += " \"" + rt + "\"";
    cmd += lflags;
  }
  int rc = runCmd(cmd);
  std::remove(ll.c_str());
  if (!opt.freestanding) std::remove(rt.c_str());
  if (rc != 0) {
    errs.push_back({"link", "clang failed to link the executable", 0, 0});
    return false;
  }
  return true;
}

bool Driver::doRun(const Options& opt) {


  std::string dir = tempDir();
  std::string exe = dir + "\\oxide_run_tmp.exe";
  if (!doExe(opt, exe)) {
    errs.push_back({"run", "failed to build temp executable", 0, 0});
    return false;
  }
  int rr = std::system(("\"" + exe + "\"").c_str());
  std::remove(exe.c_str());
  return rr == 0;
}
