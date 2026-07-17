#include "Sema.h"
#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <vector>
#include <algorithm>


ExprPtr cloneExpr(const Expr* e);
StmtPtr cloneStmt(const Stmt* s);

VarInfo Sema::lookup(const std::string& name) {
  for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
    auto f = it->find(name);
    if (f != it->end()) return {f->second.type, f->second.isMut, true};
  }
  return {BType::void_, false, false};
}

void Sema::pushScope() { scopes_.emplace_back(); }
void Sema::popScope() { scopes_.pop_back(); }
void Sema::declare(const std::string& name, BType t, bool isMut, bool isGlobal) {
  scopes_.back()[name] = {t, isMut, isGlobal};
}

static void errAt(std::vector<SemanticError>& errs, int line, int col, const std::string& m) {
  errs.push_back({m, line, col, ""});
}
static void errAt(std::vector<SemanticError>& errs, int line, int col, const std::string& m, const std::string& hint) {
  errs.push_back({m, line, col, hint});
}

static void errAt(std::vector<SemanticError>& errs, int line, const std::string& m) {
  errs.push_back({m, line, 0, ""});
}


std::string mangleMethod(const std::string& structName, const std::string& methodName) {
  return "__oxm_" + structName + "__" + methodName;
}


static BType fixType(BType t);


static BType substitute(BType t, const std::map<std::string, BType>& env) {
  if (t.tag == BType::Tag::struct_) {
    auto it = env.find(t.structName);
    if (it != env.end()) return it->second;
    return t;
  }
  if (t.tag == BType::Tag::array)
    return makeArrayType(substitute(arrayElem(t), env), t.count);
  if (t.tag == BType::Tag::dynarray)
    return makeDynArray(substitute(dynArrayElem(t), env));
  if (t.tag == BType::Tag::ptr)
    return makePtr(substitute(pointee(t), env));
  if (t.tag == BType::Tag::map_ || t.tag == BType::Tag::hmap_)
    return (t.tag == BType::Tag::map_) ? makeMapType(substitute(mapKeyType(t), env), substitute(mapValType(t), env))
                                  : makeHMapType(substitute(mapKeyType(t), env), substitute(mapValType(t), env));
  if (t.tag == BType::Tag::set_ || t.tag == BType::Tag::hset_)
    return (t.tag == BType::Tag::set_) ? makeSetType(substitute(setElemType(t), env))
                                  : makeHSetType(substitute(setElemType(t), env));
  if (t.tag == BType::Tag::fn_) {
    const auto& ps = fnParams(t);
    std::vector<BType> np; np.reserve(ps.size());
    for (const auto& p : ps) np.push_back(substitute(p, env));
    return makeFnType(np, substitute(fnRet(t), env));
  }
  if (t.tag == BType::Tag::generic_) {


    const auto& args = genericInstArgs(t);
    std::vector<BType> na; na.reserve(args.size());
    for (const auto& a : args) na.push_back(substitute(a, env));
    return makeGenericInst(genericInstBase(t), na, genericInstIsFn(t));
  }
  return t;
}


static std::string mangleInst(const std::string& base, const std::vector<BType>& args) {
  std::string s = "__oxg_" + base;
  for (const auto& a : args) {
    s += "__";
    std::string sp = typeSpelling(a);
    for (char c : sp) {
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) s += c;
      else if (c == ' ') ;
      else { s += '_'; }
    }
  }
  return s;
}


BType instantiateGenericStruct(const std::string& base, const std::vector<BType>& args) {
  if (isAliasName(base)) {


  }
  const StructDecl* tmpl = findGenericStruct(base);
  if (!tmpl) {


    BType t; t.tag = BType::Tag::struct_; t.structName = base; return t;
  }
  if (args.size() != tmpl->typeParams.size()) {
    BType t; t.tag = BType::Tag::struct_; t.structName = base; return t;
  }
  std::string mangled = mangleInst(base, args);

  if (findStruct(mangled)) {
    BType t; t.tag = BType::Tag::struct_; t.structName = mangled; return t;
  }

  std::map<std::string, BType> env;
  for (size_t i = 0; i < tmpl->typeParams.size() && i < args.size(); i++)
    env[tmpl->typeParams[i]] = args[i];

  StructDef* d = registerStruct(mangled);
  d->name = mangled;
  d->fields.clear();
  for (auto& f : tmpl->fields) {
    const std::string& fname = std::get<0>(f);
    bool priv = std::get<2>(f);
    BType ft = fixType(substitute(std::get<1>(f), env));
    d->fields.push_back({fname, ft, 0, priv});
  }

  d->size = 0; d->align = 1;
  int32_t off = 0;
  for (auto& f : d->fields) {
    int32_t w = fieldByteWidth(f.type);
    int32_t a = fieldAlign(f.type);
    if (a > d->align) d->align = a;
    off = (off + a - 1) & ~(a - 1);
    f.offset = off;
    off += w;
  }
  off = (off + d->align - 1) & ~(d->align - 1);
  d->size = off;
  BType t; t.tag = BType::Tag::struct_; t.structName = mangled; return t;
}

static BType fixType(BType t) {
  if (t.tag == BType::Tag::struct_) {
    if (findEnum(t.structName)) return makeEnumType(t.structName);
    if (isAliasName(t.structName))
      return fixType(resolveAlias(t.structName).second);
    return t;
  }
  if (t.tag == BType::Tag::generic_) {

    if (genericInstIsFn(t)) {


      std::vector<BType> args; for (const auto& a : genericInstArgs(t)) args.push_back(fixType(a));
      BType ft; ft.tag = BType::Tag::struct_; ft.structName = mangleInst(genericInstBase(t), args);
      return ft;
    }
    std::vector<BType> args; for (const auto& a : genericInstArgs(t)) args.push_back(fixType(a));
    return instantiateGenericStruct(genericInstBase(t), args);
  }
  if (t.tag == BType::Tag::fn_) {
    const auto& ps = fnParams(t);
    std::vector<BType> np; for (const auto& p : ps) np.push_back(fixType(p));
    return makeFnType(np, fixType(fnRet(t)));
  }
  if (t.tag == BType::Tag::array)   return makeArrayType(fixType(arrayElem(t)), t.count);
  if (t.tag == BType::Tag::dynarray) return makeDynArray(fixType(dynArrayElem(t)));
  if (t.tag == BType::Tag::ptr)    return makePtr(fixType(pointee(t)));
  if (t.tag == BType::Tag::map_ || t.tag == BType::Tag::hmap_)
    return (t.tag == BType::Tag::map_) ? makeMapType(fixType(mapKeyType(t)), fixType(mapValType(t)))
                                  : makeHMapType(fixType(mapKeyType(t)), fixType(mapValType(t)));
  if (t.tag == BType::Tag::set_ || t.tag == BType::Tag::hset_)
    return (t.tag == BType::Tag::set_) ? makeSetType(fixType(setElemType(t)))
                                  : makeHSetType(fixType(setElemType(t)));
  return t;
}


bool Sema::foldConstExpr(Expr* e, GlobalInfo& gi) {
  if (auto l = dynamic_cast<IntLit*>(e)) {
    gi.hasConstVal = true; gi.type = BType::i64; gi.iVal = (int64_t)l->v; return true;
  }
  if (auto l = dynamic_cast<FloatLit*>(e)) {
    gi.hasConstVal = true; gi.type = BType::f64; gi.fVal = l->v; return true;
  }
  if (auto l = dynamic_cast<BoolLit*>(e)) {
    gi.hasConstVal = true; gi.type = BType::bool_; gi.bVal = l->v; return true;
  }
  if (auto l = dynamic_cast<CharLit*>(e)) {
    gi.hasConstVal = true; gi.type = BType::char_; gi.cVal = l->v; return true;
  }
  if (auto l = dynamic_cast<StrLit*>(e)) {
    gi.hasConstVal = true; gi.type = BType::str; gi.sVal = l->v; return true;
  }
  if (auto s = dynamic_cast<SizeofExpr*>(e)) {


    BType tt = fixType(s->target);
    int32_t sz = fieldByteWidth(tt);
    if (sz <= 0) sz = 1;
    s->size = sz;
    gi.hasConstVal = true; gi.type = BType::i64; gi.iVal = (int64_t)sz; return true;
  }
  if (auto v = dynamic_cast<VarRef*>(e)) {

    auto it = globals.find(v->name);
    if (it != globals.end() && it->second.hasConstVal) {
      gi = it->second;
      gi.type = it->second.type;
      return true;
    }
    return false;
  }
  if (auto u = dynamic_cast<UnaryExpr*>(e)) {
    GlobalInfo tmp{};
    if (u->op == UnaryExpr::Op::neg) {
      if (foldConstExpr(u->base.get(), tmp)) {
        gi = tmp;
        if (gi.type == BType::f64) gi.fVal = -gi.fVal;
        else gi.iVal = -gi.iVal;
        return true;
      }
    }
    if (u->op == UnaryExpr::Op::bnot) {
      if (foldConstExpr(u->base.get(), tmp)) { gi = tmp; gi.iVal = ~gi.iVal; return true; }
    }
    if (u->op == UnaryExpr::Op::not_) {
      if (foldConstExpr(u->base.get(), tmp)) { gi = tmp; gi.bVal = !gi.bVal; return true; }
    }
    return false;
  }
  if (auto b = dynamic_cast<BinaryExpr*>(e)) {
    GlobalInfo l{}, r{};

    if (!foldConstExpr(b->lhs.get(), l) || !foldConstExpr(b->rhs.get(), r)) return false;
    if (isInt(l.type) && isInt(r.type)) {
      int64_t lv = l.iVal, rv = r.iVal;
      int64_t out = 0; bool isCmp = false; bool cv = false;
      switch (b->op) {
        case BinaryExpr::Op::add: out = lv + rv; break;
        case BinaryExpr::Op::sub: out = lv - rv; break;
        case BinaryExpr::Op::mul: out = lv * rv; break;
        case BinaryExpr::Op::div: out = rv ? lv / rv : 0; break;
        case BinaryExpr::Op::mod: out = rv ? lv % rv : 0; break;
        case BinaryExpr::Op::band: out = lv & rv; break;
        case BinaryExpr::Op::bor: out = lv | rv; break;
        case BinaryExpr::Op::bxor: out = lv ^ rv; break;
        case BinaryExpr::Op::shl: out = lv << rv; break;
        case BinaryExpr::Op::shr: out = lv >> rv; break;
        case BinaryExpr::Op::eq: cv = lv == rv; isCmp = true; break;
        case BinaryExpr::Op::ne: cv = lv != rv; isCmp = true; break;
        case BinaryExpr::Op::lt: cv = lv < rv; isCmp = true; break;
        case BinaryExpr::Op::gt: cv = lv > rv; isCmp = true; break;
        case BinaryExpr::Op::le: cv = lv <= rv; isCmp = true; break;
        case BinaryExpr::Op::ge: cv = lv >= rv; isCmp = true; break;
        default: return false;
      }
      gi = l;
      if (isCmp) { gi.hasConstVal = true; gi.bVal = cv; gi.type = BType::bool_; }
      else gi.iVal = out;
      return true;
    }
    if (l.type == BType::f64 && r.type == BType::f64) {
      double lv = l.fVal, rv = r.fVal, out = 0; bool isCmp = false; bool cv = false;
      switch (b->op) {
        case BinaryExpr::Op::add: out = lv + rv; break;
        case BinaryExpr::Op::sub: out = lv - rv; break;
        case BinaryExpr::Op::mul: out = lv * rv; break;
        case BinaryExpr::Op::div: out = rv ? lv / rv : 0; break;
        case BinaryExpr::Op::mod: out = std::fmod(lv, rv); break;
        case BinaryExpr::Op::eq: cv = lv == rv; isCmp = true; break;
        case BinaryExpr::Op::ne: cv = lv != rv; isCmp = true; break;
        case BinaryExpr::Op::lt: cv = lv < rv; isCmp = true; break;
        case BinaryExpr::Op::gt: cv = lv > rv; isCmp = true; break;
        case BinaryExpr::Op::le: cv = lv <= rv; isCmp = true; break;
        case BinaryExpr::Op::ge: cv = lv >= rv; isCmp = true; break;
        default: return false;
      }
      gi = l;
      if (isCmp) { gi.hasConstVal = true; gi.bVal = cv; gi.type = BType::bool_; }
      else gi.fVal = out;
      return true;
    }
    if ((l.type == BType::str && r.type == BType::str) ||
        (l.type == BType::str && r.type == BType::char_) ||
        (l.type == BType::char_ && r.type == BType::str)) {
      if (b->op != BinaryExpr::Op::add) return false;
      std::string ls = (l.type == BType::char_) ? std::string(1, (char)l.cVal) : l.sVal;
      std::string rs = (r.type == BType::char_) ? std::string(1, (char)r.cVal) : r.sVal;
      gi.hasConstVal = true; gi.type = BType::str; gi.sVal = ls + rs; return true;
    }


    if (l.type == BType::str && r.type == BType::str) {
      int c = l.sVal.compare(r.sVal);
      bool cv = false;
      switch (b->op) {
        case BinaryExpr::Op::eq: cv = (c == 0); break;
        case BinaryExpr::Op::ne: cv = (c != 0); break;
        case BinaryExpr::Op::lt: cv = (c < 0); break;
        case BinaryExpr::Op::gt: cv = (c > 0); break;
        case BinaryExpr::Op::le: cv = (c <= 0); break;
        case BinaryExpr::Op::ge: cv = (c >= 0); break;
        default: return false;
      }
      gi.hasConstVal = true; gi.type = BType::bool_; gi.bVal = cv; return true;
    }
    return false;
  }
  if (auto c = dynamic_cast<CastExpr*>(e)) {
    GlobalInfo tmp{};
    if (!foldConstExpr(c->e.get(), tmp)) return false;

    if (isInt(tmp.type) && isInt(c->target)) { gi = tmp; gi.type = c->target; gi.iVal = (int64_t)tmp.iVal; return true; }


    if (isFloat(tmp.type) && isFloat(c->target)) {
      gi = tmp; gi.type = c->target;
      if (c->target == BType::f32) gi.fVal = (double)(float)gi.fVal;
      return true;
    }

    if (isInt(tmp.type) && isFloat(c->target)) { gi = tmp; gi.type = c->target; gi.fVal = (double)tmp.iVal; if (c->target == BType::f32) gi.fVal = (double)(float)gi.fVal; return true; }

    if (isFloat(tmp.type) && isInt(c->target)) { gi = tmp; gi.type = c->target; gi.iVal = (int64_t)tmp.fVal; return true; }
    if (tmp.type == BType::char_ && isInt(c->target)) { gi = tmp; gi.type = c->target; gi.iVal = (int64_t)tmp.cVal; return true; }
    return false;
  }
  return false;
}
static int editDist(const std::string& a, const std::string& b) {
  size_t n = a.size(), m = b.size();
  std::vector<int> prev(m + 1), cur(m + 1);
  for (size_t j = 0; j <= m; j++) prev[j] = (int)j;
  for (size_t i = 1; i <= n; i++) {
    cur[0] = (int)i;
    for (size_t j = 1; j <= m; j++) {
      int cost = (a[i-1] == b[j-1]) ? 0 : 1;
      cur[j] = std::min(std::min(prev[j] + 1, cur[j-1] + 1), prev[j-1] + cost);
    }
    prev = cur;
  }
  return prev[m];
}
static std::string suggest(const std::string& name, const std::vector<std::string>& cands) {
  std::string best; int bestD = 1000;
  for (auto& c : cands) {
    if (c.empty()) continue;
    int d = editDist(name, c);
    if (d < bestD) { bestD = d; best = c; }
  }
  if (best.empty() || bestD > (int)name.size() / 2 + 2) return "";
  return best;
}


