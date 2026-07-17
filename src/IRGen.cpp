#include "IRGen.h"

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

IRGen::IRGen(Sema& sema) : sema_(sema) {}


static std::string esc(const std::string& s);


static bool isLvalueExpr(Expr* e) {
  if (dynamic_cast<VarRef*>(e)) return true;
  if (dynamic_cast<Index*>(e)) return true;
  if (dynamic_cast<Field*>(e)) return true;
  if (auto u = dynamic_cast<UnaryExpr*>(e)) return u->op == UnaryExpr::Op::deref;
  return false;
}

std::string IRGen::typeStr(BType t) {
  switch (t.tag) {
    case BType::Tag::i64: return "i64";
    case BType::Tag::f64: return "double";
    case BType::Tag::f32: return "float";
    case BType::Tag::bool_: return "i1";
    case BType::Tag::void_: return "void";
    case BType::Tag::str: return "i8*";
    case BType::Tag::array: return "[" + std::to_string(t.count) + " x " + typeStr(arrayElem(t)) + "]";
    case BType::Tag::dynarray: return "i8*";
    case BType::Tag::map_: return "i8*";
    case BType::Tag::set_: return "i8*";
    case BType::Tag::hmap_: return "i8*";
    case BType::Tag::hset_: return "i8*";
    case BType::Tag::struct_: return "%struct." + t.structName;
    case BType::Tag::ptr: return typeStr(pointee(t)) + "*";
    case BType::Tag::char_: return "i8";
    case BType::Tag::i8: return "i8";
    case BType::Tag::i16: return "i16";
    case BType::Tag::i32: return "i32";
    case BType::Tag::u8: return "i8";
    case BType::Tag::u16: return "i16";
    case BType::Tag::u32: return "i32";
    case BType::Tag::u64: return "i64";
    case BType::Tag::usize: return "i64";
    case BType::Tag::enum_: return "i64";
    case BType::Tag::fn_: return "i8*";
    case BType::Tag::generic_: return "i64";
  }
  return "i64";
}


static std::string intIrTy(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i8: case BType::Tag::u8: case BType::Tag::char_: return "i8";
    case BType::Tag::i16: case BType::Tag::u16: return "i16";
    case BType::Tag::i32: case BType::Tag::u32: return "i32";
    case BType::Tag::i64: case BType::Tag::u64: case BType::Tag::usize:
    case BType::Tag::enum_: return "i64";
    default: return "i64";
  }
}

std::string IRGen::genCoerce(const std::string& v, BType fromT, BType toT) {
  std::string fr = typeStr(fromT), tt = typeStr(toT);
  if (fr == tt) return v;


  if (fromT.tag == BType::Tag::ptr && toT.tag == BType::Tag::ptr) {


    if (pointee(fromT) == BType::void_ && v == "null")
      return "null";
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = bitcast " << fr << " " << v << " to " << tt << "\n";
    return r;
  }

  bool fi = isInt(fromT), ti = isInt(toT);
  bool fp = fromT.tag == BType::Tag::ptr, tp = toT.tag == BType::Tag::ptr;
  if (fi && ti) {
    int wb = bitWidth(fromT), wt = bitWidth(toT);
    std::string r = freshLocal("cast");
    if (wb == wt) return v;
    if (wb > wt) {
      out_ << "  " << r << " = trunc " << fr << " " << v << " to " << tt << "\n";
    } else {
      if (isSignedInt(fromT))
        out_ << "  " << r << " = sext " << fr << " " << v << " to " << tt << "\n";
      else
        out_ << "  " << r << " = zext " << fr << " " << v << " to " << tt << "\n";
    }
    return r;
  }

  if (fi && tp) {
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = inttoptr " << fr << " " << v << " to " << tt << "\n";
    return r;
  }
  if (fp && ti) {
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = ptrtoint " << fr << " " << v << " to " << tt << "\n";
    return r;
  }

  if (fi && isFloat(toT)) {
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = " << (isSignedInt(fromT) ? "sitofp " : "uitofp ")
         << fr << " " << v << " to " << tt << "\n";
    return r;
  }
  if (isFloat(fromT) && ti) {
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = " << (isSignedInt(toT) ? "fptosi " : "fptoui ")
         << fr << " " << v << " to " << tt << "\n";
    return r;
  }


  if (isFloat(fromT) && isFloat(toT)) {
    std::string r = freshLocal("cast");
    out_ << "  " << r << " = " << (toT == BType::f32 ? "fptrunc " : "fpext ")
         << fr << " " << v << " to " << tt << "\n";
    return r;
  }
  return v;
}


std::string IRGen::elemSuffix(const BType& t) {
  switch (t.tag) {
    case BType::Tag::i64: case BType::Tag::enum_:
      return "i64";
    case BType::Tag::f64: return "f64";
    case BType::Tag::bool_: return "i1";
    case BType::Tag::str: return "str";
    case BType::Tag::char_: return "i8";
    default: return "";
  }
}
std::string IRGen::elemIrType(const BType& t) { return typeStr(t); }


std::string IRGen::vecSlotType(const std::string& sx) {
  if (sx == "i64") return "i64";
  if (sx == "f64") return "double";
  if (sx == "i1")  return "i1";
  if (sx == "i8")  return "i8";
  return "i8*";
}

BType IRGen::vecSlotBType(const std::string& sx) {
  if (sx == "i64") return BType::i64;
  if (sx == "f64") return BType::f64;
  if (sx == "i1")  return BType::bool_;
  if (sx == "i8")  return BType::char_;
  return BType::str;
}

std::string IRGen::freshLabel(const std::string& hint) {
  return hint + std::to_string(labelSeq_++);
}
std::string IRGen::freshGlobal(const std::string& hint) {
  return "@" + hint + std::to_string(strSeq_++);
}
int IRGen::freshInt() { return labelSeq_++; }
std::string IRGen::freshLocal(const std::string& hint) {
  return "%" + hint + std::to_string(labelSeq_++);
}

std::pair<std::string, BType> IRGen::findVar(const std::string& name) {
  for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
    auto f = it->find(name);
    if (f != it->end()) return {f->second.first, f->second.second};
  }
  return {"", BType::void_};
}
void IRGen::pushScope() { scopes_.emplace_back(); }
void IRGen::popScope() { scopes_.pop_back(); }
void IRGen::declareVar(const std::string& name, const std::string& storage, BType t) {
  scopes_.back()[name] = {storage, t};
}

void IRGen::collectStruct(const BType& t) {
  if (t.tag == BType::Tag::struct_) {
    if (StructDef* d = findStruct(t.structName)) {
      structDefs_[t.structName] = d;
      for (auto& f : d->fields) collectStruct(f.type);
    }
  } else if (t.tag == BType::Tag::array) {
    collectStruct(arrayElem(t));
  } else if (t.tag == BType::Tag::ptr) {
    collectStruct(pointee(t));
  } else if (t.tag == BType::Tag::dynarray) {
    collectStruct(dynArrayElem(t));
  } else if (t.tag == BType::Tag::map_ || t.tag == BType::Tag::hmap_) {
    collectStruct(mapKeyType(t)); collectStruct(mapValType(t));
  } else if (t.tag == BType::Tag::set_ || t.tag == BType::Tag::hset_) {
    collectStruct(setElemType(t));
  }
}


static long long keyCategory(const BType& t) {
  if (t == BType::str || t.tag == BType::Tag::ptr ||
      t.tag == BType::Tag::dynarray) return 3;
  if (isFloat(t)) return 2;
  if (isSignedInt(t)) return 0;
  return 1;
}


static std::string zeroVal(const BType& t) {
  switch (t.tag) {
    case BType::Tag::f64: return "0.0";
    case BType::Tag::f32: return "0.0";
    case BType::Tag::bool_: return "0";
    case BType::Tag::array: case BType::Tag::struct_:
      return "zeroinitializer";
    case BType::Tag::str: case BType::Tag::ptr: case BType::Tag::dynarray:
    case BType::Tag::map_: case BType::Tag::set_:
    case BType::Tag::hmap_: case BType::Tag::hset_:
      return "null";
    default: return "0";
  }
}

std::string IRGen::globalInit(const BType& t, const GlobalInfo& gi, bool folded) {

  if (folded) {
    if (t == BType::f64) {
      uint64_t bits; double dv = gi.fVal; std::memcpy(&bits, &dv, sizeof(bits));
      std::ostringstream s; s << "0x" << std::hex << std::setfill('0') << std::setw(16) << bits;
      return s.str();
    }
    if (t == BType::bool_) return gi.bVal ? "1" : "0";
    if (t == BType::str) {


      std::string g = freshGlobal("str");
      std::string data = esc(gi.sVal);
      size_t n = gi.sVal.size() + 1;
      globals_ << g << " = private constant [" << n << " x i8] c\"" << data << "\\00\"\n";
      std::string r = freshLocal("sg");


      std::ostringstream s;
      s << "getelementptr ([" << n << " x i8], [" << n << " x i8]* " << g
        << ", i32 0, i32 0)";
      return s.str();
    }
    if (t == BType::char_) return std::to_string((unsigned)gi.cVal);
    if (t.tag == BType::Tag::ptr) return "null";

    return std::to_string(gi.iVal);
  }

  if (t.tag == BType::Tag::array || t.tag == BType::Tag::struct_)
    return "zeroinitializer";
  if (t == BType::f64) return "0.0";
  if (t == BType::bool_) return "0";
  if (t == BType::str || t.tag == BType::Tag::ptr) return "null";
  return "0";
}

void IRGen::emitGlobalsAndExterns() {

  bool any = false;
  for (auto& kv : sema_.funcs) {
    if (!kv.second.isExtern) continue;
    if (!any) out_ << "; extern functions\n";
    any = true;
    out_ << "declare " << typeStr(kv.second.retType) << " @" << kv.first << "(";
    for (size_t i = 0; i < kv.second.paramTypes.size(); i++) {
      if (i) out_ << ", ";
      out_ << typeStr(kv.second.paramTypes[i]);
    }
    out_ << ")\n";
  }
  if (any) out_ << "\n";


  bool anyG = false;
  for (auto& kv : sema_.globals) {
    const std::string& nm = kv.first;
    const GlobalInfo& gi = kv.second;
    collectStruct(gi.type);
    if (!anyG) out_ << "; top-level globals\n";
    anyG = true;
    std::string ty = typeStr(gi.type);
    std::string sym = "@" + nm;
    if (gi.isExtern) {
      out_ << sym << " = external global " << ty << "\n";
    } else if (gi.isConst) {
      std::string init = globalInit(gi.type, gi, gi.hasConstVal);
      out_ << sym << " = private constant " << ty << " " << init << "\n";
    } else {


      std::string init = gi.hasConstVal ? globalInit(gi.type, gi, true) : globalInit(gi.type, gi, false);
      out_ << sym << " = global " << ty << " " << init << "\n";
    }
  }
  if (anyG) out_ << "\n";
}

std::pair<std::string, BType> IRGen::genAddr(Expr* e) {
  if (auto v = dynamic_cast<VarRef*>(e)) {
    auto [store, t] = findVar(v->name);
    if (store.empty()) return {"", BType::void_};
    return {store, t};
  }
  if (auto u = dynamic_cast<UnaryExpr*>(e)) {


    if (u->op == UnaryExpr::Op::deref) {
      auto [pv, pvt] = genExpr(u->base.get());
      if (pvt.tag != BType::Tag::ptr) return {"", BType::void_};
      return {pv, pointee(pvt)};
    }
  }
  if (auto ix = dynamic_cast<Index*>(e)) {
    auto [base, bt] = genAddr(ix->base.get());


    if (!base.empty() && bt.tag == BType::Tag::ptr &&
        pointee(bt).tag == BType::Tag::array) {


      std::string lp = freshLocal("adrix");
      out_ << "  " << lp << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << base << "\n";
      base = lp; bt = pointee(bt);
    }


    if (bt.tag == BType::Tag::dynarray) {
      std::string handle;
      if (base.empty()) {
        auto [hv, hvt] = genExpr(ix->base.get());
        handle = hv;
      } else {
        std::string hp = freshLocal("adh");
        out_ << "  " << hp << " = load i8*, i8** " << base << "\n";
        handle = hp;
      }
      BType et = dynArrayElem(bt);
      auto [idx, it2] = genExpr(ix->index.get());
      std::string sx = elemSuffix(et);
      if (!sx.empty()) {
        usedVec_.insert(sx);
        std::string slot = vecSlotType(sx);
        std::string raw = freshLocal("agget");
        out_ << "  " << raw << " = call " << slot << " @ox_vec_get_" << sx
             << "(i8* " << handle << ", i64 " << idx << ")\n";
        std::string r = genCoerce(raw, vecSlotBType(sx), et);
        return {r, et};
      }

      int32_t esz = fieldByteWidth(et);
      usedVec_blob_ = true;
      std::string ep = freshLocal("vbp");
      out_ << "  " << ep << " = call i8* @ox_vec_blob_ptr(i8* " << handle
           << ", i64 " << idx << ", i64 " << (esz > 0 ? esz : 8) << ")\n";
      std::string tp = freshLocal("vbpc");
      out_ << "  " << tp << " = bitcast i8* " << ep << " to " << typeStr(et) << "*\n";
      return {tp, et};
    }
    if (base.empty()) {

      auto [val, vt] = genExpr(ix->base.get());
      if (vt.tag != BType::Tag::array) return {"", BType::void_};
      std::string a = freshLocal("arrtmp");
      out_ << "  " << a << " = alloca " << typeStr(vt) << "\n";
      out_ << "  store " << typeStr(vt) << " " << val << ", " << typeStr(vt) << "* " << a << "\n";
      base = a; bt = vt;
    }
    if (bt.tag != BType::Tag::array) return {"", BType::void_};
    auto [idx2, it2b] = genExpr(ix->index.get());
    boundsCheck(idx2, bt.count);
    std::string elemPtr = freshLocal("ep");
    out_ << "  " << elemPtr << " = getelementptr inbounds " << typeStr(bt) << ", "
         << typeStr(bt) << "* " << base << ", i64 0, i64 " << idx2 << "\n";
    return {elemPtr, arrayElem(bt)};
  }
  if (auto fl = dynamic_cast<Field*>(e)) {
    auto [base, bt] = genAddr(fl->base.get());
    if (base.empty()) {
      auto [val, vt] = genExpr(fl->base.get());


      if (vt.tag == BType::Tag::ptr && pointee(vt).tag == BType::Tag::struct_) {
        std::string lp = freshLocal("adrf");
        out_ << "  " << lp << " = load " << typeStr(pointee(vt)) << ", "
             << typeStr(pointee(vt)) << "* " << val << "\n";
        std::string a = freshLocal("sttmp");
        out_ << "  " << a << " = alloca " << typeStr(pointee(vt)) << "\n";
        out_ << "  store " << typeStr(pointee(vt)) << " " << lp << ", "
             << typeStr(pointee(vt)) << "* " << a << "\n";
        base = a; bt = pointee(vt);
      } else {
        if (vt.tag != BType::Tag::struct_) return {"", BType::void_};
        std::string a = freshLocal("sttmp");
        out_ << "  " << a << " = alloca " << typeStr(vt) << "\n";
        out_ << "  store " << typeStr(vt) << " " << val << ", " << typeStr(vt) << "* " << a << "\n";
        base = a; bt = vt;
      }
    } else if (bt.tag == BType::Tag::ptr && pointee(bt).tag == BType::Tag::struct_) {


      std::string lp = freshLocal("adrf");
      out_ << "  " << lp << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << base << "\n";
      base = lp; bt = pointee(bt);
    }
    if (bt.tag != BType::Tag::struct_) return {"", BType::void_};
    StructDef* d = findStruct(bt.structName);
    if (!d) return {"", BType::void_};
    int32_t fi = structFieldIndex(d, fl->field);
    if (fi < 0) return {"", BType::void_};
    std::string fp = freshLocal("fp");
    out_ << "  " << fp << " = getelementptr inbounds " << typeStr(bt) << ", "
         << typeStr(bt) << "* " << base << ", i64 0, i32 " << fi << "\n";
    return {fp, d->fields[fi].type};
  }
  return {"", BType::void_};
}

