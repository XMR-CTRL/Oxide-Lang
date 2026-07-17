#pragma once

#include "AST.h"
#include <map>
#include <vector>
#include <string>

struct SemanticError {
  std::string msg;
  int line;
  int col = 0;
  std::string hint;
};

struct VarInfo {
  BType type = BType::void_;
  bool isMut = false;
  bool found = false;
};

struct FuncSig {
  BType retType;
  std::vector<BType> paramTypes;
  bool isExtern = false;
};


struct MethodInfo {
  BType retType = BType::void_;
  std::vector<BType> paramTypes;
  bool hasSelf = false;
  bool selfByRef = false;
  std::string implStruct;
  std::string mangled;
};


std::string mangleMethod(const std::string& structName, const std::string& methodName);


struct GlobalInfo {
  BType type = BType::void_;
  bool isConst = false;
  bool isExtern = false;
  bool isMut = false;

  bool hasConstVal = false;
  int64_t iVal = 0;
  double fVal = 0;
  bool bVal = false;
  uint8_t cVal = 0;
  std::string sVal;
};

class Sema {
public:
  bool check(Program& prog);


  bool requireMain = true;


  bool freestanding = false;

  std::vector<SemanticError> errs;
  std::map<std::string, FuncSig> funcs;


  std::map<std::string, GlobalInfo> globals;


  std::map<std::string, std::map<std::string, MethodInfo>> methods;


  std::vector<std::unique_ptr<FuncDecl>> monomorphFns;

private:
  struct Entry { BType type; bool isMut; bool isGlobal = false; };


  std::string curImpl_;
  std::vector<std::map<std::string, Entry>> scopes_;
  FuncSig* curFunc_ = nullptr;
  int loopDepth_ = 0;


  int lambdaSeq_ = 0;
  int monoSeq_ = 0;

  VarInfo lookup(const std::string& name);
  void pushScope();
  void popScope();
  void declare(const std::string& name, BType t, bool isMut, bool isGlobal = false);

  BType checkExpr(Expr* e);
  void checkStmt(Stmt* s);
  void checkBlock(const std::vector<StmtPtr>& stmts);


  bool canTouchPrivate(const std::string& structName);


  void registerMethod(const std::string& structName, FuncDecl& fn);


  const struct MethodInfo* resolveOverload(const std::string& sn,
                                           BinaryExpr::Op op, Expr* b);


  bool foldConstExpr(Expr* e, GlobalInfo& gi);


  std::string instantiateGenericFn(const std::string& name, const std::vector<BType>& args);


  BType checkLambda(LambdaLit* lam);
};