static bool isIntLitExpr(Expr* e) {
  if (dynamic_cast<IntLit*>(e)) return true;

  if (auto u = dynamic_cast<UnaryExpr*>(e))
    return u->op == UnaryExpr::Op::neg && isIntLitExpr(u->base.get());
  return false;
}


static BType coerceIntLit(Expr* e, BType litType, BType partnerType) {
  if (isIntLitExpr(e) && isInt(partnerType)) return partnerType;
  return litType;
}


static bool isFloatLitExpr(Expr* e) {
  if (dynamic_cast<FloatLit*>(e)) return true;
  if (auto u = dynamic_cast<UnaryExpr*>(e))
    return u->op == UnaryExpr::Op::neg && isFloatLitExpr(u->base.get());
  return false;
}
static BType coerceFloatLit(Expr* e, BType litType, BType partnerType) {
  if (isFloatLitExpr(e) && isFloat(partnerType)) return partnerType;
  return litType;
}


static bool litAssignable(Expr* e, BType fromType, BType toType) {
  if (fromType == toType) return true;
  if (isInt(toType) && isIntLitExpr(e)) return true;
  if (toType == BType::f64 && dynamic_cast<FloatLit*>(e)) return true;


  if (toType == BType::f32 && dynamic_cast<FloatLit*>(e)) return true;
  if (toType == BType::bool_ && dynamic_cast<BoolLit*>(e)) return true;
  if (toType == BType::char_ && dynamic_cast<CharLit*>(e)) return true;
  if (toType.tag == BType::Tag::ptr && dynamic_cast<NullLit*>(e)) return true;
  return false;
}


static BType pickCommonType(Expr* thenE, BType thenT, Expr* elseE, BType elseT,
                            std::vector<SemanticError>& errs, int line) {
  if (thenT == elseT) return thenT;
  if (litAssignable(thenE, thenT, elseT)) return elseT;
  if (litAssignable(elseE, elseT, thenT)) return thenT;
  errAt(errs, line, "ternary arms must have the same type (got " +
        typeSpelling(thenT) + " and " + typeSpelling(elseT) + ")");
  return thenT;
}


static bool isLvalueExpr(Expr* e) {
  if (dynamic_cast<VarRef*>(e)) return true;
  if (dynamic_cast<Index*>(e)) return true;
  if (dynamic_cast<Field*>(e)) return true;
  if (auto u = dynamic_cast<UnaryExpr*>(e)) return u->op == UnaryExpr::Op::deref;
  return false;
}


bool Sema::canTouchPrivate(const std::string& structName) {
  return !curImpl_.empty() && curImpl_ == structName;
}


static const char* opMethodName(BinaryExpr::Op op) {
  switch (op) {
    case BinaryExpr::Op::add: return "__add";
    case BinaryExpr::Op::sub: return "__sub";
    case BinaryExpr::Op::mul: return "__mul";
    case BinaryExpr::Op::div: return "__div";
    case BinaryExpr::Op::mod: return "__mod";
    case BinaryExpr::Op::eq:  return "__eq";
    case BinaryExpr::Op::ne:  return "__eq";
    case BinaryExpr::Op::lt:  return "__lt";
    case BinaryExpr::Op::le:  return "__le";
    case BinaryExpr::Op::gt:  return "__gt";
    case BinaryExpr::Op::ge:  return "__ge";
    case BinaryExpr::Op::band:return "__band";
    case BinaryExpr::Op::bor: return "__bor";
    case BinaryExpr::Op::bxor:return "__bxor";
    case BinaryExpr::Op::shl: return "__shl";
    case BinaryExpr::Op::shr: return "__shr";
    default: return "";
  }
}

static const char* opIMethodName(BinaryExpr::Op op) {
  switch (op) {
    case BinaryExpr::Op::add: return "__iadd";
    case BinaryExpr::Op::sub: return "__isub";
    case BinaryExpr::Op::mul: return "__imul";
    case BinaryExpr::Op::div: return "__idiv";
    case BinaryExpr::Op::mod: return "__imod";
    default: return "";
  }
}


const MethodInfo* Sema::resolveOverload(const std::string& sn,
                                        BinaryExpr::Op op, Expr* b) {
  (void)b;
  auto it = methods.find(sn);
  if (it == methods.end()) return nullptr;
  const char* nm = opMethodName(op);
  if (!nm || !nm[0]) return nullptr;
  auto mit = it->second.find(nm);
  if (mit == it->second.end()) return nullptr;
  return &mit->second;
}


void Sema::registerMethod(const std::string& structName, FuncDecl& fn) {
  BType st; st.tag = BType::Tag::struct_; st.structName = structName;
  std::string mangled = mangleMethod(structName, fn.name);
  MethodInfo mi;
  mi.retType = fn.retType;
  mi.hasSelf = fn.hasSelf;
  mi.selfByRef = fn.selfByRef;
  mi.implStruct = structName;
  mi.mangled = mangled;
  for (auto& p : fn.params) mi.paramTypes.push_back(p.type);
  if (methods.count(structName) && methods[structName].count(fn.name)) {
    errAt(errs, fn.line, "redefinition of method '" + fn.name +
          "' in impl '" + structName + "'");
  }
  methods[structName][fn.name] = mi;
  FuncSig sig;
  sig.isExtern = false;
  sig.retType = fn.retType;

  if (fn.hasSelf)
    sig.paramTypes.push_back(fn.selfByRef ? makePtr(st) : st);
  for (auto& p : fn.params) sig.paramTypes.push_back(p.type);
  funcs[mangled] = sig;
}


std::string Sema::instantiateGenericFn(const std::string& name, const std::vector<BType>& args) {
  const FuncDecl* tmpl = findGenericFn(name);
  if (!tmpl) return "";
  if (args.size() != tmpl->typeParams.size()) return "";
  std::string mangled = mangleInst(name, args);
  if (funcs.count(mangled)) return mangled;

  std::map<std::string, BType> env;
  for (size_t i = 0; i < tmpl->typeParams.size() && i < args.size(); i++)
    env[tmpl->typeParams[i]] = args[i];

  auto clone = std::make_unique<FuncDecl>();
  clone->name = mangled;
  clone->isExtern = false;
  clone->isGeneric = false;
  clone->line = tmpl->line;
  clone->retType = fixType(substitute(tmpl->retType, env));
  for (auto& p : tmpl->params) {
    Param np; np.name = p.name; np.type = fixType(substitute(p.type, env));
    clone->params.push_back(np);
  }


  for (auto& s : tmpl->body) clone->body.push_back(cloneStmt(s.get()));

  FuncSig sig; sig.isExtern = false;
  sig.retType = clone->retType;
  for (auto& p : clone->params) sig.paramTypes.push_back(p.type);
  funcs[mangled] = sig;
  FuncDecl* raw = clone.get();
  monomorphFns.push_back(std::move(clone));
  (void)raw;
  return mangled;
}


BType Sema::checkLambda(LambdaLit* lam) {
  std::string sym = "__oxfn_" + std::to_string(++lambdaSeq_);
  lam->retType = fixType(lam->retType);
  std::vector<BType> pts;
  for (auto& p : lam->params) { p.type = fixType(p.type); pts.push_back(p.type); }
  lam->fnType = makeFnType(pts, lam->retType);
  lam->loweredName = sym;


  FuncSig sig; sig.isExtern = false; sig.retType = lam->retType;
  for (auto& p : lam->params) sig.paramTypes.push_back(p.type);
  funcs[sym] = sig;


  curFunc_ = &funcs[sym];
  curImpl_.clear();


  std::vector<std::map<std::string, Entry>> saved = scopes_;
  while (scopes_.size() > 1) scopes_.pop_back();
  pushScope();
  for (auto& p : lam->params) declare(p.name, p.type, true);
  checkBlock(lam->body);
  popScope();
  scopes_ = saved;
  curFunc_ = nullptr;
  return lam->fnType;
}

