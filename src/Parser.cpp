#include "Parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> toks, std::vector<ParseError>& errs)
  : toks_(std::move(toks)), errs_(&errs) {}

const Token& Parser::cur() { return toks_[p_]; }
const Token& Parser::peek(size_t off) { return toks_[p_ + off]; }
bool Parser::at(Tok t) { return cur().kind == t; }
bool Parser::accept(Tok t) {
  if (cur().kind == t) { p_++; return true; }
  return false;
}
bool Parser::expect(Tok t, const std::string& what) {
  if (accept(t)) return true;
  error("expected " + what + " but got '" + cur().text + "'");
  return false;
}
void Parser::error(const std::string& msg) {
  errs_->push_back({msg, cur().line, cur().col});
}
bool Parser::atTopLevel() {
  return at(Tok::kw_fn) || at(Tok::kw_struct) || at(Tok::kw_enum) ||
         at(Tok::kw_typedef) || at(Tok::kw_extern) || at(Tok::kw_impl) ||
         at(Tok::kw_let) || at(Tok::kw_const) || at(Tok::end);
}

BType Parser::parseType() {


  if (at(Tok::star)) { p_++; BType inner = parseType(); return makePtr(inner); }
  if (at(Tok::amp))  { p_++; BType inner = parseType(); return makePtr(inner); }
  if (at(Tok::kw_i64)) { p_++; return BType::i64; }
  if (at(Tok::kw_f64)) { p_++; return BType::f64; }
  if (at(Tok::kw_f32)) { p_++; return BType::f32; }
  if (at(Tok::kw_bool)) { p_++; return BType::bool_; }
  if (at(Tok::kw_void)) { p_++; return BType::void_; }
  if (at(Tok::kw_str)) { p_++; return BType::str; }
  if (at(Tok::kw_char)) { p_++; return BType::char_; }
  if (at(Tok::kw_i8)) { p_++; return BType::i8; }
  if (at(Tok::kw_i16)) { p_++; return BType::i16; }
  if (at(Tok::kw_i32)) { p_++; return BType::i32; }
  if (at(Tok::kw_u8)) { p_++; return BType::u8; }
  if (at(Tok::kw_u16)) { p_++; return BType::u16; }
  if (at(Tok::kw_u32)) { p_++; return BType::u32; }
  if (at(Tok::kw_u64)) { p_++; return BType::u64; }
  if (at(Tok::kw_usize)) { p_++; return BType::usize; }
  if (at(Tok::kw_vec)) {
    p_++;
    if (!expect(Tok::lbracket, "'[' after 'vec'")) { return makeDynArray(BType::i64); }
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    return makeDynArray(elem);
  }
  if (at(Tok::kw_map)) {
    p_++;
    if (!expect(Tok::lbracket, "'[' after 'map'")) { return makeMapType(BType::i64, BType::i64); }
    BType key = parseType();
    expect(Tok::comma, "',' between key and value in map[K, V]");
    BType val = parseType();
    expect(Tok::rbracket, "']'");
    return makeMapType(key, val);
  }
  if (at(Tok::kw_set)) {
    p_++;
    if (!expect(Tok::lbracket, "'[' after 'set'")) { return makeSetType(BType::i64); }
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    return makeSetType(elem);
  }
  if (at(Tok::kw_hmap)) {
    p_++;
    if (!expect(Tok::lbracket, "'[' after 'hmap'")) { return makeHMapType(BType::i64, BType::i64); }
    BType key = parseType();
    expect(Tok::comma, "',' between key and value in hmap[K, V]");
    BType val = parseType();
    expect(Tok::rbracket, "']'");
    return makeHMapType(key, val);
  }
  if (at(Tok::kw_hset)) {
    p_++;
    if (!expect(Tok::lbracket, "'[' after 'hset'")) { return makeHSetType(BType::i64); }
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    return makeHSetType(elem);
  }
  if (at(Tok::lbracket)) {
    p_++;
    BType elem = parseType();
    if (!expect(Tok::semicolon, "';' in array type")) return makeArrayType(elem, 0);
    int count = 0;
    if (at(Tok::int_lit)) { count = (int)cur().u64; p_++; }
    else error("expected array length");
    expect(Tok::rbracket, "']'");
    return makeArrayType(elem, count);
  }
  if (at(Tok::ident)) {

    std::string name = cur().text;
    p_++;


    if (at(Tok::lt) && peek(1).kind != Tok::lt) {
      bool ok = false;
      std::vector<BType> args = tryParseTypeArgs(ok);
      if (ok) return makeGenericInst(name, args,false);
    }
    BType t; t.tag = BType::Tag::struct_; t.structName = name;
    return t;
  }
  error("expected a type but got '" + cur().text + "'");
  p_++;
  return BType::i64;
}

std::unique_ptr<Program> Parser::parseProgram() {
  auto prog = std::make_unique<Program>();


  auto recoverToTopLevel = [&] {
    while (!atTopLevel()) p_++;
  };
  while (!at(Tok::end)) {
    if (at(Tok::kw_struct)) {
      auto s = parseStruct();
      if (s) prog->structs.push_back(std::move(s));
      else recoverToTopLevel();
      continue;
    }
    if (at(Tok::kw_enum)) {
      auto e = parseEnum();
      if (e) prog->enums.push_back(std::move(e));
      else recoverToTopLevel();
      continue;
    }
    if (at(Tok::kw_impl)) {
      auto im = parseImpl();
      if (im) prog->impls.push_back(std::move(im));
      else recoverToTopLevel();
      continue;
    }
    if (at(Tok::kw_typedef)) {
      auto td = parseTypedef();
      if (td) prog->typedefs.push_back(std::move(td));
      else { while (!at(Tok::semicolon) && !at(Tok::end)) p_++; accept(Tok::semicolon); }
      continue;
    }
    if (at(Tok::kw_extern)) {

      p_++;
      if (at(Tok::kw_fn)) {
        auto fn = parseFunc(true);
        if (fn) prog->funcs.push_back(std::move(fn));
        else recoverToTopLevel();
      } else if (at(Tok::kw_let) || at(Tok::kw_const)) {
        auto vd = parseGlobal(true);
        if (vd) prog->globals.push_back(std::move(vd));
      } else {
        error("expected 'fn' or 'let' after 'extern'");
        p_++;
      }
      continue;
    }
    if (at(Tok::kw_let) || at(Tok::kw_const)) {
      auto vd = parseGlobal(false);
      if (vd) prog->globals.push_back(std::move(vd));
      else { while (!at(Tok::semicolon) && !at(Tok::end)) p_++; accept(Tok::semicolon); }
      continue;
    }
    auto f = parseFunc();
    if (f) prog->funcs.push_back(std::move(f));
    else {

      recoverToTopLevel();
    }
  }
  return prog;
}

