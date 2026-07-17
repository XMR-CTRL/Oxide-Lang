#include "AST.h"
#include <vector>
#include <map>


bool isInt(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i8: case BType::Tag::i16: case BType::Tag::i32: case BType::Tag::i64:
    case BType::Tag::u8: case BType::Tag::u16: case BType::Tag::u32: case BType::Tag::u64:
    case BType::Tag::usize: case BType::Tag::char_: case BType::Tag::enum_:
      return true;
    default: return false;
  }
}
bool isFloat(const BType& t) { return t.tag == BType::Tag::f64 || t.tag == BType::Tag::f32; }
bool isNumeric(const BType& t) { return isInt(t) || isFloat(t); }
bool isScalar(const BType& t) {
  return isInt(t) || isFloat(t) || t.tag == BType::Tag::bool_ ||
         t.tag == BType::Tag::str || t.tag == BType::Tag::ptr ||
         t.tag == BType::Tag::fn_;
}


bool isSignedInt(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i8: case BType::Tag::i16: case BType::Tag::i32: case BType::Tag::i64:
    case BType::Tag::char_: case BType::Tag::enum_:
      return true;
    default: return false;
  }
}

int bitWidth(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i8:  case BType::Tag::u8:  case BType::Tag::char_: return 8;
    case BType::Tag::i16: case BType::Tag::u16: return 16;
    case BType::Tag::i32: case BType::Tag::u32: return 32;
    case BType::Tag::i64: case BType::Tag::u64: case BType::Tag::usize:
    case BType::Tag::ptr: case BType::Tag::enum_: return 64;
    case BType::Tag::f32: return 32;
    case BType::Tag::bool_: return 1;
    case BType::Tag::fn_: return 64;
    default: return 64;
  }
}

const char* typeName(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i64: return "i64";
    case BType::Tag::f64: return "f64";
    case BType::Tag::bool_: return "bool";
    case BType::Tag::void_: return "void";
    case BType::Tag::str: return "str";
    case BType::Tag::array: return "array";
    case BType::Tag::dynarray: return "vec";
    case BType::Tag::struct_: return t.structName.c_str();
    case BType::Tag::ptr: return "ptr";
    case BType::Tag::char_: return "char";
    case BType::Tag::i8: return "i8"; case BType::Tag::i16: return "i16";
    case BType::Tag::i32: return "i32";
    case BType::Tag::u8: return "u8"; case BType::Tag::u16: return "u16";
    case BType::Tag::u32: return "u32"; case BType::Tag::u64: return "u64";
    case BType::Tag::usize: return "usize";
    case BType::Tag::f32: return "f32";
    case BType::Tag::enum_: return t.structName.c_str();
    case BType::Tag::map_: return "map";
    case BType::Tag::set_: return "set";
    case BType::Tag::hmap_: return "hmap";
    case BType::Tag::hset_: return "hset";
    case BType::Tag::fn_: return "fn";
    case BType::Tag::generic_: return "generic";
  }
  return "?";
}

std::string typeSpelling(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i64: return "i64";
    case BType::Tag::f64: return "f64";
    case BType::Tag::bool_: return "bool";
    case BType::Tag::void_: return "void";
    case BType::Tag::str: return "str";
    case BType::Tag::array: {
      auto& e = arrayElem(t);
      return "[" + typeSpelling(e) + "; " + std::to_string(t.count) + "]";
    }
    case BType::Tag::dynarray: {
      auto& e = dynArrayElem(t);
      return "vec[" + typeSpelling(e) + "]";
    }
    case BType::Tag::struct_: return t.structName;
    case BType::Tag::ptr: return "*" + typeSpelling(pointee(t));
    case BType::Tag::char_: return "char";
    case BType::Tag::i8: return "i8"; case BType::Tag::i16: return "i16";
    case BType::Tag::i32: return "i32";
    case BType::Tag::u8: return "u8"; case BType::Tag::u16: return "u16";
    case BType::Tag::u32: return "u32"; case BType::Tag::u64: return "u64";
    case BType::Tag::usize: return "usize";
    case BType::Tag::f32: return "f32";
    case BType::Tag::enum_: return t.structName;
    case BType::Tag::map_: return "map[" + typeSpelling(mapKeyType(t)) + ", " +
                                   typeSpelling(mapValType(t)) + "]";
    case BType::Tag::set_: return "set[" + typeSpelling(setElemType(t)) + "]";
    case BType::Tag::hmap_: return "hmap[" + typeSpelling(mapKeyType(t)) + ", " +
                                    typeSpelling(mapValType(t)) + "]";
    case BType::Tag::hset_: return "hset[" + typeSpelling(setElemType(t)) + "]";
    case BType::Tag::fn_: {
      const auto& ps = fnParams(t);
      std::string s = "fn(";
      for (size_t i = 0; i < ps.size(); i++) { if (i) s += ", "; s += typeSpelling(ps[i]); }
      s += ") -> " + (fnRet(t).tag == BType::Tag::void_ ? "void" : typeSpelling(fnRet(t)));
      return s;
    }
    case BType::Tag::generic_: {
      std::string s = genericInstBase(t);
      const auto& a = genericInstArgs(t);
      s += "<";
      for (size_t i = 0; i < a.size(); i++) { if (i) s += ", "; s += typeSpelling(a[i]); }
      s += ">";
      return s;
    }
  }
  return "?";
}
static std::vector<BType>& typeTable() {
  static std::vector<BType> t;
  return t;
}