BType Sema::checkExpr(Expr* e) {
  if (auto lam = dynamic_cast<LambdaLit*>(e)) {


    return checkLambda(lam);
  }
  if (auto l = dynamic_cast<IntLit*>(e)) return BType::i64;
  if (auto l = dynamic_cast<FloatLit*>(e)) return BType::f64;
  if (auto l = dynamic_cast<BoolLit*>(e)) return BType::bool_;
  if (auto l = dynamic_cast<StrLit*>(e)) return BType::str;
  if (auto l = dynamic_cast<CharLit*>(e)) return BType::char_;
  if (dynamic_cast<NullLit*>(e)) return makePtr(BType::void_);
  if (auto s = dynamic_cast<SizeofExpr*>(e)) {


    BType tt = fixType(s->target);
    s->target = tt;
    if (tt == BType::void_)
      errAt(errs, s->line, s->col, "sizeof(void) is not allowed");


    if (tt.tag == BType::Tag::struct_ && !findStruct(tt.structName))
      errAt(errs, s->line, s->col,
            "sizeof an unknown type '" + tt.structName + "'",
            "declare the struct before asking for its size");
    int32_t sz = fieldByteWidth(tt);


    if (sz <= 0) sz = 1;
    s->size = sz;
    return BType::i64;
  }
  if (auto a = dynamic_cast<AsmExpr*>(e)) {


    int outCount = 0;
    for (auto& io : a->ios) {
      BType t = checkExpr(io.val.get());
      io.ty = t;
      if (io.isOutput) { outCount++; a->outputTypes.push_back(t); }
    }
    if (outCount == 1) a->resultTy = a->outputTypes[0];
    else a->resultTy = BType::void_;
    return a->resultTy;
  }
  if (auto c = dynamic_cast<CastExpr*>(e)) {
    BType ft = checkExpr(c->e.get());
    BType tt = fixType(c->target);
    c->target = tt;
    bool ok = false;
    if (ft == tt) ok = true;
    else if (isInt(ft) && isInt(tt)) ok = true;
    else if (isNumeric(ft) && isNumeric(tt)) ok = true;
    else if ((ft.tag == BType::Tag::ptr && tt.tag == BType::Tag::ptr)) ok = true;
    else if ((ft.tag == BType::Tag::ptr || ft == BType::usize || ft == BType::u64 ||
              ft == BType::i64 || ft == BType::i32) &&
             (tt.tag == BType::Tag::ptr || tt == BType::usize || tt == BType::u64 ||
              tt == BType::i64 || tt == BType::i32))
      ok = true;
    else if (ft == BType::char_ && isInt(tt)) ok = true;
    else if (isInt(ft) && tt == BType::char_) ok = true;
    if (!ok) errAt(errs, c->line, c->col, "cast from " + typeSpelling(ft) +
                   " to " + typeSpelling(tt) + " is not allowed");
    return tt;
  }
  if (auto v = dynamic_cast<VarRef*>(e)) {
    VarInfo info = lookup(v->name);
    if (info.found) return info.type;

    auto [ed, ord] = resolveEnumVariant(v->name);
    if (ed) {

      v->line = v->line;

      return makeEnumType(ed->name);
    }
    std::vector<std::string> cands;
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
      for (auto& kv : *it) cands.push_back(kv.first);
    auto s = suggest(v->name, cands);
    errAt(errs, v->line, v->col, "use of undeclared variable '" + v->name + "'",
          s.empty() ? "" : "did you mean '" + s + "'?");
    return info.type;
  }
  if (auto u = dynamic_cast<UnaryExpr*>(e)) {
    BType bt = checkExpr(u->base.get());
    switch (u->op) {
      case UnaryExpr::Op::neg: {


        BType rt = bt;
        if (rt.tag == BType::Tag::ptr && pointee(rt).tag == BType::Tag::struct_)
          rt = pointee(rt);
        if (rt.tag == BType::Tag::struct_ &&
            methods.count(rt.structName) && methods[rt.structName].count("__neg")) {
          const MethodInfo& mi = methods[rt.structName].at("__neg");
          if (!mi.paramTypes.empty())
            errAt(errs, u->line, u->col, "__neg must take no arguments");
          u->methodOverload = true;
          u->overloadStruct = rt.structName;
          u->overloadMethod = "__neg";
          u->overloadRecvType = rt;
          u->recvByRef = mi.selfByRef;
          return mi.retType;
        }
        if (!isNumeric(bt))
          errAt(errs, u->line, "unary '-' requires a number");
        return bt;
      }
      case UnaryExpr::Op::not_:
        if (bt != BType::bool_) errAt(errs, u->line, "'!' requires a bool");
        return BType::bool_;
      case UnaryExpr::Op::bnot:
        if (!isInt(bt)) errAt(errs, u->line, "'~' requires an int");
        return BType::i64;
      case UnaryExpr::Op::addr:
        if (bt == BType::void_) {
          errAt(errs, u->line, "'&' requires an lvalue");
          return makePtr(BType::i64);
        }
        return makePtr(bt);
      case UnaryExpr::Op::deref:
        if (bt.tag != BType::Tag::ptr) {
          errAt(errs, u->line, "'*' requires a pointer");
          return BType::i64;
        }
        return pointee(bt);
    }
  }
  if (auto b = dynamic_cast<BinaryExpr*>(e)) {
    BType lt = checkExpr(b->lhs.get());
    BType rt = checkExpr(b->rhs.get());

    lt = coerceIntLit(b->lhs.get(), lt, rt);
    rt = coerceIntLit(b->rhs.get(), rt, lt);

    lt = coerceFloatLit(b->lhs.get(), lt, rt);
    rt = coerceFloatLit(b->rhs.get(), rt, lt);


    if (b->op != BinaryExpr::Op::land && b->op != BinaryExpr::Op::lor) {
      BType recvT = lt;
      if (recvT.tag == BType::Tag::ptr && pointee(recvT).tag == BType::Tag::struct_)
        recvT = pointee(recvT);
      if (recvT.tag == BType::Tag::struct_) {
        const MethodInfo* mi = resolveOverload(recvT.structName, b->op, b);
        if (mi) {
          bool isCmp = (b->op >= BinaryExpr::Op::eq && b->op <= BinaryExpr::Op::ge);
          bool wantParam = (b->op == BinaryExpr::Op::ne) || !isCmp;
          if (isCmp) {


          }

          size_t expect = 1;
          (void)wantParam;
          if (mi->paramTypes.size() != expect) {
            errAt(errs, b->line, b->col,
                  "'" + std::string(opMethodName(b->op)) + "' on '" + recvT.structName +
                  "' must take " + std::to_string(expect) + " argument(s), got " +
                  std::to_string(mi->paramTypes.size()));
          } else if (rt != mi->paramTypes[0] &&
                     !litAssignable(b->rhs.get(), rt, mi->paramTypes[0]) &&
                     !dynamic_cast<CastExpr*>(b->rhs.get())) {
            errAt(errs, b->rhs->line, "operator overload '" +
                  std::string(opMethodName(b->op)) + "': rhs type " +
                  typeSpelling(rt) + " does not match expected " +
                  typeSpelling(mi->paramTypes[0]));
          }
          b->methodOverload = true;
          b->overloadStruct = recvT.structName;
          b->overloadMethod = opMethodName(b->op);
          b->overloadRecvType = recvT;
          b->recvByRef = mi->selfByRef;


          if (b->op >= BinaryExpr::Op::eq && b->op <= BinaryExpr::Op::ge)
            return BType::bool_;
          return mi->retType;
        }
      }
    }
    switch (b->op) {
      case BinaryExpr::Op::add: case BinaryExpr::Op::sub:
      case BinaryExpr::Op::mul: case BinaryExpr::Op::div:
      case BinaryExpr::Op::mod:


        if (b->op == BinaryExpr::Op::add || b->op == BinaryExpr::Op::sub) {
          auto isPtrIntMix = [&](const BType& a, const BType& d) {
            return a.tag == BType::Tag::ptr && isInt(d);
          };
          if (isPtrIntMix(lt, rt) || isPtrIntMix(rt, lt)) {
            BType pt = (lt.tag == BType::Tag::ptr) ? lt : rt;
            b->isPtrArith = true;
            b->ptrArithPointee = pointee(pt);
            return pt;
          }
        }
        if (isNumeric(lt) && lt == rt) return lt;
        if (lt == BType::str && rt == BType::str && b->op == BinaryExpr::Op::add) return BType::str;

        if (b->op == BinaryExpr::Op::add &&
            ((lt == BType::str && rt == BType::char_) ||
             (lt == BType::char_ && rt == BType::str)))
          return BType::str;
        errAt(errs, b->line, "arithmetic operands must be the same numeric type");
        return lt == BType::void_ ? BType::i64 : lt;
      case BinaryExpr::Op::band: case BinaryExpr::Op::bor:
      case BinaryExpr::Op::bxor: case BinaryExpr::Op::shl:
      case BinaryExpr::Op::shr:
        if (isInt(lt) && lt == rt) return lt;
        errAt(errs, b->line, "bitwise operands must be the same int type");
        return BType::i64;
      case BinaryExpr::Op::eq: case BinaryExpr::Op::ne:
        if (lt == rt && lt != BType::void_) return BType::bool_;
        if (isNumeric(lt) && isNumeric(rt) && lt == rt) return BType::bool_;

        if (lt.tag == BType::Tag::enum_ && isIntLitExpr(b->rhs.get())) return BType::bool_;
        if (rt.tag == BType::Tag::enum_ && isIntLitExpr(b->lhs.get())) return BType::bool_;
        if (lt.tag == BType::Tag::ptr && rt.tag == BType::Tag::ptr) return BType::bool_;
        errAt(errs, b->line, "comparison operands must match");
        return BType::bool_;
      case BinaryExpr::Op::lt: case BinaryExpr::Op::gt:
      case BinaryExpr::Op::le: case BinaryExpr::Op::ge:
        if (lt == rt && lt != BType::void_) return BType::bool_;
        if (isNumeric(lt) && isNumeric(rt) && lt == rt) return BType::bool_;
        if (lt.tag == BType::Tag::ptr && rt.tag == BType::Tag::ptr) return BType::bool_;
        errAt(errs, b->line, "comparison operands must match");
        return BType::bool_;
      case BinaryExpr::Op::land: case BinaryExpr::Op::lor:
        if (lt != BType::bool_ || rt != BType::bool_)
          errAt(errs, b->line, "logical operands must be bool");
        return BType::bool_;
    }
  }
  if (auto t = dynamic_cast<TernaryExpr*>(e)) {


    BType ct = checkExpr(t->cond.get());
    if (ct != BType::bool_) errAt(errs, t->line, "ternary condition must be bool");
    BType tt = checkExpr(t->thenE.get());
    BType et = checkExpr(t->elseE.get());
    BType res = pickCommonType(t->thenE.get(), tt, t->elseE.get(), et, errs, t->line);
    t->resultTy = res;
    return res;
  }
  if (auto d = dynamic_cast<IncDecExpr*>(e)) {


    BType lvt = BType::void_;
    bool isMut = false, found = false;
    if (d->kind == AssignTarget::Kind::var) {
      VarInfo info = lookup(d->name);
      if (!info.found)
        errAt(errs, d->line, "increment/decrement of undeclared variable '" + d->name + "'");
      else if (!info.isMut)
        errAt(errs, d->line, "cannot ++/-- immutable variable '" + d->name + "' (use mut)");
      lvt = info.type; isMut = info.isMut; found = info.found;
    } else if (d->kind == AssignTarget::Kind::index) {
      BType baseT = checkExpr(d->base.get());
      if (baseT.tag == BType::Tag::ptr && (pointee(baseT).tag == BType::Tag::array ||
          pointee(baseT).tag == BType::Tag::dynarray || pointee(baseT).tag == BType::Tag::ptr))
        baseT = pointee(baseT);
      if (baseT.tag == BType::Tag::array) { lvt = arrayElem(baseT); found = true; }
      else if (baseT.tag == BType::Tag::dynarray) { lvt = dynArrayElem(baseT); found = true; }
      else if (baseT.tag == BType::Tag::ptr) { lvt = pointee(baseT); found = true; }
      else errAt(errs, d->line, "increment/decrement of indexed non-array/non-pointer");
      BType idxT = checkExpr(d->index.get());
      if (!isInt(idxT)) errAt(errs, d->line, "array index must be an int");
      (void)isMut;
    } else if (d->kind == AssignTarget::Kind::deref) {
      BType baseT = checkExpr(d->base.get());
      if (baseT.tag != BType::Tag::ptr)
        errAt(errs, d->line, "increment/decrement of '*p' requires a pointer");
      else { lvt = pointee(baseT); found = true; }
    } else if (d->kind == AssignTarget::Kind::field) {
      BType baseT = checkExpr(d->base.get());
      if (baseT.tag == BType::Tag::ptr && pointee(baseT).tag == BType::Tag::struct_)
        baseT = pointee(baseT);
      if (baseT.tag != BType::Tag::struct_) {
        errAt(errs, d->line, "increment/decrement of a field requires a struct value");
      } else {
        StructDef* sd = findStruct(baseT.structName);
        if (!sd) errAt(errs, d->line, "unknown struct '" + baseT.structName + "'");
        else if (structFieldIndex(sd, d->field) < 0)
          errAt(errs, d->line, "struct '" + baseT.structName + "' has no field '" + d->field + "'");
        else {
          int32_t fi = structFieldIndex(sd, d->field);
          if (sd->fields[fi].isPrivate && !canTouchPrivate(baseT.structName))
            errAt(errs, d->line, d->col,
                  "field '" + d->field + "' of '" + baseT.structName + "' is private");
          lvt = sd->fields[fi].type; found = true;
        }
      }
    }
    if (found && lvt.tag != BType::Tag::void_) {
      bool ok = isInt(lvt) || lvt == BType::char_ || lvt == BType::usize ||
                lvt == BType::bool_ || lvt.tag == BType::Tag::ptr;
      if (!ok)
        errAt(errs, d->line, "++/-- requires an int, char, usize, bool, or pointer operand");
    }
    d->valueTy = found ? lvt : BType::i64;
    return d->valueTy;
  }
  if (auto a = dynamic_cast<AssignTarget*>(e)) {

    BType lvt = BType::void_;
    bool isMut = false, found = false;
    if (a->kind == AssignTarget::Kind::var) {
      VarInfo info = lookup(a->name);
      if (!info.found)
        errAt(errs, a->line, "assignment to undeclared variable '" + a->name + "'");
      else if (!info.isMut)
        errAt(errs, a->line, "cannot assign to immutable variable '" + a->name + "' (use mut)");
      lvt = info.type; isMut = info.isMut; found = info.found;
    } else if (a->kind == AssignTarget::Kind::index) {
      BType baseT = checkExpr(a->base.get());

      if (baseT.tag == BType::Tag::ptr && (pointee(baseT).tag == BType::Tag::array ||
          pointee(baseT).tag == BType::Tag::dynarray || pointee(baseT).tag == BType::Tag::ptr))
        baseT = pointee(baseT);
      if (baseT.tag == BType::Tag::array) {
        lvt = arrayElem(baseT); found = true;
      } else if (baseT.tag == BType::Tag::dynarray) {
        lvt = dynArrayElem(baseT); found = true;
      } else if (baseT.tag == BType::Tag::ptr) {
        lvt = pointee(baseT); found = true;
      } else {
        errAt(errs, a->line, "indexed assignment requires an array or pointer value");
      }
      BType idxT = checkExpr(a->index.get());
      if (!isInt(idxT)) errAt(errs, a->line, "array index must be an int");
      (void)isMut;
    } else if (a->kind == AssignTarget::Kind::deref) {
      BType baseT = checkExpr(a->base.get());
      if (baseT.tag != BType::Tag::ptr) {
        errAt(errs, a->line, "deref assignment requires a pointer ('*p = v')");
      } else {
        lvt = pointee(baseT); found = true;
      }
    } else if (a->kind == AssignTarget::Kind::field) {
      BType baseT = checkExpr(a->base.get());

      if (baseT.tag == BType::Tag::ptr && pointee(baseT).tag == BType::Tag::struct_)
        baseT = pointee(baseT);
      if (baseT.tag != BType::Tag::struct_) {
        errAt(errs, a->line, "field assignment requires a struct value");
      } else {
        StructDef* d = findStruct(baseT.structName);
        if (!d) {
          errAt(errs, a->line, "unknown struct '" + baseT.structName + "'");
        } else if (structFieldIndex(d, a->field) < 0) {
          errAt(errs, a->line, "struct '" + baseT.structName + "' has no field '" + a->field + "'");
          lvt = BType::void_; found = true;
        } else {
          int32_t fi = structFieldIndex(d, a->field);
          if (d->fields[fi].isPrivate && !canTouchPrivate(baseT.structName)) {
            errAt(errs, a->line, a->col, "field '" + a->field + "' of '" + baseT.structName +
                  "' is private", "mutate it through the struct's impl methods");
          }
          lvt = d->fields[fi].type;
          found = true;
        }
      }
    }
    BType rtype = checkExpr(a->value.get());


    if (found && lvt.tag != BType::Tag::void_) {
      BType recvT = lvt;
      if (recvT.tag == BType::Tag::ptr && pointee(recvT).tag == BType::Tag::struct_)
        recvT = pointee(recvT);
      if (recvT.tag == BType::Tag::struct_) {
        const char* mname = a->isCompound
            ? opIMethodName(a->compound)
            : "__assign";
        auto sit = methods.find(recvT.structName);
        if (sit != methods.end() && mname && mname[0] &&
            sit->second.count(mname)) {
          const MethodInfo& mi = sit->second.at(mname);
          if (mi.paramTypes.size() != 1)
            errAt(errs, a->line, a->col,
                  std::string("'") + mname + "' on '" + recvT.structName +
                  "' must take 1 argument, got " + std::to_string(mi.paramTypes.size()));
          else if (rtype != mi.paramTypes[0] &&
                   !litAssignable(a->value.get(), rtype, mi.paramTypes[0]) &&
                   !dynamic_cast<CastExpr*>(a->value.get()))
            errAt(errs, a->value->line,
                  std::string("operator overload '") + mname + "': rhs type " +
                  typeSpelling(rtype) + " does not match expected " +
                  typeSpelling(mi.paramTypes[0]));
          a->methodOverload = true;
          a->overloadStruct = recvT.structName;
          a->overloadMethod = mname;
          a->overloadRecvType = recvT;
          return mi.retType;
        }
      }
    }
    if (a->isCompound) {

      if (found && lvt != BType::void_ && rtype != lvt)
        errAt(errs, a->line, "compound assignment type mismatch");


      if (found && lvt == BType::str && a->compound != BinaryExpr::Op::add)
        errAt(errs, a->line, a->col,
              "only '+=' is supported on a str (concatenation)");
      return lvt == BType::void_ ? rtype : lvt;
    }
    if (found && lvt != BType::void_ && rtype != lvt)
      errAt(errs, a->line, "assignment type mismatch");
    return lvt == BType::void_ ? rtype : lvt;
  }
  if (auto c = dynamic_cast<Call*>(e)) {

    static const std::map<std::string, std::pair<BType, int>> builtins = {
      {"abs",   {BType::i64, 1}},
      {"imin",  {BType::i64, 2}},
      {"imax",  {BType::i64, 2}},
      {"fmin",  {BType::f64, 2}},
      {"fmax",  {BType::f64, 2}},
      {"sqrt",  {BType::f64, 1}},
      {"itos",  {BType::str, 1}},
      {"stoi",  {BType::i64, 1}},
      {"stod",  {BType::f64, 1}},
      {"ftos",  {BType::str, 1}},
      {"char_to_str", {BType::str, 1}},
      {"substr",   {BType::str, 3}},
      {"index_of", {BType::i64, 2}},
      {"read_line", {BType::str, 0}},
      {"read_file", {BType::str, 1}},
      {"file_open",  {BType::i64, 2}},
      {"file_close", {BType::i64, 1}},
      {"file_read",  {BType::str, 1}},
      {"file_write", {BType::i64, 2}},
      {"file_exists",{BType::bool_, 1}},
      {"len",   {BType::i64, 1}},
      {"push",  {BType::void_, 2}},
      {"sort",  {BType::void_, 1}},
      {"map_len",      {BType::i64,  1}},
      {"map_contains",  {BType::bool_, 2}},
      {"map_set",       {BType::void_, 3}},
      {"map_get",       {BType::void_, 2}},
      {"map_keys",      {BType::void_, 1}},
      {"set_len",       {BType::i64,  1}},
      {"set_contains",  {BType::bool_, 2}},
      {"set_insert",    {BType::void_, 2}},
      {"set_remove",    {BType::void_, 2}},
      {"set_to_vec",   {BType::void_, 1}},
      {"hmap_len",      {BType::i64,  1}},
      {"hmap_contains", {BType::bool_, 2}},
      {"hmap_set",      {BType::void_, 3}},
      {"hmap_get",      {BType::void_, 2}},
      {"hmap_keys",     {BType::void_, 1}},
      {"hset_len",       {BType::i64,  1}},
      {"hset_contains",  {BType::bool_, 2}},
      {"hset_insert",    {BType::void_, 2}},
      {"hset_remove",    {BType::void_, 2}},
      {"hset_to_vec",   {BType::void_, 1}},
      {"print",  {BType::void_, 0}},
    };
    if (c->isPrint) {
      for (auto& arg : c->args) checkExpr(arg.get());
      if (freestanding)
        errAt(errs, c->line, c->col, "print requires the hosted runtime (--freestanding omits it)",
              "extern your own output routine, or build hosted (drop --freestanding / --no-rt)");
      return BType::void_;
    }


    if (findGenericFn(c->callee)) {
      std::vector<BType> args;
      if (c->hasTypeArgs) {
        for (auto& a : c->typeArgs) args.push_back(fixType(a));
      } else {
        const FuncDecl* tmpl = findGenericFn(c->callee);
        if (tmpl->typeParams.size() == 1 && c->args.size() == 1) {
          BType at = checkExpr(c->args[0].get());
          args.push_back(at);
        } else if (tmpl->typeParams.empty()) {


        } else {
          errAt(errs, c->line, c->col,
                "generic call to '" + c->callee + "' needs explicit type arguments " +
                "(e.g. " + c->callee + "<" + tmpl->typeParams[0] + ">(...))");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::void_;
        }
      }
      std::string mangled = instantiateGenericFn(c->callee, args);
      if (mangled.empty()) {
        errAt(errs, c->line, c->col, "generic instantiation of '" + c->callee + "' failed");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }

      c->callee = mangled;
      c->hasTypeArgs = false;
      c->typeArgs.clear();
    } else if (c->hasTypeArgs) {
      errAt(errs, c->line, c->col,
            "'" + c->callee + "' is not a generic function");
      for (auto& a : c->args) checkExpr(a.get());
      return BType::void_;
    }


    if (!funcs.count(c->callee) && !c->fnPtr) {
      VarInfo vi = lookup(c->callee);
      if (!vi.found) {
        auto it = globals.find(c->callee); if (it != globals.end()) { vi.type = it->second.type; vi.found = true; }
      }
      if (vi.found && vi.type.tag == BType::Tag::fn_) {
        c->fnPtr = true;
        auto v = std::make_unique<VarRef>(); v->name = c->callee; v->line = c->line; v->col = c->col;
        c->calleeExpr = std::move(v);
        const auto& ps = fnParams(vi.type);
        BType ret = fnRet(vi.type);
        if (c->args.size() != ps.size() &&
            !(ps.size() == 0 && false)) {
        }
        if (c->args.size() != ps.size())
          errAt(errs, c->line, c->col, "indirect call expects " + std::to_string(ps.size()) +
                " args, got " + std::to_string(c->args.size()));
        for (size_t k = 0; k < c->args.size(); k++) {
          BType at = checkExpr(c->args[k].get());
          BType pt = (k < ps.size()) ? ps[k] : BType::void_;
          bool ok = (at == pt) || litAssignable(c->args[k].get(), at, pt);
          if (!ok && pt.tag == BType::Tag::ptr && at == pointee(pt) && isLvalueExpr(c->args[k].get())) ok = true;
          if (!ok)
            errAt(errs, c->args[k]->line, "argument " + std::to_string(k + 1) +
                  " of indirect call has the wrong type (expected " + typeSpelling(pt) + ")");
        }
        return ret;
      }
    }


    if (freestanding && !funcs.count(c->callee)) {
      static const std::set<std::string> rtBuiltins = {
        "len","push","sort","read_line","read_file","file_open","file_close","file_read",
        "file_write","file_exists","itos","stoi","stod","ftos","char_to_str",
        "substr","index_of","abs",
        "map_len","map_contains","map_set","map_get","map_keys",
        "set_len","set_contains","set_insert","set_remove","set_to_vec",
        "hmap_len","hmap_contains","hmap_set","hmap_get","hmap_keys",
        "hset_len","hset_contains","hset_insert","hset_remove","hset_to_vec"
      };
      if (rtBuiltins.count(c->callee))
        errAt(errs, c->line, c->col,
              "'" + c->callee + "' requires the hosted runtime (--freestanding omits it)",
              "declare an `extern fn " + c->callee + "(...) -> ...;` with your own implementation");
    }


    auto checkElemKey = [&](int argIdx, BType want) {
      BType got = checkExpr(c->args[argIdx].get());
      if (got != want && !litAssignable(c->args[argIdx].get(), got, want) &&
          !dynamic_cast<CastExpr*>(c->args[argIdx].get()))
        errAt(errs, c->args[argIdx]->line, c->args[argIdx]->col,
              c->callee + " element type does not match (expected " +
              typeSpelling(want) + ")");
    };

    auto keyOk = [&](const BType& k) {
      return isScalar(k) || k.tag == BType::Tag::enum_;
    };
    auto needContainer = [&](int aidx, BType::Tag want, const char* kind) -> BType {
      BType t = checkExpr(c->args[aidx].get());
      if (t.tag != want)
        errAt(errs, c->args[aidx]->line, c->args[aidx]->col,
              c->callee + " expects a " + kind + " as argument " +
              std::to_string(aidx + 1));
      return t;
    };
    if (!funcs.count(c->callee)) {
      const std::string& cc = c->callee;
      if (cc == "map_len" || cc == "hmap_len" || cc == "set_len" || cc == "hset_len") {
        if (c->args.size() != 1) {
          errAt(errs, c->line, c->col, cc + " expects 1 argument");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::i64;
        }
        if (cc == "map_len") (void)needContainer(0, BType::Tag::map_, "map");
        else if (cc == "hmap_len") (void)needContainer(0, BType::Tag::hmap_, "hash map");
        else if (cc == "set_len") (void)needContainer(0, BType::Tag::set_, "set");
        else (void)needContainer(0, BType::Tag::hset_, "hash set");
        return BType::i64;
      }
      if (cc == "map_contains" || cc == "hmap_contains" ||
          cc == "set_contains" || cc == "hset_contains") {
        if (c->args.size() != 2) {
          errAt(errs, c->line, c->col, cc + " expects 2 arguments (container, key)");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::bool_;
        }
        BType ct = (cc=="map_contains") ? needContainer(0, BType::Tag::map_, "map")
                 : (cc=="hmap_contains") ? needContainer(0, BType::Tag::hmap_, "hash map")
                 : (cc=="set_contains") ? needContainer(0, BType::Tag::set_, "set")
                 : needContainer(0, BType::Tag::hset_, "hash set");
        BType want = (ct.tag == BType::Tag::map_ || ct.tag == BType::Tag::hmap_)
                     ? mapKeyType(ct) : setElemType(ct);
        checkElemKey(1, want);
        return BType::bool_;
      }
      if (cc == "map_set" || cc == "hmap_set") {
        if (c->args.size() != 3) {
          errAt(errs, c->line, c->col, cc + " expects 3 arguments (map, key, value)");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::void_;
        }
        BType ct = (cc=="map_set") ? needContainer(0, BType::Tag::map_, "map")
                                   : needContainer(0, BType::Tag::hmap_, "hash map");
        checkElemKey(1, mapKeyType(ct));
        checkElemKey(2, mapValType(ct));
        return BType::void_;
      }
      if (cc == "set_insert" || cc == "set_remove" || cc == "hset_insert" || cc == "hset_remove") {
        if (c->args.size() != 2) {
          errAt(errs, c->line, c->col, cc + " expects 2 arguments (set, element)");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::void_;
        }
        BType ct = (cc=="set_insert"||cc=="set_remove") ? needContainer(0, BType::Tag::set_, "set")
                                                       : needContainer(0, BType::Tag::hset_, "hash set");
        checkElemKey(1, setElemType(ct));
        return BType::void_;
      }
      if (cc == "map_get" || cc == "hmap_get") {
        if (c->args.size() != 2) {
          errAt(errs, c->line, c->col, cc + " expects 2 arguments (map, key)");
          for (auto& a : c->args) checkExpr(a.get());
          return BType::i64;
        }
        BType ct = (cc=="map_get") ? needContainer(0, BType::Tag::map_, "map")
                                   : needContainer(0, BType::Tag::hmap_, "hash map");
        checkElemKey(1, mapKeyType(ct));
        if (ct.tag == BType::Tag::map_ || ct.tag == BType::Tag::hmap_) return mapValType(ct);
        return BType::i64;
      }
      if (cc == "map_keys" || cc == "hmap_keys") {
        if (c->args.size() != 1) {
          errAt(errs, c->line, c->col, cc + " expects 1 argument (map)");
          for (auto& a : c->args) checkExpr(a.get());
          return makeDynArray(BType::i64);
        }
        BType ct = (cc=="map_keys") ? needContainer(0, BType::Tag::map_, "map")
                                    : needContainer(0, BType::Tag::hmap_, "hash map");
        if (ct.tag == BType::Tag::map_ || ct.tag == BType::Tag::hmap_)
          return makeDynArray(mapKeyType(ct));
        return makeDynArray(BType::i64);
      }
      if (cc == "set_to_vec" || cc == "hset_to_vec") {
        if (c->args.size() != 1) {
          errAt(errs, c->line, c->col, cc + " expects 1 argument (set)");
          for (auto& a : c->args) checkExpr(a.get());
          return makeDynArray(BType::i64);
        }
        BType ct = (cc=="set_to_vec") ? needContainer(0, BType::Tag::set_, "set")
                                      : needContainer(0, BType::Tag::hset_, "hash set");
        if (ct.tag == BType::Tag::set_ || ct.tag == BType::Tag::hset_)
          return makeDynArray(setElemType(ct));
        return makeDynArray(BType::i64);
      }
    }


    if (!funcs.count(c->callee) && c->callee == "mmio_load") {
      if (c->args.size() != 1) {
        errAt(errs, c->line, c->col, "mmio_load expects 1 argument (a *T pointer)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::i64;
      }
      BType pt = checkExpr(c->args[0].get());
      if (pt.tag != BType::Tag::ptr)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "mmio_load requires a pointer (use &T / *T), got " + typeSpelling(pt));
      else if (!isScalar(pointee(pt)) && pointee(pt).tag != BType::Tag::ptr)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "mmio_load requires a *T where T is a scalar (got *" + typeSpelling(pointee(pt)) + ")");
      return pt.tag == BType::Tag::ptr ? pointee(pt) : BType::i64;
    }
    if (!funcs.count(c->callee) && c->callee == "mmio_store") {
      if (c->args.size() != 2) {
        errAt(errs, c->line, c->col, "mmio_store expects 2 arguments (*T, value)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }
      BType pt = checkExpr(c->args[0].get());
      if (pt.tag != BType::Tag::ptr)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "mmio_store requires a pointer (use &T / *T), got " + typeSpelling(pt));
      BType vt = checkExpr(c->args[1].get());
      BType want = (pt.tag == BType::Tag::ptr) ? pointee(pt) : BType::void_;
      if (want != BType::void_ && vt != want &&
          !litAssignable(c->args[1].get(), vt, want) &&
          !dynamic_cast<CastExpr*>(c->args[1].get()))
        errAt(errs, c->args[1]->line, c->args[1]->col,
              "mmio_store value type does not match pointer pointee");
      return BType::void_;
    }
    if (!funcs.count(c->callee) && c->callee == "memset") {
      if (c->args.size() != 3) {
        errAt(errs, c->line, c->col, "memset expects 3 arguments (*T, u8 fill, i64 byte count)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }
      BType pt = checkExpr(c->args[0].get());
      if (pt.tag != BType::Tag::ptr)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "memset requires a pointer as the first argument, got " + typeSpelling(pt));
      BType fv = checkExpr(c->args[1].get());
      if (!isInt(fv)) errAt(errs, c->args[1]->line, c->args[1]->col, "memset fill must be an integer (u8)");
      BType cv = checkExpr(c->args[2].get());
      if (!isInt(cv)) errAt(errs, c->args[2]->line, c->args[2]->col, "memset count must be an integer (i64)");
      return BType::void_;
    }
    if (!funcs.count(c->callee) && c->callee == "memcpy") {
      if (c->args.size() != 3) {
        errAt(errs, c->line, c->col, "memcpy expects 3 arguments (*dst, *src, i64 byte count)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }
      BType dpt = checkExpr(c->args[0].get());
      BType spt = checkExpr(c->args[1].get());
      if (dpt.tag != BType::Tag::ptr)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "memcpy requires a pointer as the first argument, got " + typeSpelling(dpt));
      if (spt.tag != BType::Tag::ptr)
        errAt(errs, c->args[1]->line, c->args[1]->col,
              "memcpy requires a pointer as the second argument, got " + typeSpelling(spt));
      BType cv = checkExpr(c->args[2].get());
      if (!isInt(cv)) errAt(errs, c->args[2]->line, c->args[2]->col, "memcpy count must be an integer (i64)");
      return BType::void_;
    }


    if (!funcs.count(c->callee) && c->callee == "str_ptr") {
      if (c->args.size() != 1) {
        errAt(errs, c->line, c->col, "str_ptr expects 1 argument (a str)");
        for (auto& a : c->args) checkExpr(a.get());
        return makePtr(BType::u8);
      }
      BType st = checkExpr(c->args[0].get());
      if (st != BType::str)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "str_ptr requires a str, got " + typeSpelling(st));
      return makePtr(BType::u8);
    }

    if (!funcs.count(c->callee) && c->callee == "len") {
      if (c->args.size() != 1) {
        errAt(errs, c->line, c->col, "len expects 1 argument");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::i64;
      }
      BType t = checkExpr(c->args[0].get());
      if (t.tag != BType::Tag::array && t.tag != BType::Tag::dynarray &&
          t != BType::str &&
          t.tag != BType::Tag::map_ && t.tag != BType::Tag::set_ &&
          t.tag != BType::Tag::hmap_ && t.tag != BType::Tag::hset_)
        errAt(errs, c->args[0]->line, c->args[0]->col,
              "len requires an array, string, map, or set");
      return BType::i64;
    }
    if (!funcs.count(c->callee) && c->callee == "push") {
      if (c->args.size() != 2) {
        errAt(errs, c->line, c->col, "push expects 2 arguments (vec, value)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }
      BType vt = checkExpr(c->args[0].get());
      if (vt.tag != BType::Tag::dynarray) {
        errAt(errs, c->args[0]->line, c->args[0]->col, "push requires a vec as the first argument");
      } else {
        BType et = checkExpr(c->args[1].get());
        BType want = dynArrayElem(vt);
        if (et != want && !litAssignable(c->args[1].get(), et, want) &&
            !dynamic_cast<CastExpr*>(c->args[1].get()))
          errAt(errs, c->args[1]->line, c->args[1]->col, "push value type does not match vec element type");
      }
      return BType::void_;
    }
    if (!funcs.count(c->callee) && c->callee == "sort") {
      if (c->args.size() != 1) {
        errAt(errs, c->line, c->col, "sort expects 1 argument (a vec)");
        for (auto& a : c->args) checkExpr(a.get());
        return BType::void_;
      }
      BType vt = checkExpr(c->args[0].get());
      if (vt.tag != BType::Tag::dynarray) {
        errAt(errs, c->args[0]->line, c->args[0]->col, "sort requires a vec as the argument");
      } else {
        BType et = dynArrayElem(vt);


        if (!isScalar(et) && et.tag != BType::Tag::enum_)
          errAt(errs, c->args[0]->line, c->args[0]->col,
                "sort requires a vec of scalar/str/enum elements (got " +
                typeSpelling(et) + ")");
      }
      return BType::void_;
    }
    auto bit = builtins.find(c->callee);
    if (bit != builtins.end() && !funcs.count(c->callee)) {

      for (auto& arg : c->args) {
        BType at = checkExpr(arg.get());
        (void)at;
      }

      if (c->callee == "abs") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (!isNumeric(t))
          errAt(errs, c->line, c->col, "abs requires a numeric argument",
                 "abs(int) or abs(f64) — abs returns the same type");
      } else if (c->callee == "sqrt") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::f64) errAt(errs, c->line, "sqrt requires f64");
      } else if (c->callee == "itos") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::i64) errAt(errs, c->line, "itos requires i64");
      } else if (c->callee == "stoi" || c->callee == "stod") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::str) errAt(errs, c->line, c->callee + " requires str");
      } else if (c->callee == "ftos") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::f64) errAt(errs, c->line, "ftos requires f64");
      } else if (c->callee == "char_to_str") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::char_ && !isIntLitExpr(c->args[0].get()))
          errAt(errs, c->line, "char_to_str requires char");
      } else if (c->callee == "substr") {
        if (c->args.size() != 3) {
          errAt(errs, c->line, "substr expects 3 args (str, start, len)");
        } else {
          BType t0 = checkExpr(c->args[0].get());
          BType t1 = checkExpr(c->args[1].get());
          BType t2 = checkExpr(c->args[2].get());
          if (t0 != BType::str) errAt(errs, c->args[0]->line, "substr requires str");
          if (!isInt(t1) && !isIntLitExpr(c->args[1].get()))
            errAt(errs, c->args[1]->line, "substr start must be an int");
          if (!isInt(t2) && !isIntLitExpr(c->args[2].get()))
            errAt(errs, c->args[2]->line, "substr len must be an int");
        }
      } else if (c->callee == "index_of") {
        if (c->args.size() != 2) {
          errAt(errs, c->line, "index_of expects 2 args (str, char)");
        } else {
          BType t0 = checkExpr(c->args[0].get());
          BType t1 = checkExpr(c->args[1].get());
          if (t0 != BType::str) errAt(errs, c->args[0]->line, "index_of requires str");
          if (t1 != BType::char_ && !isIntLitExpr(c->args[1].get()))
            errAt(errs, c->args[1]->line, "index_of char must be a char");
        }
      } else if (c->callee == "read_file" || c->callee == "file_exists") {
        BType t = c->args.empty() ? BType::void_ : checkExpr(c->args[0].get());
        if (t != BType::str) errAt(errs, c->line, c->callee + " requires str");
      } else {
        if ((int)c->args.size() != bit->second.second)
          errAt(errs, c->line, c->callee + " expects " +
                std::to_string(bit->second.second) + " args, got " +
                std::to_string(c->args.size()));
      }


      if (c->callee == "abs" && !c->args.empty()) {
        BType at = checkExpr(c->args[0].get());
        if (isNumeric(at)) return at;
      }
      return bit->second.first;
    }
    auto it = funcs.find(c->callee);
    if (it == funcs.end()) {
      std::vector<std::string> fc;
      for (auto& kv : funcs) fc.push_back(kv.first);
      static const char* builtin[] = {"abs","imin","imax","fmin","fmax","sqrt",
        "itos","stoi","stod","ftos","char_to_str","substr","index_of",
        "read_line","read_file","file_open","file_close",
        "file_read","file_write","file_exists","print"};
      for (auto b : builtin) fc.push_back(b);
      auto s = suggest(c->callee, fc);
      errAt(errs, c->line, c->col, "call to undeclared function '" + c->callee + "'",
            s.empty() ? "" : "did you mean '" + s + "'?");
      for (auto& arg : c->args) checkExpr(arg.get());
      return BType::void_;
    }
    if (c->args.size() != it->second.paramTypes.size()) {
      errAt(errs, c->line, "function '" + c->callee + "' expects " +
            std::to_string(it->second.paramTypes.size()) + " args, got " +
            std::to_string(c->args.size()));
    }
    for (size_t k = 0; k < c->args.size(); k++) {
      BType at = checkExpr(c->args[k].get());
      BType pt = (k < it->second.paramTypes.size()) ? it->second.paramTypes[k] : BType::void_;
      bool ok = (at == pt);
      if (!ok) ok = litAssignable(c->args[k].get(), at, pt);
      if (!ok) ok = (dynamic_cast<CastExpr*>(c->args[k].get()) != nullptr);

      if (!ok && pt.tag == BType::Tag::ptr && at == pointee(pt) && isLvalueExpr(c->args[k].get()))
        ok = true;
      if (!ok)
        errAt(errs, c->args[k]->line, "argument " + std::to_string(k + 1) +
              " of '" + c->callee + "' has wrong type");
    }
    return it->second.retType;
  }
  if (auto ix = dynamic_cast<Index*>(e)) {
    BType baseT = checkExpr(ix->base.get());


    if (baseT.tag == BType::Tag::ptr && pointee(baseT).tag == BType::Tag::array)
      baseT = pointee(baseT);
    BType idxT = checkExpr(ix->index.get());


    BType recvT = baseT;
    if (recvT.tag == BType::Tag::ptr && pointee(recvT).tag == BType::Tag::struct_)
      recvT = pointee(recvT);
    if (recvT.tag == BType::Tag::struct_ &&
        methods.count(recvT.structName) && methods[recvT.structName].count("__index")) {
      const MethodInfo& mi = methods[recvT.structName].at("__index");
      if (mi.paramTypes.size() != 1) {
        errAt(errs, ix->line, ix->col, "__index must take 1 argument, got " +
              std::to_string(mi.paramTypes.size()));
      } else if (idxT != mi.paramTypes[0] &&
                 !litAssignable(ix->index.get(), idxT, mi.paramTypes[0]) &&
                 !dynamic_cast<CastExpr*>(ix->index.get())) {
        errAt(errs, ix->index->line, "__index argument type " + typeSpelling(idxT) +
              " does not match expected " + typeSpelling(mi.paramTypes[0]));
      }
      ix->methodOverload = true;
      ix->overloadStruct = recvT.structName;
      ix->overloadMethod = "__index";
      ix->overloadRecvType = recvT;
      ix->recvByRef = mi.selfByRef;
      return mi.retType;
    }
    if (!isInt(idxT)) errAt(errs, ix->line, "array index must be an int");
    if (baseT.tag == BType::Tag::array) return arrayElem(baseT);
    if (baseT.tag == BType::Tag::dynarray) return dynArrayElem(baseT);
    if (baseT.tag == BType::Tag::ptr) return pointee(baseT);
    if (baseT == BType::str) return BType::char_;
    errAt(errs, ix->line, "cannot index a non-array value");
    return BType::i64;
  }
  if (auto dn = dynamic_cast<DynNew*>(e)) {


    dn->elemType = fixType(dn->elemType);
    if (fieldByteWidth(dn->elemType) <= 0) {
      errAt(errs, e->line, 0, "vec element type has no known size (use a scalar, struct, array, or nested vec)",
            "a vec element must have a fixed compile-time byte size");
    }
    return makeDynArray(dn->elemType);
  }
  if (auto mn = dynamic_cast<MapNew*>(e)) {


    mn->keyType = fixType(mn->keyType);
    mn->valType = fixType(mn->valType);
    if (!isScalar(mn->keyType) && mn->keyType.tag != BType::Tag::enum_)
      errAt(errs, e->line, 0, "map keys must be scalar/str/enum (got " + typeSpelling(mn->keyType) + ")");
    else if (mn->keyType == BType::void_)
      errAt(errs, e->line, 0, "map keys may not be void");
    if (fieldByteWidth(mn->valType) <= 0 && mn->valType.tag != BType::Tag::map_ &&
        mn->valType.tag != BType::Tag::hmap_ && mn->valType.tag != BType::Tag::dynarray &&
        mn->valType.tag != BType::Tag::set_ && mn->valType.tag != BType::Tag::hset_)
      errAt(errs, e->line, 0, "map value type has no known size (got " + typeSpelling(mn->valType) + ")",
            "values may be scalars, str, structs, fixed arrays, vec, or another collection");
    return makeMapType(mn->keyType, mn->valType);
  }
  if (auto hmn = dynamic_cast<HMapNew*>(e)) {

    hmn->keyType = fixType(hmn->keyType);
    hmn->valType = fixType(hmn->valType);
    if (!isScalar(hmn->keyType) && hmn->keyType.tag != BType::Tag::enum_)
      errAt(errs, e->line, 0, "hmap keys must be scalar/str/enum (got " + typeSpelling(hmn->keyType) + ")");
    else if (hmn->keyType == BType::void_)
      errAt(errs, e->line, 0, "hmap keys may not be void");
    if (fieldByteWidth(hmn->valType) <= 0 && hmn->valType.tag != BType::Tag::map_ &&
        hmn->valType.tag != BType::Tag::hmap_ && hmn->valType.tag != BType::Tag::dynarray &&
        hmn->valType.tag != BType::Tag::set_ && hmn->valType.tag != BType::Tag::hset_)
      errAt(errs, e->line, 0, "hmap value type has no known size (got " + typeSpelling(hmn->valType) + ")",
            "values may be scalars, str, structs, fixed arrays, vec, or another collection");
    return makeHMapType(hmn->keyType, hmn->valType);
  }
  if (auto sn = dynamic_cast<SetNew*>(e)) {

    sn->elemType = fixType(sn->elemType);
    if (!isScalar(sn->elemType) && sn->elemType.tag != BType::Tag::enum_)
      errAt(errs, e->line, 0, "set elements must be scalar/str/enum (got " + typeSpelling(sn->elemType) + ")");
    else if (sn->elemType == BType::void_)
      errAt(errs, e->line, 0, "set elements may not be void");
    return makeSetType(sn->elemType);
  }
  if (auto hsn = dynamic_cast<HSetNew*>(e)) {

    hsn->elemType = fixType(hsn->elemType);
    if (!isScalar(hsn->elemType) && hsn->elemType.tag != BType::Tag::enum_)
      errAt(errs, e->line, 0, "hset elements must be scalar/str/enum (got " + typeSpelling(hsn->elemType) + ")");
    else if (hsn->elemType == BType::void_)
      errAt(errs, e->line, 0, "hset elements may not be void");
    return makeHSetType(hsn->elemType);
  }
  if (auto fl = dynamic_cast<Field*>(e)) {
    BType baseT = checkExpr(fl->base.get());

    if (baseT.tag == BType::Tag::ptr && pointee(baseT).tag == BType::Tag::struct_)
      baseT = pointee(baseT);
    if (baseT.tag != BType::Tag::struct_) {
      errAt(errs, fl->line, "field access requires a struct value");
      return BType::i64;
    }
    StructDef* d = findStruct(baseT.structName);
    if (!d) {
      errAt(errs, fl->line, "unknown struct '" + baseT.structName + "'");
      return BType::i64;
    }
    int32_t fi = structFieldIndex(d, fl->field);
    if (fi < 0) {
      errAt(errs, fl->line, "struct '" + baseT.structName + "' has no field '" + fl->field + "'");
      return BType::i64;
    }
    if (d->fields[fi].isPrivate && !canTouchPrivate(baseT.structName)) {
      errAt(errs, fl->line, fl->col, "field '" + fl->field + "' of '" + baseT.structName +
            "' is private", "access it through the struct's impl methods");
      return d->fields[fi].type;
    }
    return d->fields[fi].type;
  }
  if (auto al = dynamic_cast<ArrayLit*>(e)) {
    if (al->elems.empty()) {
      errAt(errs, al->line, "array literal must have at least one element");
      return makeArrayType(BType::i64, 0);
    }
    BType et = checkExpr(al->elems[0].get());
    for (size_t i = 1; i < al->elems.size(); i++) {
      BType t = checkExpr(al->elems[i].get());


      t = coerceIntLit(al->elems[i].get(), t, et);
      t = coerceFloatLit(al->elems[i].get(), t, et);
      if (t != et && !(isInt(et) && isIntLitExpr(al->elems[i].get())) &&
          !litAssignable(al->elems[i].get(), t, et))
        errAt(errs, al->elems[i]->line, "array literal elements must have the same type");
    }
    return makeArrayType(et, (int32_t)al->elems.size());
  }
  if (auto sl = dynamic_cast<StructLit*>(e)) {


    if (sl->hasTypeArgs) {
      std::vector<BType> args; for (auto& a : sl->typeArgs) args.push_back(fixType(a));
      BType inst = instantiateGenericStruct(sl->name, args);
      sl->name = inst.structName;
      sl->hasTypeArgs = false;
    }
    StructDef* d = findStruct(sl->name);
    if (!d) {
      errAt(errs, sl->line, "unknown struct '" + sl->name + "'");
      for (auto& v : sl->values) checkExpr(v.get());
      return sl->name.empty() ? BType::void_ : (BType{BType::Tag::struct_, 0, 0, sl->name});
    }

    std::vector<char> seen(d->fields.size(), 0);
    for (size_t i = 0; i < sl->fieldNames.size() && i < sl->values.size(); i++) {
      int32_t fi = structFieldIndex(d, sl->fieldNames[i]);
      if (fi < 0) {
        errAt(errs, sl->line, "struct '" + sl->name + "' has no field '" + sl->fieldNames[i] + "'");
        checkExpr(sl->values[i].get());
        continue;
      }
      if (d->fields[fi].isPrivate && !canTouchPrivate(sl->name)) {
        errAt(errs, sl->line, sl->line, "field '" + sl->fieldNames[i] + "' of '" + sl->name +
              "' is private", "construct the value through the struct's impl, e.g. " +
              sl->name + "::new(...)");

      }
      if (seen[fi]) errAt(errs, sl->line, "field '" + sl->fieldNames[i] + "' set more than once");
      seen[fi] = 1;
      BType vt = checkExpr(sl->values[i].get());
      BType ft = d->fields[fi].type;
      if (vt != ft && !litAssignable(sl->values[i].get(), vt, ft) &&
          !dynamic_cast<CastExpr*>(sl->values[i].get()))
        errAt(errs, sl->values[i]->line, "field '" + sl->fieldNames[i] + "' type mismatch");
    }
    if (sl->fieldNames.size() < d->fields.size()) {


      for (size_t i = 0; i < d->fields.size(); i++)
        if (!seen[i]) {
          bool priv = d->fields[i].isPrivate;
          if (priv && !canTouchPrivate(sl->name)) continue;
          errAt(errs, sl->line, "struct literal '" + sl->name + "' is missing field '" +
                d->fields[i].name + "'");
          break;
        }
    }
    return BType{BType::Tag::struct_, 0, 0, sl->name};
  }
  if (auto mc = dynamic_cast<MethodCall*>(e)) {
    BType rt = checkExpr(mc->receiver.get());

    if (rt.tag == BType::Tag::ptr && pointee(rt).tag == BType::Tag::struct_)
      rt = pointee(rt);
    if (rt.tag != BType::Tag::struct_) {
      errAt(errs, mc->line, mc->col, "method call requires a struct receiver");
      for (auto& a : mc->args) checkExpr(a.get());
      return BType::void_;
    }
    const std::string& sn = rt.structName;
    auto sit = methods.find(sn);
    if (sit == methods.end() || !sit->second.count(mc->callee)) {
      std::vector<std::string> cands;
      if (sit != methods.end()) for (auto& kv : sit->second) cands.push_back(kv.first);
      auto s = suggest(mc->callee, cands);
      errAt(errs, mc->line, mc->col,
            "struct '" + sn + "' has no method '" + mc->callee + "'",
            s.empty() ? "" : "did you mean '" + s + "'?");
      for (auto& a : mc->args) checkExpr(a.get());
      return BType::void_;
    }
    const MethodInfo& mi = sit->second.at(mc->callee);
    if (!mi.hasSelf) {
      errAt(errs, mc->line, mc->col,
            "'" + mc->callee + "' is an associated function (call it as " + sn +
            "::" + mc->callee + ", not via a receiver)");
      for (auto& a : mc->args) checkExpr(a.get());
      return mi.retType;
    }
    if (mc->args.size() != mi.paramTypes.size()) {
      errAt(errs, mc->line, mc->col, "method '" + sn + "::" + mc->callee +
            "' expects " + std::to_string(mi.paramTypes.size()) + " args, got " +
            std::to_string(mc->args.size()));
    }
    for (size_t k = 0; k < mc->args.size(); k++) {
      BType at = checkExpr(mc->args[k].get());
      BType pt = (k < mi.paramTypes.size()) ? mi.paramTypes[k] : BType::void_;
      bool ok = (at == pt) || litAssignable(mc->args[k].get(), at, pt) ||
                dynamic_cast<CastExpr*>(mc->args[k].get());
      if (!ok && pt.tag == BType::Tag::ptr && at == pointee(pt) &&
          isLvalueExpr(mc->args[k].get())) ok = true;
      if (!ok)
        errAt(errs, mc->args[k]->line, "argument " + std::to_string(k + 1) +
              " of '" + sn + "::" + mc->callee + "' has wrong type");
    }
    mc->receiverByRef = mi.selfByRef;
    mc->recvType = rt;
    return mi.retType;
  }
  if (auto ac = dynamic_cast<AssocCall*>(e)) {

    std::string sn = ac->typeName;
    if (isAliasName(sn)) sn = resolveAlias(sn).second.structName;
    if (!findStruct(sn)) {
      errAt(errs, ac->line, ac->col, "associated call on unknown type '" + ac->typeName + "'");
      for (auto& a : ac->args) checkExpr(a.get());
      return BType::void_;
    }
    auto sit = methods.find(sn);
    if (sit == methods.end() || !sit->second.count(ac->callee)) {
      std::vector<std::string> cands;
      if (sit != methods.end()) for (auto& kv : sit->second) cands.push_back(kv.first);
      auto s = suggest(ac->callee, cands);
      errAt(errs, ac->line, ac->col,
            "type '" + sn + "' has no associated function '" + ac->callee + "'",
            s.empty() ? "" : "did you mean '" + s + "'?");
      for (auto& a : ac->args) checkExpr(a.get());
      return BType::void_;
    }
    const MethodInfo& mi = sit->second.at(ac->callee);
    if (mi.hasSelf) {
      errAt(errs, ac->line, ac->col,
            "'" + ac->callee + "' is a method (call it via a receiver, not " +
            sn + "::" + ac->callee + ")");
      for (auto& a : ac->args) checkExpr(a.get());
      return mi.retType;
    }
    if (ac->args.size() != mi.paramTypes.size()) {
      errAt(errs, ac->line, ac->col, "'" + sn + "::" + ac->callee +
            "' expects " + std::to_string(mi.paramTypes.size()) + " args, got " +
            std::to_string(ac->args.size()));
    }
    for (size_t k = 0; k < ac->args.size(); k++) {
      BType at = checkExpr(ac->args[k].get());
      BType pt = (k < mi.paramTypes.size()) ? mi.paramTypes[k] : BType::void_;
      bool ok = (at == pt) || litAssignable(ac->args[k].get(), at, pt) ||
                dynamic_cast<CastExpr*>(ac->args[k].get());
      if (!ok && pt.tag == BType::Tag::ptr && at == pointee(pt) &&
          isLvalueExpr(ac->args[k].get())) ok = true;
      if (!ok)
        errAt(errs, ac->args[k]->line, "argument " + std::to_string(k + 1) +
              " of '" + sn + "::" + ac->callee + "' has wrong type");
    }
    return mi.retType;
  }
  return BType::void_;
}