void IRGen::boundsCheck(const std::string& idx, int32_t count) {

  std::string neg = freshLocal("ic");
  out_ << "  " << neg << " = icmp slt i64 " << idx << ", 0\n";
  std::string ovf = freshLocal("ic");
  out_ << "  " << ovf << " = icmp sge i64 " << idx << ", " << count << "\n";
  std::string bad = freshLocal("bad");
  out_ << "  " << bad << " = or i1 " << neg << ", " << ovf << "\n";
  std::string okBB = freshLabel("idx_ok");
  std::string failBB = freshLabel("idx_fail");
  branch(bad, failBB, okBB);
  beginBlock(failBB);
  out_ << "  call void @ox_bounds_fail(i64 " << idx << ", i64 " << count << ")\n";
  out_ << "  unreachable\n";
  terminated_ = false;
  beginBlock(okBB);
}

void IRGen::strBoundsCheck(const std::string& strPtr, const std::string& idx) {

  std::string len = freshLocal("slen");
  out_ << "  " << len << " = call i64 @ox_strlen(i8* " << strPtr << ")\n";
  std::string neg = freshLocal("ic");
  out_ << "  " << neg << " = icmp slt i64 " << idx << ", 0\n";
  std::string ovf = freshLocal("ic");
  out_ << "  " << ovf << " = icmp sge i64 " << idx << ", " << len << "\n";
  std::string bad = freshLocal("bad");
  out_ << "  " << bad << " = or i1 " << neg << ", " << ovf << "\n";
  std::string okBB = freshLabel("sidx_ok");
  std::string failBB = freshLabel("sidx_fail");
  branch(bad, failBB, okBB);
  beginBlock(failBB);
  out_ << "  call void @ox_bounds_fail(i64 " << idx << ", i64 " << len << ")\n";
  out_ << "  unreachable\n";
  terminated_ = false;
  beginBlock(okBB);
}


static int g_tmp = 0;
static std::string tmp() { return std::string("%t") + std::to_string(g_tmp++); }

void IRGen::emitHeaderAndRuntime() {

  for (auto& kv : structDefs_) {
    std::string def = "type { ";
    bool first = true;
    for (auto& f : kv.second->fields) {
      if (!first) def += ", ";
      first = false;
      def += typeStr(f.type);
    }
    def += " }";
    out_ << "%struct." << kv.first << " = " << def << "\n";
  }
  if (!structDefs_.empty()) out_ << "\n";


  auto rd = [&](const char* name, const char* declText) {
    if (userDefinedFns_.count(name)) return;
    out_ << declText;
  };

  rd("ox_puts",       "declare i32 @ox_puts(i8*)\n");
  rd("ox_puti",       "declare i32 @ox_puti(i64)\n");
  rd("ox_putf",       "declare i32 @ox_putf(double)\n");
  rd("ox_newline",    "declare i32 @ox_newline()\n");
  rd("ox_putc",       "declare i32 @ox_putc(i64)\n");

  rd("ox_abs_i64",    "declare i64 @ox_abs_i64(i64)\n");
  out_ << "declare double @llvm.fabs.f64(double)\n";
  rd("ox_sqrt",       "declare double @ox_sqrt(double)\n");
  rd("ox_imin",       "declare i64 @ox_imin(i64, i64)\n");
  rd("ox_imax",       "declare i64 @ox_imax(i64, i64)\n");
  rd("ox_fmin2",      "declare double @ox_fmin2(double, double)\n");
  rd("ox_fmax2",      "declare double @ox_fmax2(double, double)\n");

  rd("ox_itos",       "declare i8* @ox_itos(i64)\n");
  rd("ox_stoi",       "declare i64 @ox_stoi(i8*)\n");
  rd("ox_stod",       "declare double @ox_stod(i8*)\n");

  rd("ox_strlen",     "declare i64 @ox_strlen(i8*)\n");
  rd("ox_strcmp",     "declare i64 @ox_strcmp(i8*, i8*)\n");

  rd("ox_substr",     "declare i8* @ox_substr(i8*, i64, i64)\n");
  rd("ox_strchr",     "declare i64 @ox_strchr(i8*, i64)\n");
  rd("ox_char_str",   "declare i8* @ox_char_str(i64)\n");
  rd("ox_ftos",       "declare i8* @ox_ftos(double)\n");

  rd("ox_sb_new",     "declare i8* @ox_sb_new()\n");
  rd("ox_sb_puts",    "declare void @ox_sb_puts(i8*, i8*)\n");
  rd("ox_sb_finish",  "declare i8* @ox_sb_finish(i8*)\n");

  rd("ox_read_line",  "declare i8* @ox_read_line()\n");
  rd("ox_read_file",  "declare i8* @ox_read_file(i8*)\n");
  rd("ox_file_open",  "declare i64 @ox_file_open(i8*, i8*)\n");
  rd("ox_file_close", "declare i64 @ox_file_close(i64)\n");
  rd("ox_file_read",  "declare i8* @ox_file_read(i64)\n");
  rd("ox_file_write", "declare i64 @ox_file_write(i64, i8*)\n");
  rd("ox_file_exists","declare i1 @ox_file_exists(i8*)\n");

  rd("ox_bounds_fail", "declare void @ox_bounds_fail(i64, i64)\n");


  if (!usedVec_.empty() || usedVec_blob_) out_ << "declare i64  @ox_vec_len(i8*)\n";
  for (const auto& sx : usedVec_) {

    std::string et = (sx == "i64") ? "i64" : (sx == "f64") ? "double" :
                     (sx == "i1") ? "i1" : (sx == "i8") ? "i8" : "i8*";
    out_ << "declare i8*  @ox_vec_new_" << sx << "()\n";
    out_ << "declare void @ox_vec_push_" << sx << "(i8*, " << et << ")\n";
    out_ << "declare " << et << " @ox_vec_get_" << sx << "(i8*, i64)\n";
    out_ << "declare void @ox_vec_set_" << sx << "(i8*, i64, " << et << ")\n";
    out_ << "declare void @ox_vec_print_" << sx << "(i8*)\n";
  }

  if (usedVec_blob_) {
    out_ << "declare i8*  @ox_vec_blob_new(i64)\n";
    out_ << "declare void @ox_vec_blob_push(i8*, i64, i8*)\n";
    out_ << "declare void @ox_vec_blob_get(i8*, i64, i64, i8*)\n";
    out_ << "declare void @ox_vec_blob_set(i8*, i64, i64, i8*)\n";
    out_ << "declare i8*  @ox_vec_blob_ptr(i8*, i64, i64)\n";

    out_ << "declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)\n";
  }

  for (const auto& sx : usedSort_) {
    if (sx == "i64")      out_ << "declare void @ox_sort_i64(i8*)\n";
    else if (sx == "f64") out_ << "declare void @ox_sort_f64(i8*)\n";
    else if (sx == "i1")  out_ << "declare void @ox_sort_i1(i8*)\n";
    else if (sx == "i8")  out_ << "declare void @ox_sort_i8(i8*)\n";
    else if (sx == "str") out_ << "declare void @ox_sort_str(i8*)\n";
  }
  if (usedSort_blob_) out_ << "declare void @ox_sort_blob(i8*, i64, i64)\n";


  if (usedMap_) {
    out_ << "declare i8*  @ox_map_new(i64, i64, i64)\n";
    out_ << "declare i64  @ox_map_len(i8*)\n";
    out_ << "declare i64  @ox_map_contains(i8*, i8*)\n";
    out_ << "declare void @ox_map_set(i8*, i8*, i8*)\n";
    out_ << "declare i64  @ox_map_get(i8*, i8*, i8*)\n";
    out_ << "declare i8*  @ox_map_key_ptr(i8*, i64)\n";

    out_ << "declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)\n";
  }

  if (usedSet_) {
    out_ << "declare i8*  @ox_set_new(i64, i64)\n";
    out_ << "declare i64  @ox_set_len(i8*)\n";
    out_ << "declare i64  @ox_set_contains(i8*, i8*)\n";
    out_ << "declare void @ox_set_insert(i8*, i8*)\n";
    out_ << "declare void @ox_set_remove(i8*, i8*)\n";
    out_ << "declare i8*  @ox_set_ptr(i8*, i64)\n";
    out_ << "declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)\n";
  }
  out_ << "\n";
}

static std::string esc(const std::string& s) {
  std::string r;
  for (unsigned char c : s) {
    if (c == '\\') { r += "\\\\"; }
    else if (c == '"') { r += "\\22"; }
    else if (c >= 32 && c < 127) r += (char)c;
    else {
      char buf[8];
      std::snprintf(buf, sizeof(buf), "\\%02X", c);
      r += buf;
    }
  }
  return r;
}