BType makeArrayType(const BType& elem, int32_t count) {

  auto& t = typeTable();
  for (size_t i = 0; i < t.size(); i++) {
    if (t[i] == elem) {
      BType a; a.tag = BType::Tag::array; a.count = count; a.elem = (int32_t)i;
      return a;
    }
  }
  t.push_back(elem);
  BType a; a.tag = BType::Tag::array; a.count = count; a.elem = (int32_t)t.size() - 1;
  return a;
}
const BType& arrayElem(const BType& arr) {
  static BType fallback = BType::i64;
  if (arr.tag != BType::Tag::array) return fallback;
  if (arr.elem < 0 || (size_t)arr.elem >= typeTable().size()) return fallback;
  return typeTable()[arr.elem];
}

BType makeDynArray(const BType& elem) {


  auto& t = typeTable();
  int32_t ei = -1;
  for (size_t i = 0; i < t.size(); i++) { if (t[i] == elem) { ei = (int32_t)i; break; } }
  if (ei < 0) { t.push_back(elem); ei = (int32_t)t.size() - 1; }
  BType d; d.tag = BType::Tag::dynarray; d.elem = ei; return d;
}
const BType& dynArrayElem(const BType& arr) {
  static BType fallback = BType::i64;
  if (arr.tag != BType::Tag::dynarray) return fallback;
  if (arr.elem < 0 || (size_t)arr.elem >= typeTable().size()) return fallback;
  return typeTable()[arr.elem];
}


BType makePtr(const BType& pt) {

  auto& t = typeTable();
  int32_t ei = -1;
  for (size_t i = 0; i < t.size(); i++) { if (t[i] == pt) { ei = (int32_t)i; break; } }
  if (ei < 0) { t.push_back(pt); ei = (int32_t)t.size() - 1; }
  BType p; p.tag = BType::Tag::ptr; p.elem = ei; return p;
}


static int32_t internType(const BType& x) {
  auto& t = typeTable();
  for (size_t i = 0; i < t.size(); i++) if (t[i] == x) return (int32_t)i;
  t.push_back(x); return (int32_t)t.size() - 1;
}
BType makeMapType(const BType& key, const BType& val) {
  BType m; m.tag = BType::Tag::map_;
  m.elem = internType(key);
  m.count = internType(val);
  return m;
}
const BType& mapKeyType(const BType& m) {
  static BType fallback = BType::i64;
  if (m.tag != BType::Tag::map_ && m.tag != BType::Tag::hmap_) return fallback;
  if (m.elem < 0 || (size_t)m.elem >= typeTable().size()) return fallback;
  return typeTable()[m.elem];
}
const BType& mapValType(const BType& m) {
  static BType fallback = BType::i64;
  if (m.tag != BType::Tag::map_ && m.tag != BType::Tag::hmap_) return fallback;
  if (m.count < 0 || (size_t)m.count >= typeTable().size()) return fallback;
  return typeTable()[m.count];
}


BType makeSetType(const BType& elem) {
  BType s; s.tag = BType::Tag::set_;
  s.elem = internType(elem);
  return s;
}
const BType& setElemType(const BType& s) {
  static BType fallback = BType::i64;
  if ((s.tag != BType::Tag::set_ && s.tag != BType::Tag::hset_) ||
      s.elem < 0 || (size_t)s.elem >= typeTable().size()) return fallback;
  return typeTable()[s.elem];
}


BType makeHMapType(const BType& key, const BType& val) {
  BType m; m.tag = BType::Tag::hmap_;
  m.elem = internType(key);
  m.count = internType(val);
  return m;
}