void Sema::checkStmt(Stmt* s) {
  if (auto es = dynamic_cast<ExprStmt*>(s)) {
    checkExpr(es->expr.get());
    return;
  }
  if (auto ls = dynamic_cast<LetStmt*>(s)) {
    if (ls->typeAnnotated) ls->type = fixType(ls->type);
    BType t = ls->typeAnnotated ? ls->type : BType::void_;
    if (ls->init) {
      BType it = checkExpr(ls->init.get());
      if (!ls->typeAnnotated) {
        t = it;
      } else if (t != it) {
        bool ok = litAssignable(ls->init.get(), it, t) ||
                  dynamic_cast<CastExpr*>(ls->init.get());


        if (!ok && t.tag == BType::Tag::array) {
          if (auto al = dynamic_cast<ArrayLit*>(ls->init.get())) {
            bool fits = true;
            BType want = arrayElem(t);
            for (auto& el : al->elems) {
              BType et = checkExpr(el.get());
              if (et != want && !litAssignable(el.get(), et, want) &&
                  !dynamic_cast<CastExpr*>(el.get())) { fits = false; break; }
            }
            if (fits) {


              for (auto& el : al->elems) {
                if (auto* ce = dynamic_cast<CastExpr*>(el.get())) { (void)ce; continue; }
                auto cast = std::make_unique<CastExpr>();
                cast->target = want;
                cast->e = std::move(el);
                el = std::move(cast);
              }
              ok = true;
            }
          }
        }
        if (!ok) errAt(errs, ls->line, "let initializer type does not match annotation");
      }
    }
    if (t == BType::void_) {
      errAt(errs, ls->line, "cannot infer type for '" + ls->name + "'");
      t = BType::i64;
    }
    if (t.tag == BType::Tag::array && t.count <= 0 && !ls->typeAnnotated) {

    }
    ls->type = t;
    ls->typeAnnotated = true;
    declare(ls->name, t, ls->isMut);
    return;
  }
  if (auto rs = dynamic_cast<ReturnStmt*>(s)) {
    BType rt = rs->value ? checkExpr(rs->value.get()) : BType::void_;
    if (!curFunc_) return;
    if (rt != curFunc_->retType) {
      if (curFunc_->retType == BType::void_ && rs->value)
        errAt(errs, rs->line, "void function cannot return a value");
      else if (curFunc_->retType != BType::void_ && !rs->value)
        errAt(errs, rs->line, "non-void function must return a value");
      else if (!litAssignable(rs->value.get(), rt, curFunc_->retType) &&
               !dynamic_cast<CastExpr*>(rs->value.get()))
        errAt(errs, rs->line, "return type does not match function signature");
    }
    return;
  }
  if (auto is = dynamic_cast<IfStmt*>(s)) {
    BType ct = checkExpr(is->cond.get());
    if (ct != BType::bool_) errAt(errs, is->line, "if condition must be bool");
    pushScope(); checkBlock(is->then); popScope();
    pushScope(); checkBlock(is->else_); popScope();
    return;
  }
  if (auto ws = dynamic_cast<WhileStmt*>(s)) {
    BType ct = checkExpr(ws->cond.get());
    if (ct != BType::bool_) errAt(errs, ws->line, "while condition must be bool");
    loopDepth_++;
    pushScope(); checkBlock(ws->body); popScope();
    loopDepth_--;
    return;
  }
  if (auto fs = dynamic_cast<ForStmt*>(s)) {
    if (fs->isForeach) {
      BType it = checkExpr(fs->iter.get());
      BType et = BType::i64;
      if (it.tag == BType::Tag::array)       et = arrayElem(it);
      else if (it.tag == BType::Tag::dynarray) et = dynArrayElem(it);
      else if (it == BType::str)             et = BType::char_;
      else errAt(errs, fs->line, "for-in requires an array, vec, or string");
      fs->elemType = et;
      pushScope();
      declare(fs->varName, et, true);
      loopDepth_++;
      checkBlock(fs->body);
      loopDepth_--;
      popScope();
      return;
    }
    BType st = checkExpr(fs->start.get());
    pushScope();
    declare(fs->varName, BType::i64, true);
    BType ct = fs->end ? checkExpr(fs->end.get()) : BType::bool_;
    if (ct != BType::bool_) errAt(errs, fs->line, "for condition must be bool");
    if (fs->step) checkExpr(fs->step.get());
    loopDepth_++;
    checkBlock(fs->body);
    loopDepth_--;
    popScope();
    (void)st;
    return;
  }
  if (dynamic_cast<BreakStmt*>(s) || dynamic_cast<ContinueStmt*>(s)) {
    if (loopDepth_ == 0) errAt(errs, s->line, "break/continue outside a loop");
    return;
  }
  if (auto b = dynamic_cast<Block*>(s)) {
    pushScope(); checkBlock(b->stmts); popScope();
    return;
  }
}