std::unique_ptr<EnumDecl> Parser::parseEnum() {
  p_++;
  auto ed = std::make_unique<EnumDecl>();
  ed->line = cur().line;
  if (!expect(Tok::ident, "enum name")) return nullptr;
  ed->name = toks_[p_ - 1].text;
  if (!expect(Tok::lbrace, "'{' to begin enum body")) return nullptr;
  while (!at(Tok::rbrace) && !at(Tok::end)) {
    if (!expect(Tok::ident, "variant name")) break;
    ed->variants.push_back(toks_[p_ - 1].text);
    if (!accept(Tok::comma)) break;
  }
  expect(Tok::rbrace, "'}'");
  return ed;
}


std::unique_ptr<ImplDecl> Parser::parseImpl() {
  p_++;
  auto im = std::make_unique<ImplDecl>();
  im->line = cur().line;
  if (!expect(Tok::ident, "struct name after 'impl'")) return nullptr;
  im->structName = toks_[p_ - 1].text;
  if (!expect(Tok::lbrace, "'{' to begin impl body")) return nullptr;
  while (!at(Tok::rbrace) && !at(Tok::end)) {
    if (!at(Tok::kw_fn)) {
      error("expected 'fn' for a method in impl block");

      while (!at(Tok::kw_fn) && !at(Tok::rbrace) && !at(Tok::end)) p_++;
      if (at(Tok::kw_fn)) { continue; }
      break;
    }
    auto fn = parseFunc(false);
    if (fn) im->methods.push_back(std::move(fn));
    else {
      while (!at(Tok::kw_fn) && !at(Tok::rbrace) && !at(Tok::end)) p_++;
    }
  }
  expect(Tok::rbrace, "'}' to close impl body");
  return im;
}

std::unique_ptr<StructDecl> Parser::parseStruct() {
  p_++;
  auto sd = std::make_unique<StructDecl>();
  sd->line = cur().line;
  if (!expect(Tok::ident, "struct name")) return nullptr;
  sd->name = toks_[p_ - 1].text;
  sd->typeParams = parseTypeParams();
  sd->isGeneric = !sd->typeParams.empty();
  if (!expect(Tok::lbrace, "'{' to begin struct body")) return nullptr;
  while (!at(Tok::rbrace) && !at(Tok::end)) {
    bool priv = accept(Tok::kw_private);
    if (!expect(Tok::ident, "field name")) break;
    std::string fname = toks_[p_ - 1].text;
    if (!expect(Tok::colon, "':' after field name")) break;
    BType ft = parseType();
    sd->fields.push_back({fname, ft, priv});
    accept(Tok::semicolon);
    accept(Tok::comma);
  }
  expect(Tok::rbrace, "'}'");
  return sd;
}

std::vector<std::string> Parser::parseTypeParams() {
  std::vector<std::string> out;


  if (!at(Tok::lt)) return out;
  if (peek(1).kind != Tok::ident) return out;
  size_t save = p_;
  p_++;
  while (!at(Tok::gt) && !at(Tok::end)) {
    if (!at(Tok::ident)) { error("expected type-parameter name"); break; }
    out.push_back(cur().text);
    p_++;
    if (accept(Tok::comma)) continue;
    break;
  }
  if (!accept(Tok::gt)) {

    p_ = save;
    out.clear();
  }
  return out;
}

std::vector<BType> Parser::tryParseTypeArgs(bool& ok) {
  ok = false;
  std::vector<BType> out;
  if (!at(Tok::lt)) return out;


  size_t save = p_;
  p_++;


  size_t errCount = errs_->size();
  while (!at(Tok::gt) && !at(Tok::end)) {
    out.push_back(parseType());
    if (!accept(Tok::comma)) break;
  }
  if (at(Tok::gt)) {


    errs_->resize(errCount);
    p_++;
    ok = true;
    return out;
  }


  p_ = save;
  if (errs_->size() > errCount) errs_->resize(errCount);
  out.clear();
  return out;
}