std::pair<std::string, BType> IRGen::genExpr(Expr* e) {


  if (auto l = dynamic_cast<IntLit*>(e)) return {std::to_string(l->v), BType::i64};
  if (auto l = dynamic_cast<FloatLit*>(e)) {


    uint64_t bits;
    std::memcpy(&bits, &l->v, sizeof(bits));
    std::ostringstream s;
    s << "0x" << std::hex << std::setfill('0') << std::setw(16) << bits;
    return {s.str(), BType::f64};
  }
  if (auto l = dynamic_cast<BoolLit*>(e))
    return {l->v ? std::string("1") : std::string("0"), BType::bool_};
  if (auto l = dynamic_cast<StrLit*>(e)) {
    std::string g = freshGlobal("str");
    std::string data = esc(l->v);
    size_t n = l->v.size() + 1;
    globals_ << g << " = private constant [" << n << " x i8] c\"" << data << "\\00\"\n";
    std::string r = tmp();
    out_ << "  " << r << " = getelementptr inbounds [" << n << " x i8], [" << n << " x i8]* "
         << g << ", i64 0, i64 0\n";
    return {r, BType::str};
  }
  if (auto l = dynamic_cast<CharLit*>(e))
    return {std::to_string((unsigned)l->v), BType::char_};
  if (dynamic_cast<NullLit*>(e)) {


    return {"null", makePtr(BType::void_)};
  }
  if (auto s = dynamic_cast<SizeofExpr*>(e)) {


    std::string sz = (s->size > 0) ? std::to_string(s->size) : std::to_string(fieldByteWidth(s->target));
    return {sz, BType::i64};
  }
  if (auto a = dynamic_cast<AsmExpr*>(e)) {


    auto esc = [&](const std::string& s) {
      std::string o;
      for (char c : s) {
        if (c == '\\') o += "\\\\";
        else if (c == '"') o += "\\22";
        else if (c == '\n') o += "\\0A";
        else if (c == '\t') o += "\\09";
        else o += c;

      }
      return o;
    };

    std::string cons;
    std::vector<std::string> argTypes, argVals;


    std::vector<std::pair<size_t, Expr*>> inoutPairs;
    size_t outCounter = 0;

    for (auto& io : a->ios) {
      if (!io.isOutput) continue;
      auto [v, vt] = genExpr(io.val.get());
      (void)v; (void)vt;
      if (!cons.empty()) cons += ",";
      cons += "=" + io.constraint;
      if (io.isInOut) inoutPairs.push_back({outCounter, io.val.get()});
      ++outCounter;
    }

    for (auto& io : a->ios) {
      if (io.isOutput) continue;
      auto [v, vt] = genExpr(io.val.get());
      std::string argT = typeStr(vt);
      argTypes.push_back(argT);
      argVals.push_back(v);
      if (!cons.empty()) cons += ",";
      cons += io.constraint;
    }
    for (auto& [oidx, iexpr] : inoutPairs) {
      auto [v, vt] = genExpr(iexpr);
      std::string argT = typeStr(vt);
      argTypes.push_back(argT);
      argVals.push_back(v);
      if (!cons.empty()) cons += ",";
      cons += std::to_string(oidx);
    }

    if (!a->clobbers.empty()) {

      size_t start = 0;
      while (start <= a->clobbers.size()) {
        size_t comma = a->clobbers.find(',', start);
        std::string tok = (comma == std::string::npos)
                          ? a->clobbers.substr(start)
                          : a->clobbers.substr(start, comma - start);

        auto lb = tok.find_first_not_of(" \t");
        auto rb = tok.find_last_not_of(" \t");
        if (lb != std::string::npos) {
          std::string reg = tok.substr(lb, rb - lb + 1);

          if (reg.front() == '{' && reg.back() == '}') reg = reg.substr(1, reg.size() - 2);
          if (!cons.empty()) cons += ",";
          cons += "~{" + reg + "}";
        }
        if (comma == std::string::npos) break;
        start = comma + 1;
      }
    }
    if (a->hasMemory) {
      if (!cons.empty()) cons += ",";
      cons += "~{memory}";
    }


    size_t nOut = a->outputTypes.size();
    std::string r;
    if (nOut >= 1) r = freshLocal("asm");
    out_ << "  ";
    if (nOut >= 1) out_ << r << " = ";
    std::string retIr;
    if (nOut == 0) retIr = "void";
    else if (nOut == 1) retIr = typeStr(a->outputTypes[0]);
    else {
      retIr = "{";
      for (size_t i = 0; i < nOut; i++) {
        if (i) retIr += ", ";
        retIr += typeStr(a->outputTypes[i]);
      }
      retIr += "}";
    }
    out_ << "call " << retIr << " asm ";
    if (a->sideEffect) out_ << "sideeffect ";
    out_ << "\"" << esc(a->asmText) << "\", \"" << cons << "\"(";
    for (size_t i = 0; i < argTypes.size(); i++) {
      if (i) out_ << ", ";
      out_ << argTypes[i] << " " << argVals[i];
    }
    out_ << ")\n";

    size_t outIdx = 0;
    for (auto& io : a->ios) {
      if (!io.isOutput) continue;

      std::string ov;
      BType ot = a->outputTypes[outIdx];
      if (nOut == 1) ov = r;
      else {
        ov = freshLocal("asmout");
        out_ << "  " << ov << " = extractvalue " << retIr << " " << r << ", " << outIdx << "\n";
      }
      auto [addr, addrTy] = genAddr(io.val.get());
      if (!addr.empty()) {
        std::string v = genCoerce(ov, ot, addrTy);
        out_ << "  store " << typeStr(addrTy) << " " << v << ", " << typeStr(addrTy)
             << "* " << addr << "\n";
      }
      outIdx++;
    }

    if (nOut == 1) return {r, a->outputTypes[0]};
    return {"", BType::void_};
  }
  if (auto c = dynamic_cast<CastExpr*>(e)) {
    auto [v, vt] = genExpr(c->e.get());
    return {genCoerce(v, vt, c->target), c->target};
  }
  if (auto v = dynamic_cast<VarRef*>(e)) {
    auto [store, t] = findVar(v->name);
    if (store.empty()) {

      auto [ed, ord] = resolveEnumVariant(v->name);
      if (ed) return {std::to_string(ord), makeEnumType(ed->name)};
      return {"0", BType::i64};
    }
    std::string r = tmp();
    out_ << "  " << r << " = load " << typeStr(t) << ", " << typeStr(t) << "* " << store << "\n";
    return {r, t};
  }
  if (auto u = dynamic_cast<UnaryExpr*>(e)) {


    if (u->methodOverload) {
      std::vector<ExprPtr> noargs;
      return emitOverloadCall(u->overloadStruct, u->overloadMethod,
                              u->overloadRecvType, u->recvByRef, u->base.get(),
                              noargs, false);
    }
    auto [b, bt] = genExpr(u->base.get());
    bool fp = (bt == BType::f64);
    if (u->op == UnaryExpr::Op::addr || u->op == UnaryExpr::Op::deref) {
      if (u->op == UnaryExpr::Op::addr) {
        auto [p, pte] = genAddr(u->base.get());
        (void)pte;
        if (p.empty()) return {"null", makePtr(BType::i8)};

        return {p, makePtr(bt)};
      }

      auto [pv, pvt] = genExpr(u->base.get());
      if (pvt.tag != BType::Tag::ptr) return {"0", BType::i64};
      BType et = pointee(pvt);
      std::string r = freshLocal("deref");
      out_ << "  " << r << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << pv << "\n";
      return {r, et};
    }
    std::string r = tmp();
    switch (u->op) {
      case UnaryExpr::Op::neg:
        if (fp) out_ << "  " << r << " = fneg double " << b << "\n";
        else out_ << "  " << r << " = sub " << intIrTy(bt) << " 0, " << b << "\n";
        return {r, bt};
      case UnaryExpr::Op::not_:
        out_ << "  " << r << " = xor i1 " << b << ", true\n";
        return {r, BType::bool_};
      case UnaryExpr::Op::bnot:
        out_ << "  " << r << " = xor " << intIrTy(bt) << " " << b << ", -1\n";
        return {r, BType::i64};
      default: return {"0", BType::i64};
    }
  }
  if (auto b = dynamic_cast<BinaryExpr*>(e)) {
    if (b->op == BinaryExpr::Op::land || b->op == BinaryExpr::Op::lor) {
      std::string rhsBB = freshLabel("sc_rhs");
      std::string doneBB = freshLabel("sc_done");
      std::string trueBB = freshLabel("sc_true");
      std::string r = tmp();
      std::string falseSrc;
      if (b->op == BinaryExpr::Op::land) {
        auto [l, lt] = genExpr(b->lhs.get());
        falseSrc = curBlock_;
        branch(l, rhsBB, doneBB);
        beginBlock(rhsBB);
        auto [rr, rt] = genExpr(b->rhs.get());
        std::string rhsSrc = curBlock_;
        jump(doneBB);
        beginBlock(doneBB);
        out_ << "  " << r << " = phi i1 [ false, %" << falseSrc << " ], [ " << rr << ", %" << rhsSrc << " ]\n";
      } else {
        auto [l, lt] = genExpr(b->lhs.get());
        branch(l, trueBB, rhsBB);
        beginBlock(rhsBB);
        auto [rr, rt] = genExpr(b->rhs.get());
        std::string rhsSrc = curBlock_;
        jump(doneBB);
        beginBlock(trueBB);
        std::string trueSrc = trueBB;
        jump(doneBB);
        beginBlock(doneBB);
        out_ << "  " << r << " = phi i1 [ " << rr << ", %" << rhsSrc << " ], [ true, %" << trueSrc << " ]\n";
      }
      return {r, BType::bool_};
    }


    if (b->methodOverload) {
      std::vector<ExprPtr> args;
      args.push_back(std::unique_ptr<Expr>(b->rhs.release()));
      bool neg = (b->op == BinaryExpr::Op::ne);
      return emitOverloadCall(b->overloadStruct, b->overloadMethod,
                              b->overloadRecvType, b->recvByRef, b->lhs.get(),
                              args, neg);
    }
    auto [l, lt] = genExpr(b->lhs.get());
    auto [r, rt] = genExpr(b->rhs.get());
    bool fp = isFloat(lt);
    bool isCmp = (b->op >= BinaryExpr::Op::eq && b->op <= BinaryExpr::Op::ge);


    if (lt.tag == BType::Tag::ptr && (b->op == BinaryExpr::Op::add || b->op == BinaryExpr::Op::sub) && isInt(rt)) {
      std::string idx = r;
      if (b->op == BinaryExpr::Op::sub) {
        std::string n = freshLocal("ni");
        out_ << "  " << n << " = sub i64 0, " << idx << "\n";
        idx = n;
      }
      std::string res = freshLocal("gep");
      out_ << "  " << res << " = getelementptr inbounds " << typeStr(pointee(lt)) << ", "
           << typeStr(lt) << " " << l << ", i64 " << idx << "\n";
      return {res, lt};
    }
    if (rt.tag == BType::Tag::ptr && b->op == BinaryExpr::Op::add && isInt(lt)) {
      std::string res = freshLocal("gep");
      out_ << "  " << res << " = getelementptr inbounds " << typeStr(pointee(rt)) << ", "
           << typeStr(rt) << " " << r << ", i64 " << l << "\n";
      return {res, rt};
    }

    if (lt.tag == BType::Tag::ptr && rt.tag == BType::Tag::ptr && (b->op == BinaryExpr::Op::eq || b->op == BinaryExpr::Op::ne)) {
      std::string res = freshLocal("pc");
      out_ << "  " << res << " = icmp " << (b->op == BinaryExpr::Op::eq ? "eq" : "ne")
           << " " << typeStr(lt) << " " << l << ", " << r << "\n";
      return {res, BType::bool_};
    }


    if (lt == BType::str && rt == BType::str && b->op == BinaryExpr::Op::add) {
      std::string sb = freshLocal("sb"); out_ << "  " << sb << " = call i8* @ox_sb_new()\n";
      out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << l << ")\n";
      out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << r << ")\n";
      std::string res = freshLocal("cat");
      out_ << "  " << res << " = call i8* @ox_sb_finish(i8* " << sb << ")\n";
      return {res, BType::str};
    }


    if (b->op == BinaryExpr::Op::add &&
        ((lt == BType::str && rt == BType::char_) ||
         (lt == BType::char_ && rt == BType::str))) {
      std::string ls = l, rs = r;
      if (lt == BType::char_) {
        std::string c = genCoerce(l, BType::char_, BType::i64);
        ls = freshLocal("c2s");
        out_ << "  " << ls << " = call i8* @ox_char_str(i64 " << c << ")\n";
      }
      if (rt == BType::char_) {
        std::string c = genCoerce(r, BType::char_, BType::i64);
        rs = freshLocal("c2s");
        out_ << "  " << rs << " = call i8* @ox_char_str(i64 " << c << ")\n";
      }
      std::string sb = freshLocal("sb"); out_ << "  " << sb << " = call i8* @ox_sb_new()\n";
      out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << ls << ")\n";
      out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << rs << ")\n";
      std::string res = freshLocal("cat");
      out_ << "  " << res << " = call i8* @ox_sb_finish(i8* " << sb << ")\n";
      return {res, BType::str};
    }

    if (lt == BType::str && rt == BType::str && (b->op == BinaryExpr::Op::eq || b->op == BinaryExpr::Op::ne)) {
      std::string d = freshLocal("scmp");
      out_ << "  " << d << " = call i64 @ox_strcmp(i8* " << l << ", i8* " << r << ")\n";
      std::string res = freshLocal("ceq");
      out_ << "  " << res << " = icmp eq i64 " << d << ", 0\n";
      if (b->op == BinaryExpr::Op::ne) {
        std::string nres = freshLocal("cne");
        out_ << "  " << nres << " = xor i1 " << res << ", true\n";
        res = nres;
      }
      return {res, BType::bool_};
    }


    if (lt == BType::str && rt == BType::str &&
        (b->op == BinaryExpr::Op::lt || b->op == BinaryExpr::Op::le ||
         b->op == BinaryExpr::Op::gt || b->op == BinaryExpr::Op::ge)) {
      std::string d = freshLocal("scmp");
      out_ << "  " << d << " = call i64 @ox_strcmp(i8* " << l << ", i8* " << r << ")\n";
      const char* pred = nullptr;
      switch (b->op) {
        case BinaryExpr::Op::lt: pred = "slt"; break;
        case BinaryExpr::Op::le: pred = "sle"; break;
        case BinaryExpr::Op::gt: pred = "sgt"; break;
        case BinaryExpr::Op::ge: pred = "sge"; break;
        default: pred = "slt"; break;
      }
      std::string res = freshLocal("ord");
      out_ << "  " << res << " = icmp " << pred << " i64 " << d << ", 0\n";
      return {res, BType::bool_};
    }


    std::string lval = genCoerce(l, lt, lt);
    std::string rval = genCoerce(r, rt, lt);
    std::string res = tmp();
    std::string instruction;
    bool isIntOp = isInt(lt);


    std::string ity = isIntOp ? intIrTy(lt) : typeStr(lt);
    bool signedOp = isSignedInt(lt);
    switch (b->op) {
      case BinaryExpr::Op::add: instruction = fp ? ("fadd " + ity) : ("add " + ity); break;
      case BinaryExpr::Op::sub: instruction = fp ? ("fsub " + ity) : ("sub " + ity); break;
      case BinaryExpr::Op::mul: instruction = fp ? ("fmul " + ity) : ("mul " + ity); break;
      case BinaryExpr::Op::div: instruction = fp ? ("fdiv " + ity) : (std::string(signedOp ? "sdiv " : "udiv ") + ity); break;
      case BinaryExpr::Op::mod: instruction = fp ? ("frem " + ity) : (std::string(signedOp ? "srem " : "urem ") + ity); break;
      case BinaryExpr::Op::band: instruction = "and " + ity; break;
      case BinaryExpr::Op::bor: instruction = "or " + ity; break;
      case BinaryExpr::Op::bxor: instruction = "xor " + ity; break;
      case BinaryExpr::Op::shl: instruction = "shl " + ity; break;
      case BinaryExpr::Op::shr: instruction = std::string(signedOp ? "ashr " : "lshr ") + ity; break;
      case BinaryExpr::Op::eq: instruction = fp ? ("fcmp oeq " + ity) : ("icmp eq " + ity); break;
      case BinaryExpr::Op::ne: instruction = fp ? ("fcmp one " + ity) : ("icmp ne " + ity); break;
      case BinaryExpr::Op::lt: instruction = fp ? ("fcmp olt " + ity) : (std::string("icmp ") + (signedOp ? "slt " : "ult ") + ity); break;
      case BinaryExpr::Op::gt: instruction = fp ? ("fcmp ogt " + ity) : (std::string("icmp ") + (signedOp ? "sgt " : "ugt ") + ity); break;
      case BinaryExpr::Op::le: instruction = fp ? ("fcmp ole " + ity) : (std::string("icmp ") + (signedOp ? "sle " : "ule ") + ity); break;
      case BinaryExpr::Op::ge: instruction = fp ? ("fcmp oge " + ity) : (std::string("icmp ") + (signedOp ? "sge " : "uge ") + ity); break;
      default: instruction = fp ? ("fadd " + ity) : ("add " + ity); break;
    }
    out_ << "  " << res << " = " << instruction << " " << lval << ", " << rval << "\n";
    return {res, isCmp ? BType::bool_ : lt};
  }
  if (auto t = dynamic_cast<TernaryExpr*>(e)) {


    BType rt = t->resultTy;
    auto [cv, ct] = genExpr(t->cond.get());
    std::string thenBB = freshLabel("tern_then");
    std::string elseBB = freshLabel("tern_else");
    std::string doneBB = freshLabel("tern_done");
    branch(cv, thenBB, elseBB);
    beginBlock(thenBB);
    auto [tv, tt] = genExpr(t->thenE.get());
    std::string tc = genCoerce(tv, tt, rt);
    std::string thenSrc = curBlock_;
    jump(doneBB);
    beginBlock(elseBB);
    auto [ev, et] = genExpr(t->elseE.get());
    std::string ec = genCoerce(ev, et, rt);
    std::string elseSrc = curBlock_;
    jump(doneBB);
    beginBlock(doneBB);
    std::string r = tmp();
    out_ << "  " << r << " = phi " << typeStr(rt) << " [ " << tc << ", %" << thenSrc
         << " ], [ " << ec << ", %" << elseSrc << " ]\n";
    return {r, rt};
  }
  if (auto d = dynamic_cast<IncDecExpr*>(e)) {


    ExprPtr lv;
    if (d->kind == AssignTarget::Kind::var) {
      auto vr = std::make_unique<VarRef>();
      vr->name = d->name; vr->line = d->line; vr->col = d->col;
      lv = std::move(vr);
    } else if (d->kind == AssignTarget::Kind::field) {
      auto fl = std::make_unique<Field>();
      fl->field = d->field; fl->line = d->line; fl->col = d->col;
      fl->base = cloneExpr(d->base.get());
      lv = std::move(fl);
    } else if (d->kind == AssignTarget::Kind::index) {
      auto ix = std::make_unique<Index>();
      ix->line = d->line; ix->col = d->col;
      ix->base = cloneExpr(d->base.get());
      ix->index = cloneExpr(d->index.get());
      lv = std::move(ix);
    } else {
      auto u = std::make_unique<UnaryExpr>();
      u->op = UnaryExpr::Op::deref; u->line = d->line; u->col = d->col;
      u->base = cloneExpr(d->base.get());
      lv = std::move(u);
    }
    auto [addr, st] = genAddr(lv.get());


    if (d->kind == AssignTarget::Kind::index && d->valueTy.tag != BType::Tag::void_ &&
        (addr.empty() || st != d->valueTy)) {

      auto [bv, bvt] = genExpr(d->base.get());
      auto [idxv, idxt] = genExpr(d->index.get());
      BType et = d->valueTy;
      std::string sx = elemSuffix(et);
      if (!sx.empty()) {
        usedVec_.insert(sx);
        std::string oldv = freshLocal("igg");
        out_ << "  " << oldv << " = call " << typeStr(et) << " @ox_vec_get_"
             << sx << "(i8* " << bv << ", i64 " << idxv << ")\n";
        std::string one = freshLocal("one");
        out_ << "  " << one << " = add " << typeStr(et) << " " << oldv << ", "
             << (d->isInc ? "1" : "-1") << "\n";
        out_ << "  call void @ox_vec_set_" << sx << "(i8* " << bv << ", i64 "
             << idxv << ", " << typeStr(et) << " " << one << ")\n";
        return {d->isPost ? oldv : one, et};
      }


    }
    if (addr.empty()) return {"", d->valueTy};
    BType vt = d->valueTy;
    std::string oldv = freshLocal("inc");
    out_ << "  " << oldv << " = load " << typeStr(vt) << ", " << typeStr(vt) << "* "
         << addr << "\n";


    std::string newv, storev;
    if (vt.tag == BType::Tag::ptr) {
      newv = freshLocal("incp");
      BType pt = pointee(vt);
      out_ << "  " << newv << " = getelementptr inbounds " << typeStr(pt) << ", "
           << typeStr(vt) << " " << oldv << ", i64 " << (d->isInc ? "1" : "-1") << "\n";
      storev = newv;
    } else if (vt == BType::bool_) {

      storev = freshLocal("incb");
      const char* op = d->isInc ? "or" : "and";
      std::string imm = d->isInc ? "true" : "false";
      out_ << "  " << storev << " = " << op << " i1 " << oldv << ", " << imm << "\n";
      newv = storev;
    } else {
      bool fp = (vt == BType::f64 || vt == BType::f32);

      newv = freshLocal("incn");
      const char* op = d->isInc ? (fp ? "fadd" : "add") : (fp ? "fsub" : "sub");
      std::string imm = fp ? "1.0" : "1";
      out_ << "  " << newv << " = " << op << " " << typeStr(vt) << " " << oldv << ", " << imm << "\n";
      storev = newv;
    }
    out_ << "  store " << typeStr(vt) << " " << storev << ", " << typeStr(vt) << "* "
         << addr << "\n";
    return {d->isPost ? oldv : newv, vt};
  }
  if (auto a = dynamic_cast<AssignTarget*>(e)) {


    if (a->methodOverload) {
      ExprPtr recv;
      a->line = a->line;
      if (a->kind == AssignTarget::Kind::var) {
        auto vr = std::make_unique<VarRef>();
        vr->name = a->name; vr->line = a->line; vr->col = a->col;
        recv = std::move(vr);
      } else if (a->kind == AssignTarget::Kind::field) {
        auto fl = std::make_unique<Field>();
        fl->field = a->field; fl->line = a->line; fl->col = a->col;
        fl->base = std::unique_ptr<Expr>(a->base.release());
        recv = std::move(fl);
      } else if (a->kind == AssignTarget::Kind::index) {
        auto ix = std::make_unique<Index>();
        ix->line = a->line; ix->col = a->col;
        ix->base = std::unique_ptr<Expr>(a->base.release());
        ix->index = std::unique_ptr<Expr>(a->index.release());
        recv = std::move(ix);
      } else if (a->kind == AssignTarget::Kind::deref) {
        auto u = std::make_unique<UnaryExpr>();
        u->op = UnaryExpr::Op::deref; u->line = a->line; u->col = a->col;
        u->base = std::unique_ptr<Expr>(a->base.release());
        recv = std::move(u);
      }
      std::vector<ExprPtr> args;
      args.push_back(std::unique_ptr<Expr>(a->value.release()));
      return emitOverloadCall(a->overloadStruct, a->overloadMethod,
                              a->overloadRecvType,true, recv.get(),
                              args, false);
    }
    std::string addr;
    BType st = BType::void_;
    if (a->kind == AssignTarget::Kind::var) {
      auto [s, t] = findVar(a->name);
      addr = s; st = t;
    } else if (a->kind == AssignTarget::Kind::index) {
      auto [baddr, bt] = genAddr(a->base.get());

      if (!baddr.empty() && bt.tag == BType::Tag::ptr &&
          pointee(bt).tag == BType::Tag::array) {
        std::string lp = freshLocal("adri");
        out_ << "  " << lp << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << baddr << "\n";
        BType at = pointee(bt);
        auto [idx, it2] = genExpr(a->index.get());
        boundsCheck(idx, at.count);
        std::string ep = freshLocal("ep");
        out_ << "  " << ep << " = getelementptr inbounds " << typeStr(at) << ", "
             << typeStr(at) << "* " << lp << ", i64 0, i64 " << idx << "\n";
        addr = ep; st = arrayElem(at);
      } else if (!baddr.empty() && bt.tag == BType::Tag::array) {
        auto [idx, it2] = genExpr(a->index.get());
        boundsCheck(idx, bt.count);
        std::string ep = freshLocal("ep");
        out_ << "  " << ep << " = getelementptr inbounds " << typeStr(bt) << ", "
             << typeStr(bt) << "* " << baddr << ", i64 0, i64 " << idx << "\n";
        addr = ep; st = arrayElem(bt);
      } else {

        auto [bv, bvt] = genExpr(a->base.get());
        auto [idx, it2] = genExpr(a->index.get());
        if (bvt.tag == BType::Tag::dynarray) {
          BType et = dynArrayElem(bvt);
          std::string sx = elemSuffix(et);
          dynSetPending_ = true;
          dynSetBlob_ = sx.empty();
          dynSetHandle_ = bv; dynSetIdx_ = idx; dynSetSx_ = sx; dynSetEt_ = et;
          st = et;
          if (!sx.empty()) usedVec_.insert(sx); else usedVec_blob_ = true;
        }
      }
    } else if (a->kind == AssignTarget::Kind::field) {
      auto [baddr, bt] = genAddr(a->base.get());


      if (!baddr.empty() && bt.tag == BType::Tag::ptr &&
          pointee(bt).tag == BType::Tag::struct_) {
        std::string lp = freshLocal("adrfw");
        out_ << "  " << lp << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << baddr << "\n";
        bt = pointee(bt);
        baddr = lp;
      }
      if (!baddr.empty() && bt.tag == BType::Tag::struct_) {
        StructDef* d = findStruct(bt.structName);
        int32_t fi = d ? structFieldIndex(d, a->field) : -1;
        if (fi >= 0) {
          std::string fp = freshLocal("fp");
          out_ << "  " << fp << " = getelementptr inbounds " << typeStr(bt) << ", "
               << typeStr(bt) << "* " << baddr << ", i64 0, i32 " << fi << "\n";
          addr = fp; st = d->fields[fi].type;
        }
      }
    } else if (a->kind == AssignTarget::Kind::deref) {


      auto [pv, pvt] = genExpr(a->base.get());
      if (pvt.tag == BType::Tag::ptr) { addr = pv; st = pointee(pvt); }
    }
    auto [v, vt] = genExpr(a->value.get());

    if (dynSetPending_) {
      dynSetPending_ = false;
      if (dynSetBlob_) {


        std::string cv = genCoerce(v, vt, dynSetEt_);
        std::string tmp = freshLocal("vbset");
        out_ << "  " << tmp << " = alloca " << typeStr(dynSetEt_) << "\n";
        out_ << "  store " << typeStr(dynSetEt_) << " " << cv << ", "
             << typeStr(dynSetEt_) << "* " << tmp << "\n";
        int32_t esz = fieldByteWidth(dynSetEt_);
        out_ << "  call void @ox_vec_blob_set(i8* " << dynSetHandle_
             << ", i64 " << dynSetIdx_ << ", i64 " << (esz > 0 ? esz : 8)
             << ", i8* " << tmp << ")\n";
        return {cv, dynSetEt_};
      }
      if (a->isCompound) {
        auto oldv = freshLocal("vget");
        out_ << "  " << oldv << " = call " << typeStr(dynSetEt_) << " @ox_vec_get_"
             << dynSetSx_ << "(i8* " << dynSetHandle_ << ", i64 " << dynSetIdx_ << ")\n";
        bool fp = (dynSetEt_ == BType::f64);
        const char* op = nullptr;
        switch (a->compound) {
          case BinaryExpr::Op::add: op = fp ? "fadd double" : "add i64"; break;
          case BinaryExpr::Op::sub: op = fp ? "fsub double" : "sub i64"; break;
          case BinaryExpr::Op::mul: op = fp ? "fmul double" : "mul i64"; break;
          case BinaryExpr::Op::div: op = fp ? "fdiv double" : "sdiv i64"; break;
          case BinaryExpr::Op::mod: op = fp ? "frem double" : "srem i64"; break;
          default: op = "add i64"; break;
        }
        std::string res = freshLocal("op");
        out_ << "  " << res << " = " << op << " " << oldv << ", " << v << "\n";
        out_ << "  call void @ox_vec_set_" << dynSetSx_ << "(i8* " << dynSetHandle_
             << ", i64 " << dynSetIdx_ << ", " << typeStr(dynSetEt_) << " " << res << ")\n";
        return {res, dynSetEt_};
      }
      out_ << "  call void @ox_vec_set_" << dynSetSx_ << "(i8* " << dynSetHandle_
           << ", i64 " << dynSetIdx_ << ", " << typeStr(dynSetEt_) << " " << v << ")\n";
      return {v, dynSetEt_};
    }
    if (addr.empty()) return {v, st};
    if (a->isCompound) {


      if (st == BType::str && a->compound == BinaryExpr::Op::add) {
        std::string cur = freshLocal("ld");
        out_ << "  " << cur << " = load " << typeStr(st) << ", " << typeStr(st) << "* " << addr << "\n";
        std::string sb = freshLocal("sb"); out_ << "  " << sb << " = call i8* @ox_sb_new()\n";
        out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << cur << ")\n";
        out_ << "  call void @ox_sb_puts(i8* " << sb << ", i8* " << v << ")\n";
        std::string res = freshLocal("cat");
        out_ << "  " << res << " = call i8* @ox_sb_finish(i8* " << sb << ")\n";
        out_ << "  store " << typeStr(st) << " " << res << ", " << typeStr(st) << "* " << addr << "\n";
        return {res, st};
      }
      std::string cur = freshLocal("ld");
      out_ << "  " << cur << " = load " << typeStr(st) << ", " << typeStr(st) << "* " << addr << "\n";
      bool fp = (st == BType::f64);
      const char* op = nullptr;
      switch (a->compound) {
        case BinaryExpr::Op::add: op = fp ? "fadd double" : "add i64"; break;
        case BinaryExpr::Op::sub: op = fp ? "fsub double" : "sub i64"; break;
        case BinaryExpr::Op::mul: op = fp ? "fmul double" : "mul i64"; break;
        case BinaryExpr::Op::div: op = fp ? "fdiv double" : "sdiv i64"; break;
        case BinaryExpr::Op::mod: op = fp ? "frem double" : "srem i64"; break;
        case BinaryExpr::Op::band: op = "and i64"; break;
        case BinaryExpr::Op::bor: op = "or i64"; break;
        case BinaryExpr::Op::bxor: op = "xor i64"; break;
        case BinaryExpr::Op::shl: op = "shl i64"; break;
        case BinaryExpr::Op::shr: op = "ashr i64"; break;
        default: op = "add i64"; break;
      }
      std::string res = freshLocal("op");
      out_ << "  " << res << " = " << op << " " << cur << ", " << v << "\n";
      out_ << "  store " << typeStr(st) << " " << res << ", " << typeStr(st) << "* " << addr << "\n";
      return {res, st};
    }
    out_ << "  store " << typeStr(st) << " " << v << ", " << typeStr(st) << "* " << addr << "\n";
    return {v, st};
  }
  if (auto ix = dynamic_cast<Index*>(e)) {


    if (ix->methodOverload) {
      std::vector<ExprPtr> args;
      args.push_back(std::unique_ptr<Expr>(ix->index.release()));
      return emitOverloadCall(ix->overloadStruct, ix->overloadMethod,
                              ix->overloadRecvType, ix->recvByRef, ix->base.get(),
                              args, false);
    }

    auto [bv, bvt] = genExpr(ix->base.get());
    if (bvt.tag == BType::Tag::dynarray) {
      BType et = dynArrayElem(bvt);
      std::string sx = elemSuffix(et);
      if (!sx.empty()) {

        usedVec_.insert(sx);
        auto [iv, it2] = genExpr(ix->index.get());
        std::string slot = vecSlotType(sx);
        std::string raw = freshLocal("vget");
        out_ << "  " << raw << " = call " << slot << " @ox_vec_get_" << sx
             << "(i8* " << bv << ", i64 " << iv << ")\n";
        std::string r = genCoerce(raw, vecSlotBType(sx), et);
        return {r, et};
      }


      usedVec_blob_ = true;
      auto [iv, it2] = genExpr(ix->index.get());
      int32_t esz = fieldByteWidth(et);
      std::string dst = freshLocal("vbg");
      out_ << "  " << dst << " = alloca " << typeStr(et) << "\n";
      out_ << "  call void @ox_vec_blob_get(i8* " << bv << ", i64 " << iv
           << ", i64 " << (esz > 0 ? esz : 8) << ", i8* " << dst << ")\n";


      std::string r = freshLocal("vbl");
      out_ << "  " << r << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << dst << "\n";
      return {r, et};
    }

    if (bvt == BType::str) {
      auto [ivRaw, it2] = genExpr(ix->index.get());
      std::string iv = genCoerce(ivRaw, it2, BType::i64);
      strBoundsCheck(bv, iv);

      std::string ce = freshLocal("sc");
      out_ << "  " << ce << " = getelementptr inbounds i8, i8* " << bv << ", i64 " << iv << "\n";
      std::string r = freshLocal("sl");
      out_ << "  " << r << " = load i8, i8* " << ce << "\n";
      return {r, BType::char_};
    }
    auto [addr, bt] = genAddr(ix);
    if (addr.empty()) return {"0", BType::i64};
    std::string r = freshLocal("idx");
    out_ << "  " << r << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << addr << "\n";
    return {r, bt};
  }
  if (auto fl = dynamic_cast<Field*>(e)) {
    auto [addr, bt] = genAddr(fl);
    if (addr.empty()) return {"0", BType::i64};
    std::string r = freshLocal("fld");
    out_ << "  " << r << " = load " << typeStr(bt) << ", " << typeStr(bt) << "* " << addr << "\n";
    return {r, bt};
  }
  if (auto al = dynamic_cast<ArrayLit*>(e)) {

    BType elemT = BType::i64;
    if (!al->elems.empty()) elemT = genExpr(al->elems[0].get()).second;
    BType arrT = makeArrayType(elemT, (int32_t)al->elems.size());
    std::string a = freshLocal("arr");
    out_ << "  " << a << " = alloca " << typeStr(arrT) << "\n";
    for (size_t i = 0; i < al->elems.size(); i++) {
      auto [v, vt] = genExpr(al->elems[i].get());
      std::string cv = genCoerce(v, vt, elemT);
      std::string ep = freshLocal("ae");
      out_ << "  " << ep << " = getelementptr inbounds " << typeStr(arrT) << ", "
           << typeStr(arrT) << "* " << a << ", i64 0, i64 " << i << "\n";
      out_ << "  store " << typeStr(elemT) << " " << cv << ", " << typeStr(elemT) << "* " << ep << "\n";
    }
    std::string r = freshLocal("arrval");
    out_ << "  " << r << " = load " << typeStr(arrT) << ", " << typeStr(arrT) << "* " << a << "\n";
    return {r, arrT};
  }
  if (auto sl = dynamic_cast<StructLit*>(e)) {
    BType st; st.tag = BType::Tag::struct_; st.structName = sl->name;
    std::string a = freshLocal("st");
    out_ << "  " << a << " = alloca " << typeStr(st) << "\n";
    StructDef* d = findStruct(sl->name);
    for (size_t i = 0; i < sl->values.size() && i < sl->fieldNames.size(); i++) {
      auto [v, vt] = genExpr(sl->values[i].get());
      if (!d) continue;
      int32_t fi = structFieldIndex(d, sl->fieldNames[i]);
      if (fi < 0) continue;
      BType ft = d->fields[fi].type;
      std::string cv = genCoerce(v, vt, ft);
      std::string fp = freshLocal("sf");
      out_ << "  " << fp << " = getelementptr inbounds " << typeStr(st) << ", "
           << typeStr(st) << "* " << a << ", i64 0, i32 " << fi << "\n";
      out_ << "  store " << typeStr(ft) << " " << cv << ", " << typeStr(ft) << "* " << fp << "\n";
    }
    std::string r = freshLocal("stval");
    out_ << "  " << r << " = load " << typeStr(st) << ", " << typeStr(st) << "* " << a << "\n";
    return {r, st};
  }
  if (auto dn = dynamic_cast<DynNew*>(e)) {
    std::string sx = elemSuffix(dn->elemType);
    std::string r = freshLocal("vnew");
    if (!sx.empty()) {

      usedVec_.insert(sx);
      out_ << "  " << r << " = call i8* @ox_vec_new_" << sx << "()\n";
    } else {


      usedVec_blob_ = true;
      int32_t esz = fieldByteWidth(dn->elemType);
      out_ << "  " << r << " = call i8* @ox_vec_blob_new(i64 " << (esz > 0 ? esz : 8) << ")\n";
    }
    return {r, makeDynArray(dn->elemType)};
  }
  if (auto mn = dynamic_cast<MapNew*>(e)) {


    BType kt = mn->keyType, vt = mn->valType;
    long long kw = fieldByteWidth(kt), vw = fieldByteWidth(vt), kk = keyCategory(kt);
    if (kw <= 0) kw = 8;
    if (vw <= 0) vw = 8;
    usedMap_ = true;
    collectStruct(kt); collectStruct(vt);
    std::string r = freshLocal("mnew");
    out_ << "  " << r << " = call i8* @ox_map_new(i64 " << kw << ", i64 " << vw
         << ", i64 " << kk << ")\n";
    return {r, makeMapType(kt, vt)};
  }
  if (auto sn = dynamic_cast<SetNew*>(e)) {
    BType et = sn->elemType;
    long long kw = fieldByteWidth(et), kk = keyCategory(et);
    if (kw <= 0) kw = 8;
    usedSet_ = true; collectStruct(et);
    std::string r = freshLocal("snew");
    out_ << "  " << r << " = call i8* @ox_set_new(i64 " << kw << ", i64 " << kk << ")\n";
    return {r, makeSetType(et)};
  }
  if (auto c = dynamic_cast<Call*>(e)) {


    if (c->fnPtr && c->calleeExpr) {
      auto it = sema_.funcs.end();
      BType fnType = BType::void_;


      if (auto v = dynamic_cast<VarRef*>(c->calleeExpr.get())) {
        BType vt = BType::void_;

        for (auto rit = scopes_.rbegin(); rit != scopes_.rend(); ++rit) {
          auto f = rit->find(v->name);
          if (f != rit->end()) { vt = f->second.second; break; }
        }
        if (vt.tag != BType::Tag::fn_ && sema_.globals.count(v->name))
          vt = sema_.globals.at(v->name).type;
        if (vt.tag == BType::Tag::fn_) fnType = vt;
      }
      BType retT = fnRet(fnType);
      BType retEff = (retT == BType::void_) ? BType::i64 : retT;
      std::string retIr = (retT == BType::void_) ? "void" : typeStr(retT);

      auto [fv, fvt] = genExpr(c->calleeExpr.get());
      std::string ptr = fv;
      if (fvt.tag != BType::Tag::ptr && fvt.tag != BType::Tag::fn_) {

        std::string p = freshLocal("fnp");
        out_ << "  " << p << " = inttoptr i64 " << fv << " to i8*\n";
        ptr = p;
      }

      const auto& ps = fnParams(fnType);
      std::ostringstream fnty;
      fnty << retIr << " (";
      for (size_t i = 0; i < ps.size(); i++) { if (i) fnty << ", "; fnty << typeStr(ps[i]); }
      fnty << ")*";

      std::string cp = freshLocal("fntype");
      out_ << "  " << cp << " = bitcast i8* " << ptr << " to " << fnty.str() << "\n";

      std::ostringstream args;
      for (size_t k = 0; k < c->args.size() && k < ps.size(); k++) {
        BType pt = ps[k];
        if (pt.tag == BType::Tag::ptr && isLvalueExpr(c->args[k].get())) {
          auto [addr, bt] = genAddr(c->args[k].get());
          if (!addr.empty() && bt == pointee(pt)) {
            std::string cv = genCoerce(addr, bt, pt);
            if (k) args << ", ";
            args << typeStr(pt) << " " << cv;
            continue;
          }
        }
        auto [v, vt] = genExpr(c->args[k].get());
        std::string cv = genCoerce(v, vt, pt);
        if (k) args << ", ";
        args << typeStr(pt) << " " << cv;
      }
      if (retT == BType::void_) {
        out_ << "  call void " << fnty.str() << " " << cp << "(" << args.str() << ")\n";
        return {"", BType::void_};
      }
      std::string r = tmp();
      out_ << "  " << r << " = call " << retIr << " " << fnty.str() << " " << cp
           << "(" << args.str() << ")\n";
      return {r, retT};
    }
    if (c->isPrint) {
      std::string sp = strConst(" ");
      for (size_t i = 0; i < c->args.size(); i++) {
        if (i) out_ << "  call i32 @ox_puts(i8* " << sp << ")\n";
        auto [v, vt] = genExpr(c->args[i].get());
        printValue(v, vt);
      }
      out_ << "  call i32 @ox_newline()\n";
      return {"", BType::void_};
    }

    if (!c->isPrint && !sema_.funcs.count(c->callee)) {
      auto r = lowerBuiltin(c);
      if (r.first != "|||no|||") return {r.first, r.second};
    }
    auto it = sema_.funcs.find(c->callee);
    if (it == sema_.funcs.end()) return {"", BType::void_};
    std::ostringstream args;
    for (size_t k = 0; k < c->args.size() && k < it->second.paramTypes.size(); k++) {
      BType pt = it->second.paramTypes[k];


      if (pt.tag == BType::Tag::ptr && isLvalueExpr(c->args[k].get())) {

        auto [addr, bt] = genAddr(c->args[k].get());
        if (!addr.empty() && bt == pointee(pt)) {
          std::string cv = genCoerce(addr, bt, pt);
          if (k) args << ", ";
          args << typeStr(pt) << " " << cv;
          continue;
        }
      }
      auto [v, vt] = genExpr(c->args[k].get());
      std::string cv = genCoerce(v, vt, pt);
      if (k) args << ", ";
      args << typeStr(pt) << " " << cv;
    }
    if (it->second.retType == BType::void_) {
      out_ << "  call void @" << c->callee << "(" << args.str() << ")\n";
      return {"", BType::void_};
    }
    std::string r = tmp();
    out_ << "  " << r << " = call " << typeStr(it->second.retType) << " @" << c->callee
         << "(" << args.str() << ")\n";
    return {r, it->second.retType};
  }
  if (auto mc = dynamic_cast<MethodCall*>(e)) {
    return emitMethodCall(mc->recvType.structName, mc->callee, mc->recvType,
                          mc->receiverByRef, mc->receiver.get(), mc->args);
  }
  if (auto ac = dynamic_cast<AssocCall*>(e)) {

    std::string sn = ac->typeName;
    if (isAliasName(sn)) sn = resolveAlias(sn).second.structName;
    auto sit = sema_.methods.find(sn);
    if (sit == sema_.methods.end() || !sit->second.count(ac->callee))
      return {"0", BType::void_};
    const MethodInfo& mi = sit->second.at(ac->callee);
    std::ostringstream args;
    bool first = true;
    for (size_t k = 0; k < ac->args.size() && k < mi.paramTypes.size(); k++) {
      BType pt = mi.paramTypes[k];
      std::string argStr;
      if (pt.tag == BType::Tag::ptr && isLvalueExpr(ac->args[k].get())) {
        auto [addr, bt] = genAddr(ac->args[k].get());
        if (!addr.empty() && bt == pointee(pt)) argStr = genCoerce(addr, bt, pt);
        else { auto [v, vt] = genExpr(ac->args[k].get()); argStr = genCoerce(v, vt, pt); }
      } else {
        auto [v, vt] = genExpr(ac->args[k].get());
        argStr = genCoerce(v, vt, pt);
      }
      if (!first) args << ", ";
      first = false;
      args << typeStr(pt) << " " << argStr;
    }
    if (mi.retType == BType::void_) {
      out_ << "  call void @" << mi.mangled << "(" << args.str() << ")\n";
      return {"", BType::void_};
    }
    std::string r = tmp();
    out_ << "  " << r << " = call " << typeStr(mi.retType) << " @" << mi.mangled
         << "(" << args.str() << ")\n";
    return {r, mi.retType};
  }
  if (auto lam = dynamic_cast<LambdaLit*>(e)) {


    genLambda(lam);


    std::string p = freshLocal("lam");
    out_ << "  " << p << " = bitcast " << typeStr(fnRet(lam->fnType)) << " (";
    const auto& ps = fnParams(lam->fnType);
    for (size_t i = 0; i < ps.size(); i++) { if (i) out_ << ", "; out_ << typeStr(ps[i]); }
    out_ << ")* @" << lam->loweredName << " to i8*\n";
    return {p, lam->fnType};
  }
  return {"0", BType::i64};
}