BType makeHSetType(const BType& elem) {
  BType s; s.tag = BType::Tag::hset_;
  s.elem = internType(elem);
  return s;
}
const BType& pointee(const BType& p) {
  static BType fallback = BType::i64;
  if (p.tag != BType::Tag::ptr) return fallback;
  if (p.elem < 0 || (size_t)p.elem >= typeTable().size()) return fallback;
  return typeTable()[p.elem];
}


struct FnSig {
  std::vector<BType> params;
  BType ret;
};
static std::vector<FnSig>& fnSigTable() {
  static std::vector<FnSig> t;
  return t;
}
BType makeFnType(const std::vector<BType>& params, const BType& ret) {


  auto& tbl = fnSigTable();
  for (size_t i = 0; i < tbl.size(); i++) {
    if (tbl[i].ret == ret && tbl[i].params == params) {
      BType f; f.tag = BType::Tag::fn_; f.elem = (int32_t)i; return f;
    }
  }
  tbl.push_back({params, ret});
  BType f; f.tag = BType::Tag::fn_; f.elem = (int32_t)tbl.size() - 1; return f;
}
const std::vector<BType>& fnParams(const BType& t) {
  static const std::vector<BType> empty;
  if (t.tag != BType::Tag::fn_ || t.elem < 0 || (size_t)t.elem >= fnSigTable().size())
    return empty;
  return fnSigTable()[t.elem].params;
}
BType fnRet(const BType& t) {
  if (t.tag != BType::Tag::fn_ || t.elem < 0 || (size_t)t.elem >= fnSigTable().size())
    return BType::i64;
  return fnSigTable()[(size_t)t.elem].ret;
}


struct GenericInst {
  std::string base;
  std::vector<BType> args;
  bool isFn = false;
};
static std::vector<GenericInst>& genericInstTable() {
  static std::vector<GenericInst> t;
  return t;
}
BType makeGenericInst(const std::string& base, const std::vector<BType>& args, bool isFn) {
  GenericInst gi; gi.base = base; gi.args = args; gi.isFn = isFn;
  auto& tbl = genericInstTable();

  for (size_t i = 0; i < tbl.size(); i++) {
    if (tbl[i].base == base && tbl[i].isFn == isFn && tbl[i].args == args) {
      BType g; g.tag = BType::Tag::generic_; g.elem = (int32_t)i; return g;
    }
  }
  tbl.push_back(std::move(gi));
  BType g; g.tag = BType::Tag::generic_; g.elem = (int32_t)tbl.size() - 1; return g;
}
bool isGenericInst(const BType& t) { return t.tag == BType::Tag::generic_; }
const GenericInst& genericInst(const BType& t) {
  static GenericInst fallback;
  if (t.tag != BType::Tag::generic_ || t.elem < 0 || (size_t)t.elem >= genericInstTable().size())
    return fallback;
  return genericInstTable()[(size_t)t.elem];
}
const std::string& genericInstBase(const BType& t) {
  static const std::string empty;
  if (!isGenericInst(t) || t.elem < 0 || (size_t)t.elem >= genericInstTable().size()) return empty;
  return genericInstTable()[(size_t)t.elem].base;
}
const std::vector<BType>& genericInstArgs(const BType& t) {
  static const std::vector<BType> empty;
  if (!isGenericInst(t) || t.elem < 0 || (size_t)t.elem >= genericInstTable().size()) return empty;
  return genericInstTable()[(size_t)t.elem].args;
}
bool genericInstIsFn(const BType& t) {
  if (!isGenericInst(t) || t.elem < 0 || (size_t)t.elem >= genericInstTable().size()) return false;
  return genericInstTable()[(size_t)t.elem].isFn;
}


static std::map<std::string, const StructDecl*>& genericStructTemplates() {
  static std::map<std::string, const StructDecl*> t;
  return t;
}
static std::map<std::string, const FuncDecl*>& genericFnTemplates() {
  static std::map<std::string, const FuncDecl*> t;
  return t;
}
void registerGenericStruct(const StructDecl* tmpl) { genericStructTemplates()[tmpl->name] = tmpl; }
const StructDecl* findGenericStruct(const std::string& name) {
  auto it = genericStructTemplates().find(name);
  return it == genericStructTemplates().end() ? nullptr : it->second;
}
void registerGenericFn(const FuncDecl* tmpl) { genericFnTemplates()[tmpl->name] = tmpl; }
const FuncDecl* findGenericFn(const std::string& name) {
  auto it = genericFnTemplates().find(name);
  return it == genericFnTemplates().end() ? nullptr : it->second;
}