std::unique_ptr<FuncDecl> Parser::parseFunc(bool isExtern) {
  if (!at(Tok::kw_fn)) {
    error("expected 'fn'");
    return nullptr;
  }
  p_++;
  auto fn = std::make_unique<FuncDecl>();
  fn->line = cur().line;
  fn->isExtern = isExtern;
  if (!expect(Tok::ident, "function name")) return nullptr;
  fn->name = toks_[p_ - 1].text;
  fn->typeParams = parseTypeParams();
  fn->isGeneric = !fn->typeParams.empty();
  if (!expect(Tok::lparen, "'('")) return nullptr;


  if (at(Tok::kw_self)) {
    p_++;
    fn->hasSelf = true;
    fn->selfByRef = false;
    if (!accept(Tok::comma)) {
      expect(Tok::rparen, "')'");
      if (accept(Tok::arrow)) fn->retType = parseType();
      else fn->retType = BType::void_;
      if (isExtern) { expect(Tok::semicolon, "';' after extern fn declaration"); return fn; }
      if (!at(Tok::lbrace)) { error("expected '{' to begin function body"); return nullptr; }
      fn->body = parseBlock();
      return fn;
    }
  } else if (at(Tok::amp) && peek(1).kind == Tok::kw_self) {
    p_ += 2;
    fn->hasSelf = true;
    fn->selfByRef = true;
    if (!accept(Tok::comma)) {
      expect(Tok::rparen, "')'");
      if (accept(Tok::arrow)) fn->retType = parseType();
      else fn->retType = BType::void_;
      if (isExtern) { expect(Tok::semicolon, "';' after extern fn declaration"); return fn; }
      if (!at(Tok::lbrace)) { error("expected '{' to begin function body"); return nullptr; }
      fn->body = parseBlock();
      return fn;
    }
  }
  while (!at(Tok::rparen) && !at(Tok::end)) {
    if (!expect(Tok::ident, "parameter name")) break;
    std::string pname = toks_[p_ - 1].text;
    if (!expect(Tok::colon, "':' after parameter name")) break;
    BType pt = parseType();
    fn->params.push_back({pname, pt});
    if (!accept(Tok::comma)) break;
  }
  expect(Tok::rparen, "')'");
  if (accept(Tok::arrow)) {
    fn->retType = parseType();
  } else {
    fn->retType = BType::void_;
  }
  if (fn->isExtern) {


    expect(Tok::semicolon, "';' after extern fn declaration");
    return fn;
  }
  if (!at(Tok::lbrace)) {
    error("expected '{' to begin function body");
    return nullptr;
  }
  fn->body = parseBlock();
  return fn;
}


std::unique_ptr<TypedefDecl> Parser::parseTypedef() {
  p_++;
  auto td = std::make_unique<TypedefDecl>();
  td->line = cur().line;
  if (!expect(Tok::ident, "typedef name")) return nullptr;
  td->name = toks_[p_ - 1].text;
  if (!expect(Tok::eq, "'=' in typedef")) return nullptr;
  td->target = parseType();
  expect(Tok::semicolon, "';' after typedef");
  return td;
}


std::unique_ptr<VarDecl> Parser::parseGlobal(bool isExtern) {
  auto vd = std::make_unique<VarDecl>();
  vd->isExtern = isExtern;
  vd->line = cur().line;
  if (accept(Tok::kw_const)) {
    vd->isConst = true;
  } else {
    p_++;
    if (accept(Tok::kw_mut)) vd->isMut = true;
  }
  if (!expect(Tok::ident, "global name")) return nullptr;
  vd->name = toks_[p_ - 1].text;
  if (accept(Tok::colon)) {
    vd->type = parseType();
    vd->typeAnnotated = true;
  }
  if (accept(Tok::eq)) {
    if (!at(Tok::semicolon)) vd->init = parseExpr();
  }
  expect(Tok::semicolon, "';' after global declaration");
  return vd;
}

std::vector<StmtPtr> Parser::parseBlock() {
  std::vector<StmtPtr> out;
  expect(Tok::lbrace, "'{'");
  while (!at(Tok::rbrace) && !at(Tok::end)) {
    auto s = parseStmt();
    if (s) out.push_back(std::move(s));
    else {

      while (!at(Tok::semicolon) && !at(Tok::rbrace) && !at(Tok::end)) p_++;
      accept(Tok::semicolon);
    }
  }
  expect(Tok::rbrace, "'}'");
  return out;
}

StmtPtr Parser::parseStmt() {
  Tok k = cur().kind;
  if (k == Tok::kw_let) { p_++; return parseLetMut(false); }
  if (k == Tok::kw_mut) { p_++; return parseLetMut(true); }
  if (k == Tok::kw_return) return parseReturn();
  if (k == Tok::kw_if) return parseIf();
  if (k == Tok::kw_while) return parseWhile();
  if (k == Tok::kw_for) return parseFor();
  if (k == Tok::kw_match) return parseMatch();
  if (k == Tok::kw_break) {
    int line = cur().line;
    p_++;
    accept(Tok::semicolon);
    auto b = std::make_unique<BreakStmt>();
    b->line = line;
    return b;
  }
  if (k == Tok::kw_continue) {
    int line = cur().line;
    p_++;
    accept(Tok::semicolon);
    auto c = std::make_unique<ContinueStmt>();
    c->line = line;
    return c;
  }
  if (at(Tok::lbrace)) {
    auto blk = std::make_unique<Block>();
    blk->stmts = parseBlock();
    return blk;
  }
  auto es = std::make_unique<ExprStmt>();
  es->line = cur().line;
  es->expr = parseExpr();
  accept(Tok::semicolon);
  return es;
}

StmtPtr Parser::parseLetMut(bool isMut) {
  auto ls = std::make_unique<LetStmt>();
  if (accept(Tok::kw_mut)) isMut = true;
  ls->isMut = isMut;
  ls->line = cur().line;
  if (!expect(Tok::ident, "variable name")) { return nullptr; }
  ls->name = toks_[p_ - 1].text;
  if (accept(Tok::colon)) {
    ls->type = parseType();
    ls->typeAnnotated = true;
  }
  if (accept(Tok::eq)) {
    if (!at(Tok::semicolon)) ls->init = parseExpr();
  }
  expect(Tok::semicolon, "';' after variable");
  return ls;
}

StmtPtr Parser::parseReturn() {
  int line = cur().line;
  p_++;
  auto rs = std::make_unique<ReturnStmt>();
  rs->line = line;
  if (!at(Tok::semicolon)) {
    rs->value = parseExpr();
  }
  expect(Tok::semicolon, "';' after return");
  return rs;
}

static void copyStmtPos(Stmt* s, const Expr& e) { s->line = e.line; s->col = e.col; }

