#pragma once

#include "AST.h"
#include "Sema.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>


class IRGen {
public:
  IRGen(Sema& sema);
  void generate(Program& prog);
  std::string takeIR() { return out_.str(); }


  void setTargetTriple(const std::string& t) { targetTriple_ = t; }

private:
  Sema& sema_;
  std::string targetTriple_;
  std::ostringstream out_;
  std::ostringstream globals_;


  std::ostringstream lambdas_;
  std::map<std::string, StructDef*> structDefs_;
  std::set<std::string> userDefinedFns_;


  std::set<std::string> usedVec_;
  bool usedVec_blob_ = false;
  std::set<std::string> usedSort_;
  bool usedSort_blob_ = false;
  bool usedMap_ = false;
  bool usedSet_ = false;
  bool usedHMap_ = false;
  bool usedHSet_ = false;


  bool dynSetPending_ = false;
  bool dynSetBlob_ = false;
  std::string dynSetHandle_, dynSetIdx_, dynSetSx_;
  BType dynSetEt_ = BType::void_;

  std::string typeStr(BType t);

  std::string elemSuffix(const BType& t);

  std::string elemIrType(const BType& t);

  std::string vecSlotType(const std::string& sx);
  BType vecSlotBType(const std::string& sx);


  std::string curFnName_;
  BType curFnRet_ = BType::void_;
  std::string curBlock_;
  int labelSeq_ = 0;
  int strSeq_ = 0;
  bool terminated_ = false;
  struct LoopCtx { std::string cont; std::string brk; };
  std::vector<LoopCtx> loops_;
  std::vector<std::map<std::string, std::pair<std::string, BType>>> scopes_;

  void emit(const std::string& s);
  void beginBlock(const std::string& name);
  void branch(const std::string& condVal, const std::string& t, const std::string& f);
  void jump(const std::string& t);
  void ensureTerminated();

  std::string freshLabel(const std::string& hint);
  std::string freshGlobal(const std::string& hint);
  std::string freshLocal(const std::string& hint);
  int freshInt();

  std::pair<std::string, BType> findVar(const std::string& name);
  void pushScope();
  void popScope();
  void declareVar(const std::string& name, const std::string& storage, BType t);
  void collectStruct(const BType& t);


  std::string globalInit(const BType& t, const struct GlobalInfo& gi, bool folded);

  void emitGlobalsAndExterns();


  std::pair<std::string, BType> genAddr(Expr* e);


  void boundsCheck(const std::string& idx, int32_t count);


  void strBoundsCheck(const std::string& strPtr, const std::string& idx);


  std::pair<std::string, BType> genExpr(Expr* e);


  std::string genCoerce(const std::string& v, BType fromT, BType toT);

  void printValue(const std::string& val, const BType& t, const std::string& prefix = "");
  std::string strConst(const std::string& s);

  std::pair<std::string, BType> lowerBuiltin(Call* c);


  std::string spillScratch(const std::string& v, const BType& t);


  std::string loadScratch(const std::string& ptr, const BType& t);


  std::pair<std::string, BType> emitMethodCall(const std::string& structName,
                                                const std::string& methodName,
                                                const BType& recvType, bool recvByRef,
                                                Expr* receiver,
                                                const std::vector<ExprPtr>& args);


  std::pair<std::string, BType> emitOverloadCall(const std::string& structName,
                                                 const std::string& methodName,
                                                 const BType& recvType, bool recvByRef,
                                                 Expr* receiver,
                                                 const std::vector<ExprPtr>& args,
                                                 bool negateResult);
  void genStmt(Stmt* s);
  void genBlock(const std::vector<StmtPtr>& stmts);
  void genFunc(FuncDecl& fn);


  void genLambda(const LambdaLit* lam);
  std::set<std::string> emittedLambdas_;


  std::set<std::string> declaredIntrinsics_;
  void ensureIntrinsic(const std::string& decl, const std::string& name);


  void genMethod(const std::string& structName, FuncDecl& fn);
  void emitHeaderAndRuntime();
};