static std::map<std::string, BType>& aliasTable() {
  static std::map<std::string, BType> t;
  return t;
}
void registerAlias(const std::string& name, const BType& target) {
  aliasTable()[name] = target;
}
bool isAliasName(const std::string& name) {
  return aliasTable().count(name) != 0;
}
std::pair<bool, BType> resolveAlias(const std::string& name) {
  auto it = aliasTable().find(name);
  if (it == aliasTable().end()) return {false, BType::i64};
  return {true, it->second};
}


static std::map<std::string, StructDef>& structTable() {
  static std::map<std::string, StructDef> t;
  return t;
}

StructDef* registerStruct(const std::string& name) {
  auto& m = structTable();
  auto it = m.find(name);
  if (it != m.end()) return &it->second;
  auto& d = m[name];
  d.name = name;
  return &d;
}
StructDef* findStruct(const std::string& name) {
  auto& m = structTable();
  auto it = m.find(name);
  if (it == m.end()) return nullptr;
  return &it->second;
}
std::vector<StructDef*> allStructDefs() {
  std::vector<StructDef*> out;
  auto& m = structTable();
  out.reserve(m.size());
  for (auto& kv : m) out.push_back(&kv.second);
  return out;
}
int32_t structFieldIndex(const StructDef* d, const std::string& field) {
  if (!d) return -1;
  for (size_t i = 0; i < d->fields.size(); i++)
    if (d->fields[i].name == field) return (int32_t)i;
  return -1;
}

int32_t fieldByteWidth(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i64: case BType::Tag::u64: case BType::Tag::usize:
    case BType::Tag::f64: case BType::Tag::ptr: case BType::Tag::enum_:
    case BType::Tag::str: case BType::Tag::dynarray: return 8;
    case BType::Tag::i32: case BType::Tag::u32: case BType::Tag::f32: return 4;
    case BType::Tag::i16: case BType::Tag::u16: return 2;
    case BType::Tag::i8: case BType::Tag::u8: case BType::Tag::char_:
    case BType::Tag::bool_: return 1;
    case BType::Tag::fn_: return 8;
    case BType::Tag::array: {
      int32_t ew = fieldByteWidth(arrayElem(t));
      int32_t a = fieldAlign(arrayElem(t));
      if (a <= 0) a = 1;


      return t.count * ew;
    }
    case BType::Tag::struct_: {
      StructDef* d = findStruct(t.structName);
      if (d && d->size > 0) return d->size;
      return 0;
    }
    default: return 0;
  }
}
int32_t fieldAlign(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i64: case BType::Tag::u64: case BType::Tag::usize:
    case BType::Tag::f64: case BType::Tag::ptr: case BType::Tag::enum_:
    case BType::Tag::str: case BType::Tag::dynarray: return 8;
    case BType::Tag::i32: case BType::Tag::u32: case BType::Tag::f32: return 4;
    case BType::Tag::i16: case BType::Tag::u16: return 2;
    case BType::Tag::i8: case BType::Tag::u8: case BType::Tag::char_:
    case BType::Tag::bool_: return 1;
    case BType::Tag::array: return fieldAlign(arrayElem(t));
    case BType::Tag::fn_: return 8;
    case BType::Tag::struct_: {
      StructDef* d = findStruct(t.structName);
      if (d && d->align > 0) return d->align;
      return 8;
    }
    default: return 1;
  }
}


static std::map<std::string, EnumDef>& enumTable() {
  static std::map<std::string, EnumDef> t;
  return t;
}

EnumDef* registerEnum(const std::string& name) {
  auto& m = enumTable();
  auto it = m.find(name);
  if (it != m.end()) return &it->second;
  auto& d = m[name];
  d.name = name;
  return &d;
}
EnumDef* findEnum(const std::string& name) {
  auto& m = enumTable();
  auto it = m.find(name);
  if (it == m.end()) return nullptr;
  return &it->second;
}
std::pair<EnumDef*, int64_t> resolveEnumVariant(const std::string& name) {
  for (auto& kv : enumTable()) {
    EnumDef* d = &kv.second;
    for (size_t i = 0; i < d->variants.size(); i++)
      if (d->variants[i] == name) return {d, (int64_t)i};
  }
  return {nullptr, -1};
}
BType makeEnumType(const std::string& name) {
  BType t; t.tag = BType::Tag::enum_; t.structName = name; return t;
}