void Sema::checkBlock(const std::vector<StmtPtr>& stmts) {
  for (auto& s : stmts) checkStmt(s.get());
}

bool Sema::check(Program& prog) {

  for (auto& ed : prog.enums) {
    EnumDef* d = registerEnum(ed->name);
    if (!d->variants.empty() && d->name != ed->name) {
      errAt(errs, ed->line, "redefinition of enum '" + ed->name + "'");
    }
    d->variants = ed->variants;

    std::set<std::string> seen;
    for (auto& v : ed->variants) {
      if (seen.count(v))
        errAt(errs, ed->line, "duplicate variant '" + v + "' in enum '" + ed->name + "'");
      seen.insert(v);
    }
  }


  for (auto& td : prog.typedefs) {
    registerAlias(td->name, fixType(td->target));
  }


  for (auto& sd : prog.structs) {
    if (sd->isGeneric) {


      registerGenericStruct(sd.get());


      continue;
    }
    StructDef* d = registerStruct(sd->name);
    if (!d->fields.empty() && d->name != sd->name) {
      errAt(errs, sd->line, "redefinition of struct '" + sd->name + "'");
    }
    d->fields.clear();
    for (auto& f : sd->fields) {


      const std::string& fname = std::get<0>(f);
      BType ft = fixType(std::get<1>(f));
      bool priv = std::get<2>(f);
      d->fields.push_back({fname, ft, 0, priv});
    }

    d->size = 0; d->align = 1;
    int32_t off = 0;
    for (auto& f : d->fields) {
      int32_t w = fieldByteWidth(f.type);
      int32_t a = fieldAlign(f.type);
      if (a > d->align) d->align = a;
      off = (off + a - 1) & ~(a - 1);
      f.offset = off;
      off += w;
    }
    off = (off + d->align - 1) & ~(d->align - 1);
    d->size = off;
  }


  for (auto& im : prog.impls) {
    const std::string& sn = im->structName;
    if (!findStruct(sn)) {
      errAt(errs, im->line, "impl for unknown struct '" + sn + "'");
      continue;
    }
    for (auto& m : im->methods) {
      m->retType = fixType(m->retType);
      for (auto& p : m->params) p.type = fixType(p.type);
      m->implStruct = sn;
      registerMethod(sn, *m);
    }
  }

  bool hasMain = false;
  for (auto& fn : prog.funcs) {
    if (fn->isGeneric) {


      registerGenericFn(fn.get());
      if (fn->name == "main" && !fn->isExtern) hasMain = true;


      fn->retType = fixType(fn->retType);
      for (auto& p : fn->params) p.type = fixType(p.type);
      continue;
    }
    if (funcs.count(fn->name)) {
      errAt(errs, fn->line, "redefinition of function '" + fn->name + "'");
    }
    FuncSig sig;
    sig.isExtern = fn->isExtern;
    fn->retType = fixType(fn->retType);
    sig.retType = fn->retType;
    for (auto& p : fn->params) { p.type = fixType(p.type); sig.paramTypes.push_back(p.type); }
    funcs[fn->name] = sig;
    if (fn->name == "main" && !fn->isExtern) hasMain = true;
  }


  pushScope();
  for (auto& vd : prog.globals) {
    GlobalInfo gi{};
    gi.isConst = vd->isConst;
    gi.isExtern = vd->isExtern;
    gi.isMut = vd->isMut;
    BType t = BType::void_;
    if (vd->typeAnnotated) {
      vd->type = fixType(vd->type);
      t = vd->type;
    }
    if (vd->isExtern) {
      if (!vd->typeAnnotated) errAt(errs, vd->line, "extern global '" + vd->name + "' needs a type");
      if (vd->init) errAt(errs, vd->line, "extern global '" + vd->name + "' may not have an initializer");
      gi.type = vd->type;
    } else if (vd->init) {
      BType it = checkExpr(vd->init.get());
      if (!vd->typeAnnotated) {
        t = it;
      } else if (t != it) {
        bool ok = litAssignable(vd->init.get(), it, t) ||
                  dynamic_cast<CastExpr*>(vd->init.get());
        if (!ok) errAt(errs, vd->line, "global '" + vd->name + "': initializer type does not match annotation");
      }
      gi.type = t;


      GlobalInfo folded{};
      folded.type = t;
      if (foldConstExpr(vd->init.get(), folded)) {
        gi.hasConstVal = true;
        gi.type = folded.type;
        gi.iVal = folded.iVal; gi.fVal = folded.fVal;
        gi.bVal = folded.bVal; gi.cVal = folded.cVal; gi.sVal = folded.sVal;
        if (gi.type != t && t != BType::void_)
          errAt(errs, vd->line, "global '" + vd->name + "': fold type mismatch");
      } else if (!vd->isConst) {


        errAt(errs, vd->line, 0, "global '" + vd->name +
              "' initializer must be a compile-time constant (move runtime init into main)",
              "use a constant expression, or a const, or set it from main");
      } else {

        errAt(errs, vd->line, 0, "const '" + vd->name +
              "' initializer is not a compile-time constant",
              "const requires a literal, folded arithmetic, or a reference to a const");
      }
    } else {
      if (!vd->typeAnnotated) errAt(errs, vd->line, "global '" + vd->name + "' has no type and no initializer");
      gi.type = t;
    }
    if (gi.type == BType::void_) {
      errAt(errs, vd->line, "cannot infer type for global '" + vd->name + "'");
      gi.type = BType::i64;
    }
    if (vd->isConst) vd->isMut = false;
    vd->type = gi.type;
    vd->typeAnnotated = true;
    if (globals.count(vd->name))
      errAt(errs, vd->line, "redefinition of global '" + vd->name + "'");
    globals[vd->name] = gi;

    declare(vd->name, gi.type, gi.isMut,true);
  }

  for (auto& fn : prog.funcs) {
    if (fn->isExtern) continue;
    curFunc_ = &funcs[fn->name];

    while (scopes_.size() > 1) scopes_.pop_back();
    pushScope();
    for (auto& p : fn->params) declare(p.name, p.type, true);
    checkBlock(fn->body);
    popScope();
    curFunc_ = nullptr;
  }


  for (auto& im : prog.impls) {
    const std::string& sn = im->structName;
    if (!findStruct(sn)) continue;
    curImpl_ = sn;
    BType st; st.tag = BType::Tag::struct_; st.structName = sn;
    for (auto& m : im->methods) {
      curFunc_ = &funcs[mangleMethod(sn, m->name)];
      while (scopes_.size() > 1) scopes_.pop_back();
      pushScope();


      declare("self", m->selfByRef ? makePtr(st) : st, true);
      for (auto& p : m->params) declare(p.name, p.type, true);
      checkBlock(m->body);
      popScope();
      curFunc_ = nullptr;
    }
    curImpl_.clear();
  }


  {
    size_t checked = 0;
    while (checked < monomorphFns.size()) {
      size_t n = monomorphFns.size();
      for (; checked < n; checked++) {
        FuncDecl& fn = *monomorphFns[checked];
        curFunc_ = &funcs[fn.name];
        while (scopes_.size() > 1) scopes_.pop_back();
        pushScope();
        for (auto& p : fn.params) declare(p.name, p.type, true);
        checkBlock(fn.body);
        popScope();
        curFunc_ = nullptr;
      }
    }
  }

  popScope();

  if (requireMain && !hasMain) errAt(errs, 1, "program must define a 'main' function");
  return errs.empty();
}