StmtPtr Parser::parseIf() {
  p_++;
  auto is = std::make_unique<IfStmt>();
  is->line = cur().line;
  bool hasParen = accept(Tok::lparen);
  bool saved = allowStructLit_; allowStructLit_ = false;
  is->cond = parseExpr();
  allowStructLit_ = saved;
  if (hasParen) expect(Tok::rparen, "')'");
  if (at(Tok::lbrace)) {
    is->then = parseBlock();
  } else {
    auto s = parseStmt();
    if (s) is->then.push_back(std::move(s));
  }
  if (accept(Tok::kw_else)) {
    if (at(Tok::kw_if)) {
      auto nested = parseIf();
      if (nested) is->else_.push_back(std::move(nested));
    } else if (at(Tok::lbrace)) {
      is->else_ = parseBlock();
    } else {
      auto s = parseStmt();
      if (s) is->else_.push_back(std::move(s));
    }
  }
  return is;
}

StmtPtr Parser::parseWhile() {
  p_++;
  auto ws = std::make_unique<WhileStmt>();
  ws->line = cur().line;
  bool hasParen = accept(Tok::lparen);
  bool saved = allowStructLit_; allowStructLit_ = false;
  ws->cond = parseExpr();
  allowStructLit_ = saved;
  if (hasParen) expect(Tok::rparen, "')'");
  if (at(Tok::lbrace)) ws->body = parseBlock();
  else { auto s = parseStmt(); if (s) ws->body.push_back(std::move(s)); }
  return ws;
}


StmtPtr Parser::parseMatch() {
  p_++;
  int matchLine = cur().line;


  bool saved = allowStructLit_; allowStructLit_ = false;
  ExprPtr scrutinee = parseExpr();
  allowStructLit_ = saved;
  if (!expect(Tok::lbrace, "'{' to begin match body")) return nullptr;


  std::vector<std::pair<ExprPtr, std::vector<StmtPtr>>> arms;
  while (!at(Tok::rbrace) && !at(Tok::end)) {
    bool isDefault = false;
    if (at(Tok::ident) && cur().text == "_") { isDefault = true; p_++; }
    if (!isDefault) {
      saved = allowStructLit_; allowStructLit_ = false;
      ExprPtr pat = parseExpr();
      allowStructLit_ = saved;
      if (!expect(Tok::fatarrow, "'=>' in match arm")) break;
      std::vector<StmtPtr> body;
      if (at(Tok::lbrace)) body = parseBlock();
      else { auto s = parseStmt(); if (s) body.push_back(std::move(s)); }
      arms.emplace_back(std::move(pat), std::move(body));
      accept(Tok::comma);
      continue;
    }
    if (!expect(Tok::fatarrow, "'=>' in match arm")) break;
    std::vector<StmtPtr> body;
    if (at(Tok::lbrace)) body = parseBlock();
    else { auto s = parseStmt(); if (s) body.push_back(std::move(s)); }
    arms.emplace_back(nullptr, std::move(body));
    accept(Tok::comma);
  }
  expect(Tok::rbrace, "'}'");


  const std::string synth = "__match_scrut";
  auto let = std::make_unique<LetStmt>();
  let->isMut = false; let->name = synth;
  let->type = BType::void_;
  let->typeAnnotated = false;
  let->init = std::move(scrutinee);
  let->line = matchLine;


  StmtPtr chain;
  IfStmt* live = nullptr;
  std::vector<StmtPtr>* fill = nullptr;
  for (size_t i = 0; i < arms.size(); i++) {
    auto& arm = arms[i];
    auto isStmt = std::make_unique<IfStmt>();
    isStmt->line = matchLine;
    bool isDefault = (arm.first == nullptr);
    if (arm.first) {
      auto lhsVar = std::make_unique<VarRef>();
      lhsVar->name = synth; lhsVar->line = matchLine;
      auto cmp = std::make_unique<BinaryExpr>();
      cmp->op = BinaryExpr::Op::eq;
      cmp->line = matchLine;
      cmp->lhs = std::move(lhsVar);
      cmp->rhs = std::move(arm.first);
      isStmt->cond = std::move(cmp);
    } else {

      auto t = std::make_unique<BoolLit>(); t->v = true; t->line = matchLine;
      isStmt->cond = std::move(t);
    }
    isStmt->then = std::move(arm.second);

    if (!chain) {
      live = isStmt.get();
      chain = std::move(isStmt);
    } else {
      fill->push_back(std::move(isStmt));
      live = dynamic_cast<IfStmt*>(fill->back().get());
    }

    fill = isDefault ? nullptr : &live->else_;
  }

  auto blk = std::make_unique<Block>();
  blk->line = matchLine;
  blk->stmts.push_back(std::move(let));
  if (chain) blk->stmts.push_back(std::move(chain));
  return blk;
}

StmtPtr Parser::parseFor() {
  p_++;
  auto fs = std::make_unique<ForStmt>();
  fs->line = cur().line;
  bool hasParen = accept(Tok::lparen);
  accept(Tok::kw_let);
  accept(Tok::kw_mut);
  if (!expect(Tok::ident, "loop variable")) return nullptr;
  fs->varName = toks_[p_ - 1].text;

  if (at(Tok::kw_in)) {
    p_++;
    fs->isForeach = true;
    bool saved = allowStructLit_; allowStructLit_ = false;
    fs->iter = parseExpr();
    allowStructLit_ = saved;
    if (hasParen) expect(Tok::rparen, "')'");
    if (at(Tok::lbrace)) fs->body = parseBlock();
    else { auto s = parseStmt(); if (s) fs->body.push_back(std::move(s)); }
    return fs;
  }
  expect(Tok::eq, "'=' in for header");
  fs->start = parseExpr();
  expect(Tok::semicolon, "';'");
  bool saved = allowStructLit_; allowStructLit_ = false;
  fs->end = parseExpr();
  allowStructLit_ = saved;
  expect(Tok::semicolon, "';'");
  fs->step = parseExpr();
  if (hasParen) expect(Tok::rparen, "')'");
  if (at(Tok::lbrace)) fs->body = parseBlock();
  else { auto s = parseStmt(); if (s) fs->body.push_back(std::move(s)); }
  return fs;
}