std::pair<std::string, BType> IRGen::emitMethodCall(const std::string& structName,
                                                    const std::string& methodName,
                                                    const BType& recvType, bool recvByRef,
                                                    Expr* receiver,
                                                    const std::vector<ExprPtr>& args) {
  auto sit = sema_.methods.find(structName);
  if (sit == sema_.methods.end() || !sit->second.count(methodName))
    return {"0", BType::void_};
  const MethodInfo& mi = sit->second.at(methodName);
  BType st; st.tag = BType::Tag::struct_; st.structName = structName;
  collectStruct(st);

  std::ostringstream argStr;
  bool first = true;

  std::string recvVal;
  if (recvByRef) {

    auto [addr, bt] = genAddr(receiver);
    if (!addr.empty() && bt.tag == BType::Tag::struct_ && bt.structName == structName) {
      recvVal = addr;
    } else if (!addr.empty() && bt.tag == BType::Tag::ptr &&
               pointee(bt).tag == BType::Tag::struct_ &&
               pointee(bt).structName == structName) {
      recvVal = addr;
    } else {


      auto [v, vt2] = genExpr(receiver);
      std::string a = freshLocal("mrecv");
      out_ << "  " << a << " = alloca " << typeStr(st) << "\n";
      std::string av = genCoerce(v, vt2, st);
      out_ << "  store " << typeStr(st) << " " << av << ", " << typeStr(st) << "* " << a << "\n";
      recvVal = a;
    }
    argStr << typeStr(makePtr(st)) << " " << recvVal;
    first = false;
  } else {

    auto [val, vt] = genExpr(receiver);
    recvVal = genCoerce(val, vt, st);
    argStr << typeStr(st) << " " << recvVal;
    first = false;
  }


  for (size_t k = 0; k < args.size() && k < mi.paramTypes.size(); k++) {
    BType pt = mi.paramTypes[k];
    std::string s;
    if (pt.tag == BType::Tag::ptr && isLvalueExpr(args[k].get())) {
      auto [addr, bt] = genAddr(args[k].get());
      if (!addr.empty() && bt == pointee(pt)) s = genCoerce(addr, bt, pt);
      else { auto [v, vt] = genExpr(args[k].get()); s = genCoerce(v, vt, pt); }
    } else {
      auto [v, vt] = genExpr(args[k].get());
      s = genCoerce(v, vt, pt);
    }
    if (!first) argStr << ", ";
    first = false;
    argStr << typeStr(pt) << " " << s;
  }

  if (mi.retType == BType::void_) {
    out_ << "  call void @" << mi.mangled << "(" << argStr.str() << ")\n";
    return {"", BType::void_};
  }
  std::string r = tmp();
  out_ << "  " << r << " = call " << typeStr(mi.retType) << " @" << mi.mangled
       << "(" << argStr.str() << ")\n";
  return {r, mi.retType};
}