ExprPtr cloneExpr(const Expr* e) {
  if (!e) return nullptr;
  if (auto l = dynamic_cast<const IntLit*>(e))    { auto n = std::make_unique<IntLit>();   n->v=l->v; n->line=l->line; n->col=l->col; return n; }
  if (auto l = dynamic_cast<const FloatLit*>(e))  { auto n = std::make_unique<FloatLit>(); n->v=l->v; n->line=l->line; n->col=l->col; return n; }
  if (auto l = dynamic_cast<const BoolLit*>(e))   { auto n = std::make_unique<BoolLit>();   n->v=l->v; n->line=l->line; n->col=l->col; return n; }
  if (auto l = dynamic_cast<const StrLit*>(e))    { auto n = std::make_unique<StrLit>();    n->v=l->v; n->line=l->line; n->col=l->col; return n; }
  if (auto l = dynamic_cast<const CharLit*>(e))   { auto n = std::make_unique<CharLit>();   n->v=l->v; n->line=l->line; n->col=l->col; return n; }
  if (auto v = dynamic_cast<const VarRef*>(e))    { auto n = std::make_unique<VarRef>();    n->name=v->name; n->line=v->line; n->col=v->col; return n; }
  if (dynamic_cast<const NullLit*>(e))            { auto n = std::make_unique<NullLit>();   n->line=e->line; n->col=e->col; return n; }
  if (auto s = dynamic_cast<const SizeofExpr*>(e)){ auto n = std::make_unique<SizeofExpr>(); n->target=s->target; n->size=s->size; n->line=s->line; n->col=s->col; return n; }
  if (auto a = dynamic_cast<const AsmExpr*>(e)) {
    auto n = std::make_unique<AsmExpr>(); n->asmText=a->asmText; n->clobbers=a->clobbers;
    n->sideEffect=a->sideEffect; n->hasMemory=a->hasMemory; n->resultTy=a->resultTy;
    n->outputTypes=a->outputTypes;
    n->line=a->line; n->col=a->col;
    for (auto& io : a->ios) { AsmIO c; c.isOutput=io.isOutput; c.isInOut=io.isInOut; c.constraint=io.constraint; c.ty=io.ty; c.val=cloneExpr(io.val.get()); n->ios.push_back(std::move(c)); }
    return n;
  }
  if (auto c = dynamic_cast<const CastExpr*>(e)) { auto n = std::make_unique<CastExpr>();  n->e=cloneExpr(c->e.get()); n->target=c->target; n->line=c->line; n->col=c->col; return n; }
  if (auto u = dynamic_cast<const UnaryExpr*>(e)){ auto n = std::make_unique<UnaryExpr>(); n->op=u->op; n->base=cloneExpr(u->base.get()); n->methodOverload=u->methodOverload; n->overloadStruct=u->overloadStruct; n->overloadMethod=u->overloadMethod; n->overloadRecvType=u->overloadRecvType; n->recvByRef=u->recvByRef; n->line=u->line; n->col=u->col; return n; }
  if (auto b = dynamic_cast<const BinaryExpr*>(e)){ auto n = std::make_unique<BinaryExpr>(); n->op=b->op; n->lhs=cloneExpr(b->lhs.get()); n->rhs=cloneExpr(b->rhs.get()); n->methodOverload=b->methodOverload; n->overloadStruct=b->overloadStruct; n->overloadMethod=b->overloadMethod; n->overloadRecvType=b->overloadRecvType; n->recvByRef=b->recvByRef; n->isPtrArith=b->isPtrArith; n->ptrArithPointee=b->ptrArithPointee; n->line=b->line; n->col=b->col; return n; }
  if (auto a = dynamic_cast<const AssignTarget*>(e)) {
    auto n = std::make_unique<AssignTarget>(); n->kind=a->kind; n->name=a->name; n->base=cloneExpr(a->base.get()); n->index=cloneExpr(a->index.get()); n->field=a->field;
    n->value=cloneExpr(a->value.get()); n->compound=a->compound; n->isCompound=a->isCompound; n->methodOverload=a->methodOverload; n->overloadStruct=a->overloadStruct; n->overloadMethod=a->overloadMethod; n->overloadRecvType=a->overloadRecvType; n->recvByRef=a->recvByRef; n->line=a->line; n->col=a->col; return n;
  }
  if (auto c = dynamic_cast<const Call*>(e)) { auto n = std::make_unique<Call>(); n->callee=c->callee; n->isPrint=c->isPrint; n->typeArgs=c->typeArgs; n->hasTypeArgs=c->hasTypeArgs; n->fnPtr=c->fnPtr; n->calleeExpr=cloneExpr(c->calleeExpr.get()); n->line=c->line; n->col=c->col; for (auto& a : c->args) n->args.push_back(cloneExpr(a.get())); return n; }
  if (auto mc = dynamic_cast<const MethodCall*>(e)) { auto n = std::make_unique<MethodCall>(); n->callee=mc->callee; n->receiver=cloneExpr(mc->receiver.get()); n->receiverByRef=mc->receiverByRef; n->recvType=mc->recvType; n->line=mc->line; n->col=mc->col; for (auto& a : mc->args) n->args.push_back(cloneExpr(a.get())); return n; }
  if (auto ac = dynamic_cast<const AssocCall*>(e)) { auto n = std::make_unique<AssocCall>(); n->typeName=ac->typeName; n->callee=ac->callee; n->line=ac->line; n->col=ac->col; for (auto& a : ac->args) n->args.push_back(cloneExpr(a.get())); return n; }
  if (auto ix = dynamic_cast<const Index*>(e)) { auto n = std::make_unique<Index>(); n->base=cloneExpr(ix->base.get()); n->index=cloneExpr(ix->index.get()); n->methodOverload=ix->methodOverload; n->overloadStruct=ix->overloadStruct; n->overloadMethod=ix->overloadMethod; n->overloadRecvType=ix->overloadRecvType; n->recvByRef=ix->recvByRef; n->line=ix->line; n->col=ix->col; return n; }
  if (auto fl = dynamic_cast<const Field*>(e)) { auto n = std::make_unique<Field>(); n->base=cloneExpr(fl->base.get()); n->field=fl->field; n->line=fl->line; n->col=fl->col; return n; }
  if (auto al = dynamic_cast<const ArrayLit*>(e)) { auto n = std::make_unique<ArrayLit>(); n->line=al->line; n->col=al->col; for (auto& el : al->elems) n->elems.push_back(cloneExpr(el.get())); return n; }
  if (auto sl = dynamic_cast<const StructLit*>(e)) { auto n = std::make_unique<StructLit>(); n->name=sl->name; n->fieldNames=sl->fieldNames; n->typeArgs=sl->typeArgs; n->hasTypeArgs=sl->hasTypeArgs; n->line=sl->line; n->col=sl->col; for (auto& v : sl->values) n->values.push_back(cloneExpr(v.get())); return n; }
  if (auto dn = dynamic_cast<const DynNew*>(e)) { auto n = std::make_unique<DynNew>(); n->elemType=dn->elemType; n->line=dn->line; n->col=dn->col; return n; }
  if (auto mn = dynamic_cast<const MapNew*>(e)) { auto n = std::make_unique<MapNew>(); n->keyType=mn->keyType; n->valType=mn->valType; n->line=mn->line; n->col=mn->col; return n; }
  if (auto sn = dynamic_cast<const SetNew*>(e)) { auto n = std::make_unique<SetNew>(); n->elemType=sn->elemType; n->line=sn->line; n->col=sn->col; return n; }
  if (auto hm = dynamic_cast<const HMapNew*>(e)) { auto n = std::make_unique<HMapNew>(); n->keyType=hm->keyType; n->valType=hm->valType; n->line=hm->line; n->col=hm->col; return n; }
  if (auto hs = dynamic_cast<const HSetNew*>(e)) { auto n = std::make_unique<HSetNew>(); n->elemType=hs->elemType; n->line=hs->line; n->col=hs->col; return n; }
  if (auto lam = dynamic_cast<const LambdaLit*>(e)) { auto n = std::make_unique<LambdaLit>(); n->params=lam->params; n->retType=lam->retType; n->loweredName=lam->loweredName; n->fnType=lam->fnType; for (auto& s : lam->body) n->body.push_back(cloneStmt(s.get())); n->line=lam->line; n->col=lam->col; return n; }
  if (auto t = dynamic_cast<const TernaryExpr*>(e)) {
    auto n = std::make_unique<TernaryExpr>(); n->cond=cloneExpr(t->cond.get());
    n->thenE=cloneExpr(t->thenE.get()); n->elseE=cloneExpr(t->elseE.get());
    n->resultTy=t->resultTy; n->line=t->line; n->col=t->col; return n;
  }
  if (auto d = dynamic_cast<const IncDecExpr*>(e)) {
    auto n = std::make_unique<IncDecExpr>(); n->isInc=d->isInc; n->isPost=d->isPost;
    n->kind=d->kind; n->name=d->name; n->base=cloneExpr(d->base.get());
    n->index=cloneExpr(d->index.get()); n->field=d->field; n->valueTy=d->valueTy;
    n->line=d->line; n->col=d->col; return n;
  }
  return nullptr;
}