ExprPtr Parser::parseExpr() { return parseAssign(); }


ExprPtr Parser::parseTernary() {
  auto cond = parseLogicOr();
  if (at(Tok::question)) {
    p_++;
    auto thenE = parseAssign();
    expect(Tok::colon, "expected ':' in ternary expression");
    auto elseE = parseAssign();
    auto t = std::make_unique<TernaryExpr>();
    t->cond = std::move(cond);
    t->thenE = std::move(thenE);
    t->elseE = std::move(elseE);
    return t;
  }
  return cond;
}

ExprPtr Parser::parseAssign() {
  auto lhs = parseTernary();
  if (at(Tok::eq) || at(Tok::pluseq) || at(Tok::minuseq) || at(Tok::stareq) ||
      at(Tok::lasheq) || at(Tok::percenteq) || at(Tok::ampeq) || at(Tok::bareq) ||
      at(Tok::careteq) || at(Tok::shleq) || at(Tok::shreq)) {
    Tok op = cur().kind;
    p_++;
    auto rhs = parseAssign();
    auto a = std::make_unique<AssignTarget>();
    if (auto v = dynamic_cast<VarRef*>(lhs.get())) {
      a->kind = AssignTarget::Kind::var;
      a->name = v->name;
      a->line = v->line;
    } else if (auto ix = dynamic_cast<Index*>(lhs.get())) {
      a->kind = AssignTarget::Kind::index;
      a->base = std::move(ix->base);
      a->index = std::move(ix->index);
      a->line = lhs->line; a->col = lhs->col;
    } else if (auto fl = dynamic_cast<Field*>(lhs.get())) {
      a->kind = AssignTarget::Kind::field;
      a->base = std::move(fl->base);
      a->field = fl->field;
      a->line = lhs->line; a->col = lhs->col;
    } else if (auto u = dynamic_cast<UnaryExpr*>(lhs.get())) {
      if (u->op != UnaryExpr::Op::deref) { error("invalid assignment target"); return lhs; }
      a->kind = AssignTarget::Kind::deref;
      a->base = std::move(u->base);
      a->line = lhs->line; a->col = lhs->col;
    } else {
      error("invalid assignment target");
      return lhs;
    }
    a->value = std::move(rhs);
    if (op != Tok::eq) {
      a->isCompound = true;
      switch (op) {
        case Tok::pluseq: a->compound = BinaryExpr::Op::add; break;
        case Tok::minuseq: a->compound = BinaryExpr::Op::sub; break;
        case Tok::stareq: a->compound = BinaryExpr::Op::mul; break;
        case Tok::lasheq: a->compound = BinaryExpr::Op::div; break;
        case Tok::percenteq: a->compound = BinaryExpr::Op::mod; break;
        case Tok::ampeq: a->compound = BinaryExpr::Op::band; break;
        case Tok::bareq: a->compound = BinaryExpr::Op::bor; break;
        case Tok::careteq: a->compound = BinaryExpr::Op::bxor; break;
        case Tok::shleq: a->compound = BinaryExpr::Op::shl; break;
        case Tok::shreq: a->compound = BinaryExpr::Op::shr; break;
        default: break;
      }
    }
    return a;
  }
  return lhs;
}

