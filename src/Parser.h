#pragma once

#include "AST.h"
#include "Lexer.h"

struct ParseError {
  std::string msg;
  int line;
  int col;
};

class Parser {
public:
  Parser(std::vector<Token> toks, std::vector<ParseError>& errs);
  std::unique_ptr<Program> parseProgram();

private:
  std::vector<Token> toks_;
  size_t p_ = 0;
  std::vector<ParseError>* errs_;
  bool allowStructLit_ = true;

  const Token& cur();
  const Token& peek(size_t off = 0);
  bool at(Tok t);
  bool accept(Tok t);
  bool expect(Tok t, const std::string& what);
  void error(const std::string& msg);


  bool atTopLevel();

  BType parseType();


  std::vector<std::string> parseTypeParams();


  std::vector<BType> tryParseTypeArgs(bool& ok);
  std::unique_ptr<FuncDecl> parseFunc(bool isExtern = false);
  std::unique_ptr<StructDecl> parseStruct();
  std::unique_ptr<EnumDecl> parseEnum();
  std::unique_ptr<ImplDecl> parseImpl();
  std::unique_ptr<TypedefDecl> parseTypedef();
  std::unique_ptr<VarDecl> parseGlobal(bool isExtern);
  StmtPtr parseStmt();
  StmtPtr parseLetMut(bool isMut);
  StmtPtr parseIf();
  StmtPtr parseWhile();
  StmtPtr parseFor();
  StmtPtr parseMatch();
  StmtPtr parseReturn();
  std::vector<StmtPtr> parseBlock();

  ExprPtr parseExpr();
  ExprPtr parseAssign();
  ExprPtr parseTernary();
  ExprPtr makeIncDec(bool isInc, bool isPost, ExprPtr base);
  ExprPtr parseLogicOr();
  ExprPtr parseLogicAnd();
  ExprPtr parseBitOr();
  ExprPtr parseBitXor();
  ExprPtr parseBitAnd();
  ExprPtr parseEquality();
  ExprPtr parseRel();
  ExprPtr parseShift();
  ExprPtr parseAdd();
  ExprPtr parseMul();
  ExprPtr parseCast();
  ExprPtr parseUnary();
  ExprPtr parsePostfix();
  ExprPtr parsePrimary();


  using PrecedenceFn = ExprPtr (Parser::*)();
};