StmtPtr cloneStmt(const Stmt* s) {
  if (!s) return nullptr;
  if (auto es = dynamic_cast<const ExprStmt*>(s)) { auto n = std::make_unique<ExprStmt>(); n->expr=cloneExpr(es->expr.get()); n->line=es->line; n->col=es->col; return n; }
  if (auto ls = dynamic_cast<const LetStmt*>(s)) { auto n = std::make_unique<LetStmt>(); n->isMut=ls->isMut; n->name=ls->name; n->type=ls->type; n->typeAnnotated=ls->typeAnnotated; n->init=cloneExpr(ls->init.get()); n->line=ls->line; n->col=ls->col; return n; }
  if (auto rs = dynamic_cast<const ReturnStmt*>(s)) { auto n = std::make_unique<ReturnStmt>(); n->value=cloneExpr(rs->value.get()); n->line=rs->line; n->col=rs->col; return n; }
  if (auto is = dynamic_cast<const IfStmt*>(s)) { auto n = std::make_unique<IfStmt>(); n->cond=cloneExpr(is->cond.get()); for (auto& t : is->then) n->then.push_back(cloneStmt(t.get())); for (auto& e : is->else_) n->else_.push_back(cloneStmt(e.get())); n->line=is->line; n->col=is->col; return n; }
  if (auto ws = dynamic_cast<const WhileStmt*>(s)) { auto n = std::make_unique<WhileStmt>(); n->cond=cloneExpr(ws->cond.get()); for (auto& b : ws->body) n->body.push_back(cloneStmt(b.get())); n->line=ws->line; n->col=ws->col; return n; }
  if (auto fs = dynamic_cast<const ForStmt*>(s)) { auto n = std::make_unique<ForStmt>(); n->varName=fs->varName; n->start=cloneExpr(fs->start.get()); n->end=cloneExpr(fs->end.get()); n->step=cloneExpr(fs->step.get()); n->inclusiveEnd=fs->inclusiveEnd; n->isForeach=fs->isForeach; n->iter=cloneExpr(fs->iter.get()); n->elemType=fs->elemType; for (auto& b : fs->body) n->body.push_back(cloneStmt(b.get())); n->line=fs->line; n->col=fs->col; return n; }
  if (dynamic_cast<const BreakStmt*>(s)) { auto n = std::make_unique<BreakStmt>(); n->line=s->line; n->col=s->col; return n; }
  if (dynamic_cast<const ContinueStmt*>(s)) { auto n = std::make_unique<ContinueStmt>(); n->line=s->line; n->col=s->col; return n; }
  if (auto b = dynamic_cast<const Block*>(s)) { auto n = std::make_unique<Block>(); n->line=b->line; n->col=b->col; for (auto& st : b->stmts) n->stmts.push_back(cloneStmt(st.get())); return n; }
  return nullptr;
}