ExprPtr Parser::parseLogicOr() {
  auto lhs = parseLogicAnd();
  while (at(Tok::barbar)) {
    p_++;
    auto rhs = parseLogicAnd();
    auto b = std::make_unique<BinaryExpr>();
    b->op = BinaryExpr::Op::lor; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseLogicAnd() {
  auto lhs = parseBitOr();
  while (at(Tok::ampamp)) {
    p_++;
    auto rhs = parseBitOr();
    auto b = std::make_unique<BinaryExpr>();
    b->op = BinaryExpr::Op::land; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseBitOr() {
  auto lhs = parseBitXor();
  while (at(Tok::bar)) {
    p_++;
    auto rhs = parseBitXor();
    auto b = std::make_unique<BinaryExpr>();
    b->op = BinaryExpr::Op::bor; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseBitXor() {
  auto lhs = parseBitAnd();
  while (at(Tok::caret)) {
    p_++;
    auto rhs = parseBitAnd();
    auto b = std::make_unique<BinaryExpr>();
    b->op = BinaryExpr::Op::bxor; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseBitAnd() {
  auto lhs = parseEquality();
  while (at(Tok::amp)) {
    p_++;
    auto rhs = parseEquality();
    auto b = std::make_unique<BinaryExpr>();
    b->op = BinaryExpr::Op::band; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseEquality() {
  auto lhs = parseRel();
  while (at(Tok::eqeq) || at(Tok::bangeq)) {
    Tok t = cur().kind; p_++;
    auto rhs = parseRel();
    auto b = std::make_unique<BinaryExpr>();
    b->op = (t == Tok::eqeq) ? BinaryExpr::Op::eq : BinaryExpr::Op::ne;
    b->line = lhs->line; b->col = lhs->col; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseRel() {
  auto lhs = parseShift();
  while (at(Tok::lt) || at(Tok::gt) || at(Tok::lteq) || at(Tok::gteq)) {
    Tok t = cur().kind; p_++;
    auto rhs = parseShift();
    auto b = std::make_unique<BinaryExpr>();
    switch (t) {
      case Tok::lt: b->op = BinaryExpr::Op::lt; break;
      case Tok::gt: b->op = BinaryExpr::Op::gt; break;
      case Tok::lteq: b->op = BinaryExpr::Op::le; break;
      default: b->op = BinaryExpr::Op::ge; break;
    }
    b->line = lhs->line; b->col = lhs->col; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseShift() {
  auto lhs = parseAdd();
  while (at(Tok::shl) || at(Tok::shr)) {
    Tok t = cur().kind; p_++;
    auto rhs = parseAdd();
    auto b = std::make_unique<BinaryExpr>();
    b->op = (t == Tok::shl) ? BinaryExpr::Op::shl : BinaryExpr::Op::shr;
    b->line = lhs->line; b->col = lhs->col; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseAdd() {
  auto lhs = parseMul();
  while (at(Tok::plus) || at(Tok::minus)) {
    Tok t = cur().kind; p_++;
    auto rhs = parseMul();
    auto b = std::make_unique<BinaryExpr>();
    b->op = (t == Tok::plus) ? BinaryExpr::Op::add : BinaryExpr::Op::sub;
    b->line = lhs->line; b->col = lhs->col; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseMul() {
  auto lhs = parseCast();
  while (at(Tok::star) || at(Tok::slash) || at(Tok::percent)) {
    Tok t = cur().kind; p_++;
    auto rhs = parseCast();
    auto b = std::make_unique<BinaryExpr>();
    switch (t) {
      case Tok::star: b->op = BinaryExpr::Op::mul; break;
      case Tok::slash: b->op = BinaryExpr::Op::div; break;
      default: b->op = BinaryExpr::Op::mod; break;
    }
    b->line = lhs->line; b->col = lhs->col; b->lhs = std::move(lhs); b->rhs = std::move(rhs);
    lhs = std::move(b);
  }
  return lhs;
}
ExprPtr Parser::parseCast() {
  auto lhs = parseUnary();
  while (at(Tok::kw_as)) {
    int line = cur().line;
    p_++;
    BType target = parseType();
    auto c = std::make_unique<CastExpr>();
    c->line = line; c->col = lhs->col;
    c->e = std::move(lhs);
    c->target = target;
    lhs = std::move(c);
  }
  return lhs;
}

ExprPtr Parser::parseUnary() {
  if (at(Tok::bang)) {
    p_++;
    auto u = std::make_unique<UnaryExpr>();
    u->op = UnaryExpr::Op::not_;
    u->base = parseUnary();
    return u;
  }
  if (at(Tok::tilde)) {
    p_++;
    auto u = std::make_unique<UnaryExpr>();
    u->op = UnaryExpr::Op::bnot;
    u->base = parseUnary();
    return u;
  }
  if (at(Tok::amp)) {
    p_++;
    auto u = std::make_unique<UnaryExpr>();
    u->op = UnaryExpr::Op::addr;
    u->line = cur().line; u->col = cur().col;
    u->base = parseUnary();
    return u;
  }
  if (at(Tok::star)) {
    p_++;
    auto u = std::make_unique<UnaryExpr>();
    u->op = UnaryExpr::Op::deref;
    u->line = cur().line; u->col = cur().col;
    u->base = parseUnary();
    return u;
  }
  if (at(Tok::minus)) {
    p_++;
    auto u = std::make_unique<UnaryExpr>();
    u->op = UnaryExpr::Op::neg;
    u->base = parseUnary();
    return u;
  }


  if (at(Tok::inc) || at(Tok::dec)) {
    bool isInc = at(Tok::inc);
    p_++;
    auto base = parseUnary();
    return makeIncDec(isInc,false, std::move(base));
  }
  return parsePostfix();
}


ExprPtr Parser::makeIncDec(bool isInc, bool isPost, ExprPtr base) {
  auto d = std::make_unique<IncDecExpr>();
  d->isInc = isInc;
  d->isPost = isPost;
  d->line = base->line; d->col = base->col;
  if (auto v = dynamic_cast<VarRef*>(base.get())) {
    d->kind = AssignTarget::Kind::var;
    d->name = v->name;
  } else if (auto ix = dynamic_cast<Index*>(base.get())) {
    d->kind = AssignTarget::Kind::index;
    d->base = std::move(ix->base);
    d->index = std::move(ix->index);
  } else if (auto fl = dynamic_cast<Field*>(base.get())) {
    d->kind = AssignTarget::Kind::field;
    d->base = std::move(fl->base);
    d->field = fl->field;
  } else if (auto u = dynamic_cast<UnaryExpr*>(base.get())) {
    if (u->op != UnaryExpr::Op::deref) { error("invalid increment/decrement target"); return base; }
    d->kind = AssignTarget::Kind::deref;
    d->base = std::move(u->base);
  } else {
    error("invalid increment/decrement target");
    return base;
  }
  return d;
}

ExprPtr Parser::parsePostfix() {
  auto e = parsePrimary();

  while (true) {
    if (at(Tok::lbracket)) {
      p_++;
      auto ix = std::make_unique<Index>();
      ix->line = cur().line; ix->col = cur().col;
      ix->base = std::move(e);
      ix->index = parseExpr();
      expect(Tok::rbracket, "']'");
      e = std::move(ix);
      continue;
    }
    if (at(Tok::dot)) {
      p_++;
      int line = cur().line, col = cur().col;
      if (!expect(Tok::ident, "field or method name")) break;
      std::string name = toks_[p_ - 1].text;

      if (at(Tok::lparen)) {
        p_++;
        auto mc = std::make_unique<MethodCall>();
        mc->callee = name;
        mc->receiver = std::move(e);
        mc->line = line; mc->col = col;
        while (!at(Tok::rparen) && !at(Tok::end)) {
          mc->args.push_back(parseExpr());
          if (!accept(Tok::comma)) break;
        }
        expect(Tok::rparen, "')'");
        e = std::move(mc);
        continue;
      }
      auto fl = std::make_unique<Field>();
      fl->line = line; fl->col = col;
      fl->base = std::move(e);
      fl->field = name;
      e = std::move(fl);
      continue;
    }


    if (at(Tok::inc) || at(Tok::dec)) {
      bool isInc = at(Tok::inc);
      p_++;
      e = makeIncDec(isInc,true, std::move(e));
      continue;
    }
    break;
  }
  return e;
}
ExprPtr Parser::parsePrimary() {
  if (at(Tok::int_lit)) {
    auto l = std::make_unique<IntLit>();
    l->v = cur().u64; l->line = cur().line; l->col = cur().col;
    p_++;
    return l;
  }
  if (at(Tok::float_lit)) {
    auto l = std::make_unique<FloatLit>();
    l->v = cur().f64; l->line = cur().line; l->col = cur().col;
    p_++;
    return l;
  }
  if (at(Tok::str_lit)) {
    auto l = std::make_unique<StrLit>();
    l->v = cur().text; l->line = cur().line; l->col = cur().col;
    p_++;
    return l;
  }
  if (at(Tok::char_lit)) {
    auto l = std::make_unique<CharLit>();
    l->v = (uint8_t)cur().u64; l->line = cur().line; l->col = cur().col;
    p_++;
    return l;
  }
  if (at(Tok::kw_null)) {
    auto n = std::make_unique<NullLit>();
    n->line = cur().line; n->col = cur().col;
    p_++;
    return n;
  }
  if (at(Tok::kw_fn)) {


    int line = cur().line, col = cur().col;
    p_++;
    auto lam = std::make_unique<LambdaLit>();
    lam->line = line; lam->col = col;
    if (!expect(Tok::lparen, "'(' to begin lambda parameter list")) {
      auto f = std::make_unique<IntLit>(); f->v = 0; f->line = line; return f;
    }
    while (!at(Tok::rparen) && !at(Tok::end)) {
      if (!expect(Tok::ident, "lambda parameter name")) break;
      std::string pname = toks_[p_ - 1].text;
      if (!expect(Tok::colon, "':' after lambda parameter name")) break;
      BType pt = parseType();
      lam->params.push_back({pname, pt});
      if (!accept(Tok::comma)) break;
    }
    expect(Tok::rparen, "')'");
    if (accept(Tok::arrow)) lam->retType = parseType();
    else lam->retType = BType::void_;

    if (!at(Tok::lbrace)) {
      error("expected '{' to begin lambda body");
      auto f = std::make_unique<IntLit>(); f->v = 0; f->line = line; return f;
    }
    bool saved = allowStructLit_; allowStructLit_ = true;
    lam->body = parseBlock();
    allowStructLit_ = saved;
    return lam;
  }
  if (at(Tok::kw_sizeof)) {

    int line = cur().line, col = cur().col;
    p_++;
    expect(Tok::lparen, "'(' after 'sizeof'");
    BType ty = parseType();
    expect(Tok::rparen, "')' to close sizeof(...)");
    auto s = std::make_unique<SizeofExpr>();
    s->target = ty;
    s->line = line; s->col = col;
    return s;
  }
  if (at(Tok::kw_asm)) {


    int line = cur().line, col = cur().col;
    p_++;
    if (!accept(Tok::bang)) {
      error("expected '!' after 'asm'");
      auto z = std::make_unique<IntLit>(); z->v = 0; z->line = line; return z;
    }
    expect(Tok::lparen, "'(' after 'asm!'");
    auto a = std::make_unique<AsmExpr>();
    a->line = line; a->col = col;
    if (!at(Tok::str_lit)) {
      error("expected string literal as asm body");
      auto z = std::make_unique<IntLit>(); z->v = 0; z->line = line; return z;
    }
    a->asmText = cur().text;
    p_++;

    while (accept(Tok::comma)) {
      bool isInOutClause = (cur().kind == Tok::kw_in || cur().kind == Tok::ident) &&
                           (cur().text == "in" || cur().text == "out" || cur().text == "inout");
      if (isInOutClause) {
        bool isInOut = (cur().text == "inout");
        bool isOut = isInOut || (cur().text == "out");
        p_++;
        if (!expect(Tok::lparen, "'(' after in/out/inout clause")) break;

        std::string cstr;
        if (at(Tok::str_lit)) { cstr = cur().text; p_++; }
        else { error("expected constraint string in in/out/inout clause"); break; }
        expect(Tok::rparen, "')' to close in/out/inout clause");
        AsmIO io;
        io.isOutput = isOut;
        io.isInOut = isInOut;
        io.constraint = cstr;
        io.val = parseUnary();
        a->ios.push_back(std::move(io));
      } else if ((cur().kind == Tok::ident) && cur().text == "clobbers") {
        p_++;
        if (!expect(Tok::eq, "'=' after clobbers")) break;
        if (!at(Tok::str_lit)) { error("expected string after clobbers="); break; }
        a->clobbers = cur().text; p_++;
      } else if (at(Tok::ident) && cur().text == "sideeffect") {
        p_++; if (!expect(Tok::eq, "'=' after sideeffect")) break;
        if (at(Tok::kw_true)) { a->sideEffect = true; p_++; }
        else if (at(Tok::kw_false)) { a->sideEffect = false; p_++; }
        else error("expected true/false after sideeffect=");
      } else if (at(Tok::ident) && cur().text == "memory") {
        p_++; if (!expect(Tok::eq, "'=' after memory")) break;
        if (at(Tok::kw_true)) { a->hasMemory = true; p_++; }
        else if (at(Tok::kw_false)) { a->hasMemory = false; p_++; }
        else error("expected true/false after memory=");
      } else {
        error("expected in()/out()/inout()/clobbers=/sideeffect=/memory= in asm! block");
        p_++;
        if (!at(Tok::rparen) && !at(Tok::end)) continue; else break;
      }
    }
    expect(Tok::rparen, "')' to close asm!(...)");
    return a;
  }
  if (at(Tok::kw_self)) {


    auto v = std::make_unique<VarRef>();
    v->name = "self";
    v->line = cur().line; v->col = cur().col;
    p_++;
    return v;
  }
  if (at(Tok::kw_true) || at(Tok::kw_false)) {
    auto l = std::make_unique<BoolLit>();
    l->v = (cur().kind == Tok::kw_true); l->line = cur().line;
    p_++;
    return l;
  }
  if (at(Tok::lparen)) {
    p_++;
    auto e = parseExpr();
    expect(Tok::rparen, "')'");
    return e;
  }
  if (at(Tok::kw_vec)) {

    p_++;
    expect(Tok::lbracket, "'[' after 'vec'");
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    auto dn = std::make_unique<DynNew>();
    dn->elemType = elem;
    dn->line = cur().line; dn->col = cur().col;
    return dn;
  }
  if (at(Tok::kw_map)) {

    p_++;
    expect(Tok::lbracket, "'[' after 'map'");
    BType key = parseType();
    expect(Tok::comma, "',' between key and value in map[K, V]");
    BType val = parseType();
    expect(Tok::rbracket, "']'");
    auto mn = std::make_unique<MapNew>();
    mn->keyType = key; mn->valType = val;
    mn->line = cur().line; mn->col = cur().col;
    return mn;
  }
  if (at(Tok::kw_set)) {
    p_++;
    expect(Tok::lbracket, "'[' after 'set'");
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    auto sn = std::make_unique<SetNew>();
    sn->elemType = elem;
    sn->line = cur().line; sn->col = cur().col;
    return sn;
  }
  if (at(Tok::kw_hmap)) {
    p_++;
    expect(Tok::lbracket, "'[' after 'hmap'");
    BType key = parseType();
    expect(Tok::comma, "',' between key and value in hmap[K, V]");
    BType val = parseType();
    expect(Tok::rbracket, "']'");
    auto hn = std::make_unique<HMapNew>();
    hn->keyType = key; hn->valType = val;
    hn->line = cur().line; hn->col = cur().col;
    return hn;
  }
  if (at(Tok::kw_hset)) {
    p_++;
    expect(Tok::lbracket, "'[' after 'hset'");
    BType elem = parseType();
    expect(Tok::rbracket, "']'");
    auto hn = std::make_unique<HSetNew>();
    hn->elemType = elem;
    hn->line = cur().line; hn->col = cur().col;
    return hn;
  }
  if (at(Tok::lbracket)) {

    p_++;
    auto al = std::make_unique<ArrayLit>();
    al->line = cur().line; al->col = cur().col;
    while (!at(Tok::rbracket) && !at(Tok::end)) {
      al->elems.push_back(parseExpr());
      if (!accept(Tok::comma)) break;
    }
    expect(Tok::rbracket, "']'");
    return al;
  }
  if (at(Tok::ident) || at(Tok::kw_print)) {
    auto name = cur().text;
    int line = cur().line, col = cur().col;
    p_++;


    if (name != "print" && at(Tok::lt)) {
      bool ok = false;
      std::vector<BType> gargs = tryParseTypeArgs(ok);
      if (ok) {

        if (at(Tok::lparen)) {
          p_++;
          auto call = std::make_unique<Call>();
          call->callee = name;
          call->line = line; call->col = col;
          call->typeArgs = std::move(gargs);
          call->hasTypeArgs = true;
          while (!at(Tok::rparen) && !at(Tok::end)) {
            call->args.push_back(parseExpr());
            if (!accept(Tok::comma)) break;
          }
          expect(Tok::rparen, "')'");
          return call;
        }

        if (allowStructLit_ && at(Tok::lbrace)) {
          p_++;
          auto sl = std::make_unique<StructLit>();
          sl->name = name;
          sl->line = line; sl->col = col;
          sl->typeArgs = std::move(gargs);
          sl->hasTypeArgs = true;
          while (!at(Tok::rbrace) && !at(Tok::end)) {
            if (!expect(Tok::ident, "field name")) break;
            sl->fieldNames.push_back(toks_[p_ - 1].text);
            if (!expect(Tok::colon, "':' in struct literal")) break;
            sl->values.push_back(parseExpr());
            if (!accept(Tok::comma)) break;
          }
          expect(Tok::rbrace, "'}'");
          return sl;
        }


        auto v = std::make_unique<VarRef>();
        v->name = name; v->line = line; v->col = col;
        return v;
      }
    }


    if (at(Tok::coloncolon) && name != "print") {
      p_++;

      if (!expect(Tok::ident, "method name after '::'")) {
        auto f = std::make_unique<IntLit>(); f->v = 0; f->line = line; return f;
      }
      std::string mname = toks_[p_ - 1].text;
      if (!expect(Tok::lparen, "'(' for associated function call")) {
        auto f = std::make_unique<IntLit>(); f->v = 0; f->line = line; return f;
      }
      auto ac = std::make_unique<AssocCall>();
      ac->typeName = name;
      ac->callee = mname;
      ac->line = line; ac->col = col;
      while (!at(Tok::rparen) && !at(Tok::end)) {
        ac->args.push_back(parseExpr());
        if (!accept(Tok::comma)) break;
      }
      expect(Tok::rparen, "')'");
      return ac;
    }

    if (allowStructLit_ && at(Tok::lbrace) && name != "print") {
      p_++;
      auto sl = std::make_unique<StructLit>();
      sl->name = name;
      sl->line = line; sl->col = col;
      while (!at(Tok::rbrace) && !at(Tok::end)) {
        if (!expect(Tok::ident, "field name")) break;
        sl->fieldNames.push_back(toks_[p_ - 1].text);
        if (!expect(Tok::colon, "':' in struct literal")) break;
        sl->values.push_back(parseExpr());
        if (!accept(Tok::comma)) break;
      }
      expect(Tok::rbrace, "'}'");
      return sl;
    }
    if (at(Tok::lparen)) {
      p_++;
      auto call = std::make_unique<Call>();
      call->callee = name;
      call->isPrint = (name == "print");
      call->line = line; call->col = col;
      while (!at(Tok::rparen) && !at(Tok::end)) {
        call->args.push_back(parseExpr());
        if (!accept(Tok::comma)) break;
      }
      expect(Tok::rparen, "')'");
      return call;
    }
    auto v = std::make_unique<VarRef>();
    v->name = name; v->line = line; v->col = col;
    return v;
  }
  error("expected an expression but got '" + cur().text + "'");
  auto f = std::make_unique<IntLit>();
  f->v = 0; f->line = cur().line;
  p_++;
  return f;
}