std::pair<std::string, BType> IRGen::emitOverloadCall(const std::string& structName,
                                                      const std::string& methodName,
                                                      const BType& recvType, bool recvByRef,
                                                      Expr* receiver,
                                                      const std::vector<ExprPtr>& args,
                                                      bool negateResult) {


  bool byRef = recvByRef;
  auto sit = sema_.methods.find(structName);
  if (sit != sema_.methods.end() && sit->second.count(methodName))
    byRef = sit->second.at(methodName).selfByRef;
  auto [val, rt] = emitMethodCall(structName, methodName, recvType, byRef,
                                  receiver, args);
  if (negateResult) {

    std::string neg = freshLocal("olneg");
    out_ << "  " << neg << " = xor i1 " << val << ", true\n";
    return {neg, BType::bool_};
  }
  return {val, rt};
}

void IRGen::printValue(const std::string& val, const BType& t, const std::string& prefix) {
  (void)prefix;
  if (t == BType::void_) return;
  if (t == BType::char_) {
    std::string ext = freshLocal("cext");
    out_ << "  " << ext << " = zext i8 " << val << " to i64\n";
    out_ << "  call i32 @ox_putc(i64 " << ext << ")\n";
    return;
  }
  if (t.tag == BType::Tag::enum_) {


    EnumDef* ed = findEnum(t.structName);
    if (ed && !ed->variants.empty()) {

      std::string v = genCoerce(val, t, BType::i64);
      std::string doneBB = freshLabel("en_done");
      for (size_t i = 0; i < ed->variants.size(); i++) {
        std::string nv = freshLocal("enchk");
        out_ << "  " << nv << " = icmp eq i64 " << v << ", " << i << "\n";
        std::string yesBB = freshLabel("en_yes");
        std::string noBB = freshLabel("en_no");
        branch(nv, yesBB, noBB);
        beginBlock(yesBB);
        std::string nm = strConst(ed->variants[i]);
        out_ << "  call i32 @ox_puts(i8* " << nm << ")\n";
        jump(doneBB);
        beginBlock(noBB);
      }

      out_ << "  call i32 @ox_puti(i64 " << v << ")\n";
      jump(doneBB);
      beginBlock(doneBB);
      return;
    }

    std::string cv = genCoerce(val, t, BType::i64);
    out_ << "  call i32 @ox_puti(i64 " << cv << ")\n";
    return;
  }
  if (isInt(t)) {

    std::string cv = genCoerce(val, t, BType::i64);
    out_ << "  call i32 @ox_puti(i64 " << cv << ")\n";
    return;
  }


  if (t == BType::f64) { out_ << "  call i32 @ox_putf(double " << val << ")\n"; return; }
  if (t == BType::f32) {
    std::string dv = genCoerce(val, BType::f32, BType::f64);
    out_ << "  call i32 @ox_putf(double " << dv << ")\n";
    return;
  }
  if (t == BType::bool_) {
    std::string ext = freshLocal("bext");
    out_ << "  " << ext << " = zext i1 " << val << " to i64\n";
    out_ << "  call i32 @ox_puti(i64 " << ext << ")\n";
    return;
  }
  if (t.tag == BType::Tag::ptr) {

    std::string cv = genCoerce(val, t, BType::i64);
    out_ << "  call i32 @ox_puti(i64 " << cv << ")\n";
    return;
  }
  if (t == BType::str) { out_ << "  call i32 @ox_puts(i8* " << val << ")\n"; return; }
  if (t.tag == BType::Tag::array) {

    std::string a = freshLocal("pa");
    out_ << "  " << a << " = alloca " << typeStr(t) << "\n";
    out_ << "  store " << typeStr(t) << " " << val << ", " << typeStr(t) << "* " << a << "\n";
    std::string lb = strConst("[");
    out_ << "  call i32 @ox_puts(i8* " << lb << ")\n";
    const BType& et = arrayElem(t);
    for (int32_t i = 0; i < t.count; i++) {
      std::string sep;
      if (i) { sep = strConst(", "); out_ << "  call i32 @ox_puts(i8* " << sep << ")\n"; }
      std::string ep = freshLocal("pe");
      out_ << "  " << ep << " = getelementptr inbounds " << typeStr(t) << ", "
           << typeStr(t) << "* " << a << ", i64 0, i64 " << i << "\n";
      std::string ev = freshLocal("ev");
      out_ << "  " << ev << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << ep << "\n";
      printValue(ev, et);
    }
    std::string rb = strConst("]");
    out_ << "  call i32 @ox_puts(i8* " << rb << ")\n";
    return;
  }
  if (t.tag == BType::Tag::struct_) {
    StructDef* d = findStruct(t.structName);
    std::string hdr = strConst(t.structName + "{");
    out_ << "  call i32 @ox_puts(i8* " << hdr << ")\n";
    std::string a = freshLocal("ps");
    out_ << "  " << a << " = alloca " << typeStr(t) << "\n";
    out_ << "  store " << typeStr(t) << " " << val << ", " << typeStr(t) << "* " << a << "\n";
    if (d) {
      for (size_t i = 0; i < d->fields.size(); i++) {
        std::string sep;
        if (i) { sep = strConst(", "); out_ << "  call i32 @ox_puts(i8* " << sep << ")\n"; }
        std::string fn = strConst(d->fields[i].name + ": ");
        out_ << "  call i32 @ox_puts(i8* " << fn << ")\n";
        const BType& ft = d->fields[i].type;
        std::string fp = freshLocal("pf");
        out_ << "  " << fp << " = getelementptr inbounds " << typeStr(t) << ", "
             << typeStr(t) << "* " << a << ", i64 0, i32 " << i << "\n";
        std::string fv = freshLocal("fv");
        out_ << "  " << fv << " = load " << typeStr(ft) << ", " << typeStr(ft) << "* " << fp << "\n";
        printValue(fv, ft);
      }
    }
    std::string rb = strConst("}");
    out_ << "  call i32 @ox_puts(i8* " << rb << ")\n";
    return;
  }
  if (t.tag == BType::Tag::dynarray) {
    BType et = dynArrayElem(t);
    std::string sx = elemSuffix(et);
    if (!sx.empty()) {
      usedVec_.insert(sx);
      out_ << "  call void @ox_vec_print_" << sx << "(i8* " << val << ")\n";
      return;
    }


    usedVec_blob_ = true;
    int32_t esz = fieldByteWidth(et);
    std::string len = freshLocal("vpl");
    out_ << "  " << len << " = call i64 @ox_vec_len(i8* " << val << ")\n";
    std::string iSlot = freshLocal("vpi");
    out_ << "  " << iSlot << " = alloca i64\n";
    out_ << "  store i64 0, i64* " << iSlot << "\n";
    std::string lb = strConst("[");
    out_ << "  call i32 @ox_puts(i8* " << lb << ")\n";
    std::string condBB = freshLabel("vpc");
    std::string bodyBB = freshLabel("vpb");
    std::string stepBB = freshLabel("vps");
    std::string endBB = freshLabel("vpe");
    jump(condBB);
    beginBlock(condBB);
    {
      std::string i = freshLocal("vpil");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string cmp = freshLocal("vpc2");
      out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << len << "\n";
      branch(cmp, bodyBB, endBB);
    }
    beginBlock(bodyBB);
    {
      std::string i = freshLocal("vpib");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";

      std::string skipBB = freshLabel("vpskp");
      std::string sepsz = freshLocal("vpne");
      out_ << "  " << sepsz << " = icmp eq i64 " << i << ", 0\n";
      std::string bodyCont = freshLabel("vpbd");
      branch(sepsz, skipBB, bodyCont);
      beginBlock(bodyCont);
      {
        std::string sepStr = strConst(", ");
        out_ << "  call i32 @ox_puts(i8* " << sepStr << ")\n";
      }
      jump(skipBB);
      beginBlock(skipBB);

      std::string ep = freshLocal("vpp");
      out_ << "  " << ep << " = call i8* @ox_vec_blob_ptr(i8* " << val
           << ", i64 " << i << ", i64 " << (esz > 0 ? esz : 8) << ")\n";
      std::string ev = freshLocal("vpv");
      if (et.tag == BType::Tag::dynarray || et.tag == BType::Tag::ptr) {
        out_ << "  " << ev << " = load " << typeStr(et) << ", i8* " << ep << "\n";
      } else {
        std::string tp = freshLocal("vptc");
        out_ << "  " << tp << " = bitcast i8* " << ep << " to " << typeStr(et) << "*\n";
        out_ << "  " << ev << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << tp << "\n";
      }
      printValue(ev, et);
    }
    jump(stepBB);
    beginBlock(stepBB);
    {
      std::string i = freshLocal("vpis");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ni = freshLocal("vpin");
      out_ << "  " << ni << " = add i64 " << i << ", 1\n";
      out_ << "  store i64 " << ni << ", i64* " << iSlot << "\n";
    }
    jump(condBB);
    beginBlock(endBB);
    std::string rb = strConst("]");
    out_ << "  call i32 @ox_puts(i8* " << rb << ")\n";
    return;
  }
  if (t.tag == BType::Tag::map_) {


    usedMap_ = true;
    BType keyT = mapKeyType(t), valT = mapValType(t);
    collectStruct(keyT); collectStruct(valT);
    std::string len = freshLocal("mpl");
    out_ << "  " << len << " = call i64 @ox_map_len(i8* " << val << ")\n";
    std::string iSlot = freshLocal("mpi");
    out_ << "  " << iSlot << " = alloca i64\n";
    out_ << "  store i64 0, i64* " << iSlot << "\n";
    std::string lb = strConst("{");
    out_ << "  call i32 @ox_puts(i8* " << lb << ")\n";
    std::string condBB = freshLabel("mpc"), bodyBB = freshLabel("mpb"),
                stepBB = freshLabel("mps"), endBB = freshLabel("mpe");
    jump(condBB); beginBlock(condBB);
    {
      std::string i = freshLocal("mpcl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string cmp = freshLocal("mpcc");
      out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << len << "\n";
      branch(cmp, bodyBB, endBB);
    }
    beginBlock(bodyBB);
    {
      std::string i = freshLocal("mpbl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";

      std::string skipBB = freshLabel("mpsk");
      std::string sepsz = freshLocal("mpne");
      out_ << "  " << sepsz << " = icmp eq i64 " << i << ", 0\n";
      std::string cont = freshLabel("mpbd");
      branch(sepsz, skipBB, cont); beginBlock(cont);
      { std::string sep = strConst(", "); out_ << "  call i32 @ox_puts(i8* " << sep << ")\n"; }
      jump(skipBB); beginBlock(skipBB);

      std::string kp = freshLocal("mpkp");
      out_ << "  " << kp << " = call i8* @ox_map_key_ptr(i8* " << val << ", i64 " << i << ")\n";
      std::string kv = loadScratch(kp, keyT);
      printValue(kv, keyT);
      std::string colon = strConst(": ");
      out_ << "  call i32 @ox_puts(i8* " << colon << ")\n";

      std::string vslot = freshLocal("mpvs");
      out_ << "  " << vslot << " = alloca " << typeStr(valT) << "\n";
      out_ << "  store " << typeStr(valT) << " " << zeroVal(valT) << ", " << typeStr(valT)
           << "* " << vslot << "\n";
      std::string vptr = freshLocal("mpvp");
      out_ << "  " << vptr << " = bitcast " << typeStr(valT) << "* " << vslot << " to i8*\n";
      out_ << "  call i64 @ox_map_get(i8* " << val << ", i8* " << kp << ", i8* " << vptr << ")\n";
      std::string vv = loadScratch(vptr, valT);
      printValue(vv, valT);
    }
    jump(stepBB); beginBlock(stepBB);
    {
      std::string i = freshLocal("mpsl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ni = freshLocal("mpn");
      out_ << "  " << ni << " = add i64 " << i << ", 1\n";
      out_ << "  store i64 " << ni << ", i64* " << iSlot << "\n";
    }
    jump(condBB);
    beginBlock(endBB);
    std::string rb = strConst("}");
    out_ << "  call i32 @ox_puts(i8* " << rb << ")\n";
    return;
  }
  if (t.tag == BType::Tag::set_) {

    usedSet_ = true;
    BType et = setElemType(t);
    collectStruct(et);
    std::string len = freshLocal("spl");
    out_ << "  " << len << " = call i64 @ox_set_len(i8* " << val << ")\n";
    std::string iSlot = freshLocal("spi");
    out_ << "  " << iSlot << " = alloca i64\n";
    out_ << "  store i64 0, i64* " << iSlot << "\n";
    std::string lb = strConst("{");
    out_ << "  call i32 @ox_puts(i8* " << lb << ")\n";
    std::string condBB = freshLabel("spc"), bodyBB = freshLabel("spb"),
                stepBB = freshLabel("sps"), endBB = freshLabel("spe");
    jump(condBB); beginBlock(condBB);
    {
      std::string i = freshLocal("spcl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string cmp = freshLocal("spcc");
      out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << len << "\n";
      branch(cmp, bodyBB, endBB);
    }
    beginBlock(bodyBB);
    {
      std::string i = freshLocal("spbl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string skipBB = freshLabel("spsk");
      std::string sepsz = freshLocal("spne");
      out_ << "  " << sepsz << " = icmp eq i64 " << i << ", 0\n";
      std::string cont = freshLabel("spbd");
      branch(sepsz, skipBB, cont); beginBlock(cont);
      { std::string sep = strConst(", "); out_ << "  call i32 @ox_puts(i8* " << sep << ")\n"; }
      jump(skipBB); beginBlock(skipBB);
      std::string ep = freshLocal("spep");
      out_ << "  " << ep << " = call i8* @ox_set_ptr(i8* " << val << ", i64 " << i << ")\n";
      std::string ev = loadScratch(ep, et);
      printValue(ev, et);
    }
    jump(stepBB); beginBlock(stepBB);
    {
      std::string i = freshLocal("spsl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ni = freshLocal("spn");
      out_ << "  " << ni << " = add i64 " << i << ", 1\n";
      out_ << "  store i64 " << ni << ", i64* " << iSlot << "\n";
    }
    jump(condBB);
    beginBlock(endBB);
    std::string rb = strConst("}");
    out_ << "  call i32 @ox_puts(i8* " << rb << ")\n";
    return;
  }
}

std::string IRGen::strConst(const std::string& s) {
  std::string g = freshGlobal("str");
  std::string data = esc(s);
  size_t n = s.size() + 1;
  globals_ << g << " = private constant [" << n << " x i8] c\"" << data << "\\00\"\n";
  std::string r = freshLocal("sp");
  out_ << "  " << r << " = getelementptr inbounds [" << n << " x i8], [" << n << " x i8]* "
       << g << ", i64 0, i64 0\n";
  return r;
}

std::string IRGen::spillScratch(const std::string& v, const BType& t) {
  std::string slot = freshLocal("scr");
  out_ << "  " << slot << " = alloca " << typeStr(t) << "\n";
  out_ << "  store " << typeStr(t) << " " << v << ", " << typeStr(t) << "* " << slot << "\n";

  std::string p = freshLocal("scrp");
  out_ << "  " << p << " = bitcast " << typeStr(t) << "* " << slot << " to i8*\n";
  return p;
}

std::string IRGen::loadScratch(const std::string& ptr, const BType& t) {
  if (t.tag == BType::Tag::dynarray || t.tag == BType::Tag::ptr ||
      t.tag == BType::Tag::str) {

    std::string r = freshLocal("scl");
    out_ << "  " << r << " = load " << typeStr(t) << ", i8* " << ptr << "\n";
    return r;
  }
  std::string tp = freshLocal("sct");
  out_ << "  " << tp << " = bitcast i8* " << ptr << " to " << typeStr(t) << "*\n";
  std::string r = freshLocal("scl");
  out_ << "  " << r << " = load " << typeStr(t) << ", " << typeStr(t) << "* " << tp << "\n";
  return r;
}


void IRGen::ensureIntrinsic(const std::string& decl, const std::string& name) {
  if (declaredIntrinsics_.count(name)) return;
  declaredIntrinsics_.insert(name);
  globals_ << decl << "\n";
}

std::pair<std::string, BType> IRGen::lowerBuiltin(Call* c) {
  auto emitArgVal = [&](size_t i) -> std::pair<std::string, BType> {
    if (i >= c->args.size()) return {"", BType::void_};
    return genExpr(c->args[i].get());
  };
  const std::string& n = c->callee;


  if (n == "mmio_load") {
    auto [p, pt] = emitArgVal(0);
    BType elem = (pt.tag == BType::Tag::ptr) ? pointee(pt) : BType::i64;
    std::string r = freshLocal("mmio");
    out_ << "  " << r << " = load volatile " << typeStr(elem) << ", "
         << typeStr(makePtr(elem)) << " " << p << "\n";
    return {r, elem};
  }

  if (n == "mmio_store") {
    auto [p, pt] = emitArgVal(0);
    auto [v, vt] = emitArgVal(1);
    BType elem = (pt.tag == BType::Tag::ptr) ? pointee(pt) : vt;
    std::string cv = genCoerce(v, vt, elem);
    out_ << "  store volatile " << typeStr(elem) << " " << cv << ", "
         << typeStr(makePtr(elem)) << " " << p << "\n";
    return {"", BType::void_};
  }

  if (n == "memset") {
    auto [dp, dpt] = emitArgVal(0);
    auto [fv, fvt] = emitArgVal(1);
    auto [cv, cvt] = emitArgVal(2);
    ensureIntrinsic("declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)",
                    "@llvm.memset.p0i8.i64");
    std::string d = freshLocal("msetp");
    out_ << "  " << d << " = bitcast " << typeStr(dpt) << " " << dp << " to i8*\n";
    std::string fill = genCoerce(fv, fvt, BType::u8);
    std::string cnt = genCoerce(cv, cvt, BType::i64);
    out_ << "  call void @llvm.memset.p0i8.i64(i8* " << d << ", i8 " << fill
         << ", i64 " << cnt << ", i1 false)\n";
    return {"", BType::void_};
  }

  if (n == "memcpy") {
    auto [dp, dpt] = emitArgVal(0);
    auto [sp, spt] = emitArgVal(1);
    auto [cv, cvt] = emitArgVal(2);
    ensureIntrinsic("declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)",
                    "@llvm.memcpy.p0i8.p0i8.i64");
    std::string d = freshLocal("mcypd"), s = freshLocal("mcyps");
    out_ << "  " << d << " = bitcast " << typeStr(dpt) << " " << dp << " to i8*\n";
    out_ << "  " << s << " = bitcast " << typeStr(spt) << " " << sp << " to i8*\n";
    std::string cnt = genCoerce(cv, cvt, BType::i64);
    out_ << "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* " << d << ", i8* " << s
         << ", i64 " << cnt << ", i1 false)\n";
    return {"", BType::void_};
  }


  if (n == "str_ptr") {
    auto [sv, svt] = emitArgVal(0);


    std::string p = genCoerce(sv, svt, makePtr(BType::u8));
    return {p, makePtr(BType::u8)};
  }

  if (n == "abs") {
    auto [v, vt] = emitArgVal(0);

    if (vt == BType::f64) {
      std::string r = freshLocal("abs");
      out_ << "  " << r << " = call double @llvm.fabs.f64(double " << v << ")\n";
      return {r, BType::f64};
    }

    if (isInt(vt) && isSignedInt(vt)) {
      std::string wide = genCoerce(v, vt, BType::i64);
      std::string a = freshLocal("abs");
      out_ << "  " << a << " = call i64 @ox_abs_i64(i64 " << wide << ")\n";


      std::string r = genCoerce(a, BType::i64, vt);
      return {r, vt};
    }


    if (isInt(vt)) {
      std::string r = freshLocal("abs");
      std::string ity = intIrTy(vt);
      out_ << "  " << r << " = add " << ity << " 0, " << v << "\n";
      return {r, vt};
    }
    return {"0", BType::i64};
  }
  if (n == "sqrt") {
    auto [v, vt] = emitArgVal(0);
    if (vt == BType::f64) {
      std::string r = freshLocal("sqrt");
      out_ << "  " << r << " = call double @ox_sqrt(double " << v << ")\n";
      return {r, BType::f64};
    }
    return {"0.0", BType::f64};
  }
  if (n == "imin") {
    auto [a, at] = emitArgVal(0); auto [b, bt] = emitArgVal(1);
    std::string r = freshLocal("imin");
    out_ << "  " << r << " = call i64 @ox_imin(i64 " << a << ", i64 " << b << ")\n";
    return {r, BType::i64};
  }
  if (n == "imax") {
    auto [a, at] = emitArgVal(0); auto [b, bt] = emitArgVal(1);
    std::string r = freshLocal("imax");
    out_ << "  " << r << " = call i64 @ox_imax(i64 " << a << ", i64 " << b << ")\n";
    return {r, BType::i64};
  }
  if (n == "fmin") {
    auto [a, at] = emitArgVal(0); auto [b, bt] = emitArgVal(1);
    std::string r = freshLocal("fmin");
    out_ << "  " << r << " = call double @ox_fmin2(double " << a << ", double " << b << ")\n";
    return {r, BType::f64};
  }
  if (n == "fmax") {
    auto [a, at] = emitArgVal(0); auto [b, bt] = emitArgVal(1);
    std::string r = freshLocal("fmax");
    out_ << "  " << r << " = call double @ox_fmax2(double " << a << ", double " << b << ")\n";
    return {r, BType::f64};
  }
  if (n == "itos") {
    auto [v, vt] = emitArgVal(0);
    std::string r = freshLocal("itos");
    out_ << "  " << r << " = call i8* @ox_itos(i64 " << v << ")\n";
    return {r, BType::str};
  }
  if (n == "stoi") {
    auto [v, vt] = emitArgVal(0);
    std::string r = freshLocal("stoi");
    out_ << "  " << r << " = call i64 @ox_stoi(i8* " << v << ")\n";
    return {r, BType::i64};
  }
  if (n == "stod") {
    auto [v, vt] = emitArgVal(0);
    std::string r = freshLocal("stod");
    out_ << "  " << r << " = call double @ox_stod(i8* " << v << ")\n";
    return {r, BType::f64};
  }
  if (n == "ftos") {
    auto [v, vt] = emitArgVal(0);
    std::string r = freshLocal("ftos");
    out_ << "  " << r << " = call i8* @ox_ftos(double " << v << ")\n";
    return {r, BType::str};
  }
  if (n == "char_to_str") {
    auto [v, vt] = emitArgVal(0);

    std::string c = genCoerce(v, vt, BType::i64);
    std::string r = freshLocal("cts");
    out_ << "  " << r << " = call i8* @ox_char_str(i64 " << c << ")\n";
    return {r, BType::str};
  }
  if (n == "substr") {
    auto [s, st] = emitArgVal(0);
    auto [a, at] = emitArgVal(1);
    auto [b, bt] = emitArgVal(2);
    std::string ai = genCoerce(a, at, BType::i64);
    std::string bi = genCoerce(b, bt, BType::i64);
    std::string r = freshLocal("substr");
    out_ << "  " << r << " = call i8* @ox_substr(i8* " << s << ", i64 " << ai
         << ", i64 " << bi << ")\n";
    return {r, BType::str};
  }
  if (n == "index_of") {
    auto [s, st] = emitArgVal(0);
    auto [c, ct] = emitArgVal(1);
    std::string ci = genCoerce(c, ct, BType::i64);
    std::string r = freshLocal("indexof");
    out_ << "  " << r << " = call i64 @ox_strchr(i8* " << s << ", i64 " << ci << ")\n";
    return {r, BType::i64};
  }
  if (n == "read_line") {
    std::string r = freshLocal("rl");
    out_ << "  " << r << " = call i8* @ox_read_line()\n";
    return {r, BType::str};
  }
  if (n == "read_file") {
    auto [v, vt] = emitArgVal(0);
    std::string r = freshLocal("rf");
    out_ << "  " << r << " = call i8* @ox_read_file(i8* " << v << ")\n";
    return {r, BType::str};
  }
  if (n == "file_open") {
    auto [p, pt] = emitArgVal(0); auto [m, mt] = emitArgVal(1);
    std::string r = freshLocal("fopen");
    out_ << "  " << r << " = call i64 @ox_file_open(i8* " << p << ", i8* " << m << ")\n";
    return {r, BType::i64};
  }
  if (n == "file_close") {
    auto [h, ht] = emitArgVal(0);
    std::string r = freshLocal("fclose");
    out_ << "  " << r << " = call i64 @ox_file_close(i64 " << h << ")\n";
    return {r, BType::i64};
  }
  if (n == "file_read") {
    auto [h, ht] = emitArgVal(0);
    std::string r = freshLocal("fread");
    out_ << "  " << r << " = call i8* @ox_file_read(i64 " << h << ")\n";
    return {r, BType::str};
  }
  if (n == "file_write") {
    auto [h, ht] = emitArgVal(0); auto [s, st] = emitArgVal(1);
    std::string r = freshLocal("fwrite");
    out_ << "  " << r << " = call i64 @ox_file_write(i64 " << h << ", i8* " << s << ")\n";
    return {r, BType::i64};
  }
  if (n == "file_exists") {
    auto [p, pt] = emitArgVal(0);
    std::string r = freshLocal("fexists");
    out_ << "  " << r << " = call i1 @ox_file_exists(i8* " << p << ")\n";
    return {r, BType::bool_};
  }

  if (n == "map_len") {
    auto [h, ht] = emitArgVal(0);
    usedMap_ = true;
    std::string r = freshLocal("mlen");
    out_ << "  " << r << " = call i64 @ox_map_len(i8* " << h << ")\n";
    return {r, BType::i64};
  }
  if (n == "map_contains") {
    auto [h, ht] = emitArgVal(0); auto [k, kt] = emitArgVal(1);
    usedMap_ = true; collectStruct(mapKeyType(ht));
    BType keyT = mapKeyType(ht);
    std::string kp = spillScratch(genCoerce(k, kt, keyT), keyT);
    std::string raw = freshLocal("mcont");
    out_ << "  " << raw << " = call i64 @ox_map_contains(i8* " << h << ", i8* " << kp << ")\n";
    std::string r = freshLocal("mcontb");
    out_ << "  " << r << " = icmp ne i64 " << raw << ", 0\n";
    return {r, BType::bool_};
  }
  if (n == "map_set") {
    auto [h, ht] = emitArgVal(0); auto [k, kt] = emitArgVal(1); auto [vp, vpt] = emitArgVal(2);
    usedMap_ = true; collectStruct(mapKeyType(ht)); collectStruct(mapValType(ht));
    BType keyT = mapKeyType(ht), valT = mapValType(ht);
    std::string kp = spillScratch(genCoerce(k, kt, keyT), keyT);
    std::string vpScratch = spillScratch(genCoerce(vp, vpt, valT), valT);
    out_ << "  call void @ox_map_set(i8* " << h << ", i8* " << kp << ", i8* "
         << vpScratch << ")\n";
    return {"", BType::void_};
  }
  if (n == "map_get") {
    auto [h, ht] = emitArgVal(0); auto [k, kt] = emitArgVal(1);
    usedMap_ = true; collectStruct(mapKeyType(ht)); collectStruct(mapValType(ht));
    BType keyT = mapKeyType(ht), valT = mapValType(ht);
    std::string kp = spillScratch(genCoerce(k, kt, keyT), keyT);

    std::string vslot = freshLocal("mgvslot");
    out_ << "  " << vslot << " = alloca " << typeStr(valT) << "\n";
    out_ << "  store " << typeStr(valT) << " " << zeroVal(valT) << ", " << typeStr(valT)
         << "* " << vslot << "\n";
    std::string vptr = freshLocal("mgvp");
    out_ << "  " << vptr << " = bitcast " << typeStr(valT) << "* " << vslot << " to i8*\n";
    out_ << "  call i64 @ox_map_get(i8* " << h << ", i8* " << kp << ", i8* " << vptr << ")\n";
    std::string r = loadScratch(vptr, valT);
    return {r, valT};
  }
  if (n == "map_keys") {


    auto [h, ht] = emitArgVal(0);
    usedMap_ = true; collectStruct(mapKeyType(ht));
    BType keyT = mapKeyType(ht);
    std::string sx = elemSuffix(keyT);
    BType vecT = makeDynArray(keyT);
    std::string out;
    if (!sx.empty()) {
      usedVec_.insert(sx);
      out = freshLocal("mkvec");
      out_ << "  " << out << " = call i8* @ox_vec_new_" << sx << "()\n";
    } else {
      usedVec_blob_ = true;
      out = freshLocal("mkvec");
      int32_t esz = fieldByteWidth(keyT);
      out_ << "  " << out << " = call i8* @ox_vec_blob_new(i64 "
           << (esz > 0 ? esz : 8) << ")\n";
    }


    std::string len = freshLocal("mklen");
    out_ << "  " << len << " = call i64 @ox_map_len(i8* " << h << ")\n";
    std::string iSlot = freshLocal("mki");
    out_ << "  " << iSlot << " = alloca i64\n";
    out_ << "  store i64 0, i64* " << iSlot << "\n";
    std::string condBB = freshLabel("mkc"), bodyBB = freshLabel("mkb"),
                stepBB = freshLabel("mks"), endBB = freshLabel("mke");
    jump(condBB); beginBlock(condBB);
    {
      std::string i = freshLocal("mkcl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string cmp = freshLocal("mkcc");
      out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << len << "\n";
      branch(cmp, bodyBB, endBB);
    }
    beginBlock(bodyBB);
    {
      std::string i = freshLocal("mkbl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string kp = freshLocal("mkkp");
      int32_t kw = fieldByteWidth(keyT);
      out_ << "  " << kp << " = call i8* @ox_map_key_ptr(i8* " << h << ", i64 " << i
           << ")\n";
      if (!sx.empty()) {

        std::string tp = freshLocal("mkt");
        if (keyT == BType::str || keyT.tag == BType::Tag::ptr) {
          std::string kv = freshLocal("mkkv");
          out_ << "  " << kv << " = load " << vecSlotType(sx) << ", i8* " << kp << "\n";
          out_ << "  call void @ox_vec_push_" << sx << "(i8* " << out << ", "
               << vecSlotType(sx) << " " << kv << ")\n";
        } else {
          (void)tp; (void)kw;
          out_ << "  " << tp << " = bitcast i8* " << kp << " to " << typeStr(keyT) << "*\n";
          std::string kv = freshLocal("mkkv");
          out_ << "  " << kv << " = load " << typeStr(keyT) << ", " << typeStr(keyT)
               << "* " << tp << "\n";
          std::string cv = genCoerce(kv, keyT, vecSlotBType(sx));
          out_ << "  call void @ox_vec_push_" << sx << "(i8* " << out << ", "
               << vecSlotType(sx) << " " << cv << ")\n";
        }
      } else {

        int32_t esz = fieldByteWidth(keyT);
        std::string tmp = freshLocal("mkel");
        out_ << "  " << tmp << " = alloca " << typeStr(keyT) << "\n";
        out_ << "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* " << tmp << ", i8* " << kp
             << ", i64 " << (esz > 0 ? esz : 8) << ", i1 0)\n";
        out_ << "  call void @ox_vec_blob_push(i8* " << out << ", i64 "
             << (esz > 0 ? esz : 8) << ", i8* " << tmp << ")\n";
      }
    }
    jump(stepBB); beginBlock(stepBB);
    {
      std::string i = freshLocal("mksl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ni = freshLocal("mkn");
      out_ << "  " << ni << " = add i64 " << i << ", 1\n";
      out_ << "  store i64 " << ni << ", i64* " << iSlot << "\n";
    }
    jump(condBB); beginBlock(endBB);
    return {out, vecT};
  }

  if (n == "set_len") {
    auto [h, ht] = emitArgVal(0);
    usedSet_ = true;
    std::string r = freshLocal("setlen");
    out_ << "  " << r << " = call i64 @ox_set_len(i8* " << h << ")\n";
    return {r, BType::i64};
  }
  if (n == "set_contains") {
    auto [h, ht] = emitArgVal(0); auto [k, kt] = emitArgVal(1);
    usedSet_ = true; collectStruct(setElemType(ht));
    BType et = setElemType(ht);
    std::string kp = spillScratch(genCoerce(k, kt, et), et);
    std::string raw = freshLocal("setcont");
    out_ << "  " << raw << " = call i64 @ox_set_contains(i8* " << h << ", i8* " << kp << ")\n";
    std::string r = freshLocal("setcontb");
    out_ << "  " << r << " = icmp ne i64 " << raw << ", 0\n";
    return {r, BType::bool_};
  }
  if (n == "set_insert" || n == "set_remove") {
    auto [h, ht] = emitArgVal(0); auto [k, kt] = emitArgVal(1);
    usedSet_ = true; collectStruct(setElemType(ht));
    BType et = setElemType(ht);
    std::string kp = spillScratch(genCoerce(k, kt, et), et);
    out_ << "  call void @ox_" << (n == "set_insert" ? "set_insert" : "set_remove")
         << "(i8* " << h << ", i8* " << kp << ")\n";
    return {"", BType::void_};
  }
  if (n == "set_to_vec") {


    auto [h, ht] = emitArgVal(0);
    usedSet_ = true; collectStruct(setElemType(ht));
    BType et = setElemType(ht);
    std::string sx = elemSuffix(et);
    BType vecT = makeDynArray(et);
    std::string out;
    if (!sx.empty()) {
      usedVec_.insert(sx);
      out = freshLocal("setvec");
      out_ << "  " << out << " = call i8* @ox_vec_new_" << sx << "()\n";
    } else {
      usedVec_blob_ = true;
      out = freshLocal("setvec");
      int32_t esz = fieldByteWidth(et);
      out_ << "  " << out << " = call i8* @ox_vec_blob_new(i64 "
           << (esz > 0 ? esz : 8) << ")\n";
    }
    std::string len = freshLocal("setlen");
    out_ << "  " << len << " = call i64 @ox_set_len(i8* " << h << ")\n";
    std::string iSlot = freshLocal("seti");
    out_ << "  " << iSlot << " = alloca i64\n";
    out_ << "  store i64 0, i64* " << iSlot << "\n";
    std::string condBB = freshLabel("setc"), bodyBB = freshLabel("setb"),
                stepBB = freshLabel("sets"), endBB = freshLabel("sete");
    jump(condBB); beginBlock(condBB);
    {
      std::string i = freshLocal("setcl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string cmp = freshLocal("setcc");
      out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << len << "\n";
      branch(cmp, bodyBB, endBB);
    }
    beginBlock(bodyBB);
    {
      std::string i = freshLocal("setbl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ep = freshLocal("setkp");
      out_ << "  " << ep << " = call i8* @ox_set_ptr(i8* " << h << ", i64 " << i << ")\n";
      if (!sx.empty()) {
        if (et == BType::str || et.tag == BType::Tag::ptr) {
          std::string ev = freshLocal("setv");
          out_ << "  " << ev << " = load " << vecSlotType(sx) << ", i8* " << ep << "\n";
          out_ << "  call void @ox_vec_push_" << sx << "(i8* " << out << ", "
               << vecSlotType(sx) << " " << ev << ")\n";
        } else {
          std::string tp = freshLocal("sett");
          out_ << "  " << tp << " = bitcast i8* " << ep << " to " << typeStr(et) << "*\n";
          std::string ev = freshLocal("setv");
          out_ << "  " << ev << " = load " << typeStr(et) << ", " << typeStr(et)
               << "* " << tp << "\n";
          std::string cv = genCoerce(ev, et, vecSlotBType(sx));
          out_ << "  call void @ox_vec_push_" << sx << "(i8* " << out << ", "
               << vecSlotType(sx) << " " << cv << ")\n";
        }
      } else {
        int32_t esz = fieldByteWidth(et);
        std::string tmp = freshLocal("setel");
        out_ << "  " << tmp << " = alloca " << typeStr(et) << "\n";
        out_ << "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* " << tmp << ", i8* " << ep
             << ", i64 " << (esz > 0 ? esz : 8) << ", i1 0)\n";
        out_ << "  call void @ox_vec_blob_push(i8* " << out << ", i64 "
             << (esz > 0 ? esz : 8) << ", i8* " << tmp << ")\n";
      }
    }
    jump(stepBB); beginBlock(stepBB);
    {
      std::string i = freshLocal("setsl");
      out_ << "  " << i << " = load i64, i64* " << iSlot << "\n";
      std::string ni = freshLocal("setn");
      out_ << "  " << ni << " = add i64 " << i << ", 1\n";
      out_ << "  store i64 " << ni << ", i64* " << iSlot << "\n";
    }
    jump(condBB); beginBlock(endBB);
    return {out, vecT};
  }
  if (n == "len") {
    auto [v, vt] = emitArgVal(0);
    if (vt.tag == BType::Tag::dynarray) {
      std::string r = freshLocal("vlen");
      out_ << "  " << r << " = call i64 @ox_vec_len(i8* " << v << ")\n";
      return {r, BType::i64};
    }
    if (vt.tag == BType::Tag::array) {
      return {std::to_string(vt.count), BType::i64};
    }
    if (vt.tag == BType::Tag::map_) {
      usedMap_ = true;
      std::string r = freshLocal("mlen");
      out_ << "  " << r << " = call i64 @ox_map_len(i8* " << v << ")\n";
      return {r, BType::i64};
    }
    if (vt.tag == BType::Tag::set_) {
      usedSet_ = true;
      std::string r = freshLocal("setlen");
      out_ << "  " << r << " = call i64 @ox_set_len(i8* " << v << ")\n";
      return {r, BType::i64};
    }
    if (vt == BType::str) {
      std::string r = freshLocal("slen");
      out_ << "  " << r << " = call i64 @ox_strlen(i8* " << v << ")\n";
      return {r, BType::i64};
    }
    return {"0", BType::i64};
  }
  if (n == "push") {
    auto [hv, hvt] = emitArgVal(0);
    auto [xv, xvt] = emitArgVal(1);
    if (hvt.tag == BType::Tag::dynarray) {
      BType et = dynArrayElem(hvt);
      std::string sx = elemSuffix(et);
      if (!sx.empty()) {
        usedVec_.insert(sx);
        std::string cv = genCoerce(xv, xvt, vecSlotBType(sx));
        out_ << "  call void @ox_vec_push_" << sx << "(i8* " << hv << ", "
             << vecSlotType(sx) << " " << cv << ")\n";
      } else {

        std::string cv = genCoerce(xv, xvt, et);
        std::string tmp = freshLocal("vbp");
        out_ << "  " << tmp << " = alloca " << typeStr(et) << "\n";
        out_ << "  store " << typeStr(et) << " " << cv << ", " << typeStr(et) << "* " << tmp << "\n";
        int32_t esz = fieldByteWidth(et);
        usedVec_blob_ = true;
        out_ << "  call void @ox_vec_blob_push(i8* " << hv << ", i64 "
             << (esz > 0 ? esz : 8) << ", i8* " << tmp << ")\n";
      }
    }
    return {"", BType::void_};
  }
  if (n == "sort") {
    auto [hv, hvt] = emitArgVal(0);
    if (hvt.tag == BType::Tag::dynarray) {
      BType et = dynArrayElem(hvt);
      std::string sx = elemSuffix(et);
      if (!sx.empty()) {
        usedSort_.insert(sx);
        out_ << "  call void @ox_sort_" << sx << "(i8* " << hv << ")\n";
      } else {


        int32_t esz = fieldByteWidth(et);
        long long kind = 0;
        if (isFloat(et)) kind = 2;
        else if (!isSignedInt(et) && (et.tag == BType::Tag::u8 || et.tag == BType::Tag::u16 ||
                 et.tag == BType::Tag::u32 || et.tag == BType::Tag::u64 ||
                 et.tag == BType::Tag::usize)) kind = 1;
        else if (et.tag == BType::Tag::ptr) kind = 3;
        usedSort_blob_ = true;
        out_ << "  call void @ox_sort_blob(i8* " << hv << ", i64 "
             << (esz > 0 ? esz : 8) << ", i64 " << kind << ")\n";
      }
    }
    return {"", BType::void_};
  }
  return {"|||no|||", BType::void_};
}

void IRGen::beginBlock(const std::string& name) {
  out_ << name << ":\n";
  curBlock_ = name;
  terminated_ = false;
}

void IRGen::branch(const std::string& cond, const std::string& t, const std::string& f) {
  if (terminated_) return;
  out_ << "  br i1 " << cond << ", label %" << t << ", label %" << f << "\n";
  terminated_ = true;
}

void IRGen::jump(const std::string& t) {
  if (terminated_) return;
  out_ << "  br label %" << t << "\n";
  terminated_ = true;
}

void IRGen::ensureTerminated() {
  if (!terminated_) { out_ << "  unreachable\n"; terminated_ = true; }
}

void IRGen::genBlock(const std::vector<StmtPtr>& stmts) {
  for (auto& s : stmts) {
    if (terminated_) return;
    genStmt(s.get());
  }
}

void IRGen::genStmt(Stmt* s) {
  if (auto es = dynamic_cast<ExprStmt*>(s)) {
    genExpr(es->expr.get());
    return;
  }
  if (auto ls = dynamic_cast<LetStmt*>(s)) {
    std::string a = "%var_" + ls->name + "_" + std::to_string(labelSeq_++);
    out_ << "  " << a << " = alloca " << typeStr(ls->type) << "\n";
    if (ls->init) {
      auto [v, vt] = genExpr(ls->init.get());
      std::string cv = genCoerce(v, vt, ls->type);
      out_ << "  store " << typeStr(ls->type) << " " << cv << ", " << typeStr(ls->type) << "* " << a << "\n";
    }
    declareVar(ls->name, a, ls->type);
    return;
  }
  if (auto rs = dynamic_cast<ReturnStmt*>(s)) {
    if (rs->value) {
      auto [v, vt] = genExpr(rs->value.get());
      std::string cv = genCoerce(v, vt, curFnRet_);
      out_ << "  ret " << typeStr(curFnRet_) << " " << cv << "\n";
    } else {
      out_ << "  ret void\n";
    }
    terminated_ = true;
    return;
  }
  if (auto is = dynamic_cast<IfStmt*>(s)) {
    auto [c, ct] = genExpr(is->cond.get());

    (void)ct;
    std::string thenBB = freshLabel("then");
    std::string elseBB = freshLabel("else");
    std::string mergeBB = freshLabel("merge");

    branch(c, thenBB, elseBB);
    beginBlock(thenBB);
    pushScope(); genBlock(is->then); popScope();
    jump(mergeBB);
    beginBlock(elseBB);
    if (!is->else_.empty()) {
      pushScope(); genBlock(is->else_); popScope();
      jump(mergeBB);
    } else {
      jump(mergeBB);
    }
    beginBlock(mergeBB);
    return;
  }
  if (auto ws = dynamic_cast<WhileStmt*>(s)) {
    std::string condBB = freshLabel("while_cond");
    std::string bodyBB = freshLabel("while_body");
    std::string endBB = freshLabel("while_end");
    jump(condBB);
    beginBlock(condBB);
    auto [c, ct] = genExpr(ws->cond.get());
    branch(c, bodyBB, endBB);
    beginBlock(bodyBB);
    loops_.push_back({condBB, endBB});
    pushScope(); genBlock(ws->body); popScope();
    loops_.pop_back();
    jump(condBB);
    beginBlock(endBB);
    return;
  }
  if (auto fs = dynamic_cast<ForStmt*>(s)) {
    if (fs->isForeach) {


      BType et = fs->elemType;


      auto [iterV, iterT] = genExpr(fs->iter.get());

      pushScope();

      std::string idxSlot = "%fi_" + fs->varName + "_" + std::to_string(labelSeq_++);
      out_ << "  " << idxSlot << " = alloca i64\n";
      out_ << "  store i64 0, i64* " << idxSlot << "\n";

      std::string eltSlot = "%var_" + fs->varName + "_" + std::to_string(labelSeq_++);
      out_ << "  " << eltSlot << " = alloca " << typeStr(et) << "\n";
      declareVar(fs->varName, eltSlot, et);


      std::string basePtr;
      std::string vecHandle = iterV;
      bool isVec = (iterT.tag == BType::Tag::dynarray);
      bool isStr = (iterT == BType::str);
      int32_t arrLen = 0;
      if (isStr) {
        basePtr = iterV;
      } else if (!isVec) {

        arrLen = iterT.count;
        std::string spill = "%fa_" + fs->varName + "_" + std::to_string(labelSeq_++);
        out_ << "  " << spill << " = alloca " << typeStr(iterT) << "\n";
        out_ << "  store " << typeStr(iterT) << " " << iterV << ", " << typeStr(iterT) << "* " << spill << "\n";
        basePtr = spill;
      } else {
        BType ve = dynArrayElem(iterT);
        std::string sx = elemSuffix(ve);
        if (!sx.empty()) usedVec_.insert(sx); else usedVec_blob_ = true;
      }

      std::string condBB = freshLabel("fe_cond");
      std::string bodyBB = freshLabel("fe_body");
      std::string stepBB = freshLabel("fe_step");
      std::string endBB = freshLabel("fe_end");
      jump(condBB);
      beginBlock(condBB);
      {
        std::string i = freshLocal("fi");
        out_ << "  " << i << " = load i64, i64* " << idxSlot << "\n";
        std::string lenV;
        if (isVec) {
          lenV = freshLocal("fvl");
          out_ << "  " << lenV << " = call i64 @ox_vec_len(i8* " << vecHandle << ")\n";
        } else if (isStr) {
          lenV = freshLocal("fsl");
          out_ << "  " << lenV << " = call i64 @ox_strlen(i8* " << basePtr << ")\n";
        } else {
          lenV = std::to_string(arrLen);
        }
        std::string cmp = freshLocal("fc");
        out_ << "  " << cmp << " = icmp slt i64 " << i << ", " << lenV << "\n";
        branch(cmp, bodyBB, endBB);
      }
      beginBlock(bodyBB);
      loops_.push_back({stepBB, endBB});

      {
        std::string i = freshLocal("fi2");
        out_ << "  " << i << " = load i64, i64* " << idxSlot << "\n";
        std::string ev;
        if (isVec) {
          BType ve = dynArrayElem(iterT);
          std::string sx = elemSuffix(ve);
          if (!sx.empty()) {
            std::string slot = vecSlotType(sx);
            std::string raw = freshLocal("fg");
            out_ << "  " << raw << " = call " << slot << " @ox_vec_get_" << sx
                 << "(i8* " << vecHandle << ", i64 " << i << ")\n";
            ev = genCoerce(raw, vecSlotBType(sx), et);
          } else {

            int32_t esz = fieldByteWidth(et);
            std::string ep = freshLocal("fpb");
            out_ << "  " << ep << " = call i8* @ox_vec_blob_ptr(i8* " << vecHandle
                 << ", i64 " << i << ", i64 " << (esz > 0 ? esz : 8) << ")\n";
            if (et.tag == BType::Tag::array || et.tag == BType::Tag::struct_) {

              out_ << "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* " << eltSlot
                   << ", i8* " << ep << ", i64 " << (esz > 0 ? esz : 8)
                   << ", i1 false)\n";
              ev = "";
            } else {
              std::string ev2 = freshLocal("fbl");
              if (et.tag == BType::Tag::dynarray || et.tag == BType::Tag::ptr)
                out_ << "  " << ev2 << " = load " << typeStr(et) << ", i8* " << ep << "\n";
              else {
                std::string tp = freshLocal("fbtc");
                out_ << "  " << tp << " = bitcast i8* " << ep << " to " << typeStr(et) << "*\n";
                out_ << "  " << ev2 << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << tp << "\n";
              }
              ev = ev2;
            }
          }
        } else if (isStr) {

          std::string ep = freshLocal("fep");
          out_ << "  " << ep << " = getelementptr inbounds i8, i8* " << basePtr << ", i64 " << i << "\n";
          ev = freshLocal("fl");
          out_ << "  " << ev << " = load i8, i8* " << ep << "\n";
        } else {
          std::string ep = freshLocal("fep");
          out_ << "  " << ep << " = getelementptr inbounds " << typeStr(iterT) << ", "
               << typeStr(iterT) << "* " << basePtr << ", i64 0, i64 " << i << "\n";
          ev = freshLocal("fl");
          out_ << "  " << ev << " = load " << typeStr(et) << ", " << typeStr(et) << "* " << ep << "\n";
        }
        if (!ev.empty())
          out_ << "  store " << typeStr(et) << " " << ev << ", " << typeStr(et) << "* " << eltSlot << "\n";
      }
      pushScope(); genBlock(fs->body); popScope();
      loops_.pop_back();
      jump(stepBB);
      beginBlock(stepBB);
      {
        std::string i = freshLocal("fi3");
        out_ << "  " << i << " = load i64, i64* " << idxSlot << "\n";
        std::string ni = freshLocal("fin");
        out_ << "  " << ni << " = add i64 " << i << ", 1\n";
        out_ << "  store i64 " << ni << ", i64* " << idxSlot << "\n";
      }
      jump(condBB);
      beginBlock(endBB);
      popScope();
      return;
    }

    std::string condBB = freshLabel("for_cond");
    std::string bodyBB = freshLabel("for_body");
    std::string stepBB = freshLabel("for_step");
    std::string endBB = freshLabel("for_end");


    pushScope();
    std::string loopVar = "%var_" + fs->varName + "_" + std::to_string(labelSeq_++);
    out_ << "  " << loopVar << " = alloca i64\n";
    auto [startV, startT] = genExpr(fs->start.get());
    out_ << "  store i64 " << startV << ", i64* " << loopVar << "\n";
    declareVar(fs->varName, loopVar, BType::i64);

    jump(condBB);
    beginBlock(condBB);

    if (fs->end) {
      auto [condV, condT] = genExpr(fs->end.get());
      branch(condV, bodyBB, endBB);
    } else {
      branch("true", bodyBB, endBB);
    }
    beginBlock(bodyBB);
    loops_.push_back({stepBB, endBB});
    pushScope(); genBlock(fs->body); popScope();
    loops_.pop_back();
    jump(stepBB);
    beginBlock(stepBB);

    if (fs->step) {
      genExpr(fs->step.get());
    }
    jump(condBB);
    beginBlock(endBB);
    popScope();
    return;
  }
  if (dynamic_cast<BreakStmt*>(s)) {
    if (!loops_.empty()) jump(loops_.back().brk);
    return;
  }
  if (dynamic_cast<ContinueStmt*>(s)) {
    if (!loops_.empty()) jump(loops_.back().cont);
    return;
  }
  if (auto b = dynamic_cast<Block*>(s)) {
    pushScope(); genBlock(b->stmts); popScope();
    return;
  }
}

void IRGen::genFunc(FuncDecl& fn) {
  labelSeq_ = 0;
  terminated_ = false;
  curFnRet_ = fn.retType;
  std::ostringstream sig;
  sig << "define " << typeStr(fn.retType) << " @" << fn.name << "(";
  for (size_t i = 0; i < fn.params.size(); i++) {
    if (i) sig << ", ";
    sig << typeStr(fn.params[i].type) << " %arg_" << fn.params[i].name;
  }
  sig << ") {\n";
  out_ << sig.str();

  scopes_.clear();
  pushScope();

  std::string entry = freshLabel("entry");
  out_ << entry << ":\n";
  curBlock_ = entry;


  for (auto& kv : sema_.globals)
    declareVar(kv.first, "@" + kv.first, kv.second.type);

  for (auto& p : fn.params) {
    std::string a = "%var_" + p.name + "_" + std::to_string(labelSeq_++);
    out_ << "  " << a << " = alloca " << typeStr(p.type) << "\n";
    out_ << "  store " << typeStr(p.type) << " %arg_" << p.name << ", " << typeStr(p.type) << "* " << a << "\n";
    declareVar(p.name, a, p.type);
  }

  genBlock(fn.body);
  popScope();

  if (!terminated_) {
    if (fn.retType == BType::void_) out_ << "  ret void\n";
    else if (fn.retType == BType::f64) out_ << "  ret double 0.0\n";
    else if (fn.retType == BType::bool_) out_ << "  ret i1 0\n";
    else if (fn.retType == BType::str) out_ << "  ret i8* null\n";
    else if (fn.retType.tag == BType::Tag::array ||
             fn.retType.tag == BType::Tag::struct_)
      out_ << "  ret " << typeStr(fn.retType) << " zeroinitializer\n";
    else out_ << "  ret i64 0\n";
    terminated_ = true;
  }
  out_ << "}\n\n";
}


void IRGen::genLambda(const LambdaLit* lam) {
  if (emittedLambdas_.count(lam->loweredName)) return;
  emittedLambdas_.insert(lam->loweredName);


  std::ostringstream swapped;
  out_.swap(swapped);
  labelSeq_ = 0;
  terminated_ = false;
  curFnRet_ = lam->retType;
  out_ << "define " << typeStr(lam->retType) << " @" << lam->loweredName << "(";
  for (size_t i = 0; i < lam->params.size(); i++) {
    if (i) out_ << ", ";
    out_ << typeStr(lam->params[i].type) << " %arg_" << lam->params[i].name;
  }
  out_ << ") {\n";
  scopes_.clear();
  pushScope();
  std::string entry = freshLabel("entry");
  out_ << entry << ":\n";
  curBlock_ = entry;
  for (auto& kv : sema_.globals)
    declareVar(kv.first, "@" + kv.first, kv.second.type);
  for (auto& p : lam->params) {
    std::string a = "%var_" + p.name + "_" + std::to_string(labelSeq_++);
    out_ << "  " << a << " = alloca " << typeStr(p.type) << "\n";
    out_ << "  store " << typeStr(p.type) << " %arg_" << p.name << ", " << typeStr(p.type) << "* " << a << "\n";
    declareVar(p.name, a, p.type);
  }
  genBlock(lam->body);
  popScope();
  if (!terminated_) {
    if (lam->retType == BType::void_) out_ << "  ret void\n";
    else if (lam->retType == BType::f64) out_ << "  ret double 0.0\n";
    else if (lam->retType == BType::bool_) out_ << "  ret i1 0\n";
    else if (lam->retType == BType::str) out_ << "  ret i8* null\n";
    else if (lam->retType.tag == BType::Tag::array ||
             lam->retType.tag == BType::Tag::struct_)
      out_ << "  ret " << typeStr(lam->retType) << " zeroinitializer\n";
    else out_ << "  ret i64 0\n";
    terminated_ = true;
  }
  out_ << "}\n\n";
  lambdas_ << out_.str();
  out_.swap(swapped);
}

void IRGen::genMethod(const std::string& structName, FuncDecl& fn) {
  labelSeq_ = 0;
  terminated_ = false;
  curFnRet_ = fn.retType;
  std::string mangled = mangleMethod(structName, fn.name);
  BType st; st.tag = BType::Tag::struct_; st.structName = structName;
  collectStruct(st);

  std::ostringstream sig;
  sig << "define " << typeStr(fn.retType) << " @" << mangled << "(";

  bool first = true;
  if (fn.hasSelf) {
    sig << typeStr(fn.selfByRef ? makePtr(st) : st) << " %arg_self";
    first = false;
  }
  for (auto& p : fn.params) {
    if (!first) sig << ", ";
    first = false;
    sig << typeStr(p.type) << " %arg_" << p.name;
  }
  sig << ") {\n";
  out_ << sig.str();

  scopes_.clear();
  pushScope();
  std::string entry = freshLabel("entry");
  out_ << entry << ":\n";
  curBlock_ = entry;

  for (auto& kv : sema_.globals)
    declareVar(kv.first, "@" + kv.first, kv.second.type);


  if (fn.hasSelf) {
    if (fn.selfByRef) {
      declareVar("self", "%arg_self", st);
    } else {
      std::string a = "%var_self_" + std::to_string(labelSeq_++);
      out_ << "  " << a << " = alloca " << typeStr(st) << "\n";
      out_ << "  store " << typeStr(st) << " %arg_self, " << typeStr(st) << "* " << a << "\n";
      declareVar("self", a, st);
    }
  }

  for (auto& p : fn.params) {
    std::string a = "%var_" + p.name + "_" + std::to_string(labelSeq_++);
    out_ << "  " << a << " = alloca " << typeStr(p.type) << "\n";
    out_ << "  store " << typeStr(p.type) << " %arg_" << p.name << ", " << typeStr(p.type) << "* " << a << "\n";
    declareVar(p.name, a, p.type);
  }

  genBlock(fn.body);
  popScope();

  if (!terminated_) {
    if (fn.retType == BType::void_) out_ << "  ret void\n";
    else if (fn.retType == BType::f64) out_ << "  ret double 0.0\n";
    else if (fn.retType == BType::bool_) out_ << "  ret i1 0\n";
    else if (fn.retType == BType::str) out_ << "  ret i8* null\n";
    else if (fn.retType.tag == BType::Tag::array ||
             fn.retType.tag == BType::Tag::struct_)
      out_ << "  ret " << typeStr(fn.retType) << " zeroinitializer\n";
    else out_ << "  ret i64 0\n";
    terminated_ = true;
  }
  out_ << "}\n\n";
}

void IRGen::generate(Program& prog) {
  g_tmp = 0;


  for (auto& fn : prog.funcs)
    if (!fn->isExtern) userDefinedFns_.insert(fn->name);
  for (auto& mf : sema_.monomorphFns) userDefinedFns_.insert(mf->name);


  for (auto& sd : prog.structs) {
    StructDef* d = findStruct(sd->name);
    if (d) structDefs_[sd->name] = d;
  }

  for (auto& fn : prog.funcs) {
    collectStruct(fn->retType);
    for (auto& p : fn->params) collectStruct(p.type);
  }

  for (auto& im : prog.impls) {
    BType st; st.tag = BType::Tag::struct_; st.structName = im->structName;
    collectStruct(st);
    for (auto& m : im->methods) {
      collectStruct(m->retType);
      for (auto& p : m->params) collectStruct(p.type);
    }
  }


  for (StructDef* d : allStructDefs()) {
    if (d->name.rfind("__oxg_", 0) == 0) {
      BType t; t.tag = BType::Tag::struct_; t.structName = d->name;
      collectStruct(t);
    }
  }


  std::ostringstream bodies;
  bodies.swap(out_);
  for (auto& fn : prog.funcs) {
    if (fn->isExtern) continue;
    if (fn->isGeneric) continue;
    genFunc(*fn);
  }


  for (auto& mf : sema_.monomorphFns) {
    collectStruct(mf->retType);
    for (auto& p : mf->params) collectStruct(p.type);
    genFunc(*mf);
  }


  for (auto& im : prog.impls) {
    if (!findStruct(im->structName)) continue;
    for (auto& m : im->methods) genMethod(im->structName, *m);
  }
  bodies.swap(out_);


  out_.str("");
  out_.clear();
  out_ << "; oxide generated ir\n";


  if (!targetTriple_.empty())
    out_ << "target triple = \"" << targetTriple_ << "\"\n";
  emitHeaderAndRuntime();
  emitGlobalsAndExterns();
  out_ << globals_.str();
  out_ << "\n";

  out_ << lambdas_.str();
  out_ << bodies.str();
}
