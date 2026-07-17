#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <tuple>


struct BType {
  enum class Tag {
    i64, f64, bool_, void_, str, array, struct_, dynarray,

    char_, ptr,
    i8, i16, i32, u8, u16, u32, u64, usize,


    enum_,


    f32,


    map_,


    set_,


    hmap_,


    hset_,


    fn_,


    generic_,
  } tag = Tag::void_;

  int32_t count = 0;
  int32_t elem = 0;
  std::string structName;

  bool operator==(const BType& o) const {
    return tag == o.tag && count == o.count && elem == o.elem && structName == o.structName;
  }
  bool operator!=(const BType& o) const { return !(*this == o); }


  static const BType i64;
  static const BType f64;
  static const BType bool_;
  static const BType void_;
  static const BType str;

  static const BType char_;
  static const BType i8, i16, i32, u8, u16, u32, u64, usize;
  static const BType f32;
};

inline const BType BType::i64   {Tag::i64};
inline const BType BType::f64   {Tag::f64};
inline const BType BType::bool_ {Tag::bool_};
inline const BType BType::void_ {Tag::void_};
inline const BType BType::str    {Tag::str};
inline const BType BType::char_  {Tag::char_};
inline const BType BType::i8     {Tag::i8};
inline const BType BType::i16    {Tag::i16};
inline const BType BType::i32    {Tag::i32};
inline const BType BType::u8     {Tag::u8};
inline const BType BType::u16    {Tag::u16};
inline const BType BType::u32    {Tag::u32};
inline const BType BType::u64    {Tag::u64};
inline const BType BType::usize  {Tag::usize};
inline const BType BType::f32    {Tag::f32};

bool isInt(const BType& t);
bool isFloat(const BType& t);
bool isNumeric(const BType& t);
bool isScalar(const BType& t);
bool isSignedInt(const BType& t);
int bitWidth(const BType& t);
const char* typeName(const BType& t);
std::string typeSpelling(const BType& t);


BType makeArrayType(const BType& elem, int32_t count);
const BType& arrayElem(const BType& arr);


BType makeDynArray(const BType& elem);
const BType& dynArrayElem(const BType& arr);

BType makePtr(const BType& pointee);
const BType& pointee(const BType& p);

BType makeMapType(const BType& key, const BType& val);
const BType& mapKeyType(const BType& m);
const BType& mapValType(const BType& m);


BType makeSetType(const BType& elem);
const BType& setElemType(const BType& s);
BType makeHMapType(const BType& key, const BType& val);
BType makeHSetType(const BType& elem);


BType makeFnType(const std::vector<BType>& params, const BType& ret);
const std::vector<BType>& fnParams(const BType& t);
BType fnRet(const BType& t);


BType makeGenericInst(const std::string& base, const std::vector<BType>& args, bool isFn);
bool isGenericInst(const BType& t);
const std::string& genericInstBase(const BType& t);
const std::vector<BType>& genericInstArgs(const BType& t);
bool genericInstIsFn(const BType& t);


struct StructDecl;
struct FuncDecl;
void registerGenericStruct(const StructDecl* tmpl);
const StructDecl* findGenericStruct(const std::string& name);
void registerGenericFn(const FuncDecl* tmpl);
const FuncDecl* findGenericFn(const std::string& name);


BType instantiateGenericStruct(const std::string& base, const std::vector<BType>& args);


void registerAlias(const std::string& name, const BType& target);
bool isAliasName(const std::string& name);
std::pair<bool, BType> resolveAlias(const std::string& name);

struct StructField { std::string name; BType type; int32_t offset = 0; bool isPrivate = false; };
struct StructDef {
  std::string name;
  std::vector<StructField> fields;
  int32_t size = 0;
  int32_t align = 0;
};
StructDef* registerStruct(const std::string& name);
StructDef* findStruct(const std::string& name);


std::vector<StructDef*> allStructDefs();
int32_t structFieldIndex(const StructDef* d, const std::string& field);


int32_t fieldByteWidth(const BType& t);


int32_t fieldAlign(const BType& t);


struct EnumDef {
  std::string name;
  std::vector<std::string> variants;
};
EnumDef* registerEnum(const std::string& name);
EnumDef* findEnum(const std::string& name);


std::pair<EnumDef*, int64_t> resolveEnumVariant(const std::string& name);

BType makeEnumType(const std::string& name);


struct Expr {
  int line = 0;
  int col = 0;
  virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;


ExprPtr cloneExpr(const Expr* e);

struct IntLit : Expr { uint64_t v = 0; };
struct FloatLit : Expr { double v = 0; };
struct BoolLit : Expr { bool v = false; };
struct StrLit : Expr { std::string v; };
struct CharLit : Expr { uint8_t v = 0; };

struct VarRef : Expr { std::string name; };

struct UnaryExpr : Expr {
  enum class Op { neg, not_, bnot, addr, deref } op;
  ExprPtr base;


  bool methodOverload = false;
  std::string overloadStruct;
  std::string overloadMethod;
  BType overloadRecvType = BType::void_;
  bool recvByRef = false;
};


struct CastExpr : Expr {
  ExprPtr e;
  BType target = BType::i64;
};


struct TernaryExpr : Expr {
  ExprPtr cond;
  ExprPtr thenE;
  ExprPtr elseE;
  BType resultTy = BType::void_;
};


struct SizeofExpr : Expr {
  BType target = BType::i64;
  int32_t size = 0;
};


struct AsmIO {
  bool isOutput = false;
  bool isInOut = false;


  std::string constraint;
  ExprPtr val;
  BType ty = BType::void_;
};
struct AsmExpr : Expr {
  std::string asmText;
  std::vector<AsmIO> ios;
  std::string clobbers;
  bool sideEffect = true;
  bool hasMemory = false;
  BType resultTy = BType::void_;


  std::vector<BType> outputTypes;
};


struct NullLit : Expr {};


struct GenericTypeRef : Expr {
  std::string base;
  std::vector<ExprPtr> typeArgs;
};


struct BinaryExpr : Expr {
  enum class Op {
    add, sub, mul, div, mod,
    eq, ne, lt, gt, le, ge,
    land, lor, band, bor, bxor, shl, shr,
  } op;
  ExprPtr lhs, rhs;


  bool methodOverload = false;
  std::string overloadStruct;
  std::string overloadMethod;
  BType overloadRecvType = BType::void_;
  bool recvByRef = false;


  bool isPtrArith = false;
  BType ptrArithPointee = BType::void_;
};


struct AssignTarget : Expr {
  enum class Kind { var, index, field, deref } kind = Kind::var;
  std::string name;
  ExprPtr base;
  ExprPtr index;
  std::string field;
  ExprPtr value;
  BinaryExpr::Op compound = BinaryExpr::Op::add;
  bool isCompound = false;


  bool methodOverload = false;
  std::string overloadStruct;
  std::string overloadMethod;
  BType overloadRecvType = BType::void_;
  bool recvByRef = true;
};


struct IncDecExpr : Expr {
  bool isInc = true;
  bool isPost = false;
  AssignTarget::Kind kind = AssignTarget::Kind::var;
  std::string name;
  ExprPtr base;
  ExprPtr index;
  std::string field;
  BType valueTy = BType::void_;
};

struct Call : Expr {
  std::string callee;
  std::vector<ExprPtr> args;
  bool isPrint = false;


  std::vector<BType> typeArgs;
  bool hasTypeArgs = false;


  bool fnPtr = false;
  ExprPtr calleeExpr;
};


struct MethodCall : Expr {
  std::string callee;
  ExprPtr receiver;
  std::vector<ExprPtr> args;
  bool receiverByRef = false;
  BType recvType = BType::void_;
};


struct AssocCall : Expr {
  std::string typeName;
  std::string callee;
  std::vector<ExprPtr> args;
};


struct Index : Expr {
  ExprPtr base;
  ExprPtr index;


  bool methodOverload = false;
  std::string overloadStruct;
  std::string overloadMethod;
  BType overloadRecvType = BType::void_;
  bool recvByRef = false;
};
struct Field : Expr {
  ExprPtr base;
  std::string field;
};

struct ArrayLit : Expr {
  std::vector<ExprPtr> elems;
};
struct StructLit : Expr {
  std::string name;
  std::vector<std::string> fieldNames;
  std::vector<ExprPtr> values;


  std::vector<BType> typeArgs;
  bool hasTypeArgs = false;
};

struct DynNew : Expr {
  BType elemType = BType::i64;
};
struct MapNew : Expr {
  BType keyType = BType::i64;
  BType valType = BType::i64;
};
struct SetNew : Expr {
  BType elemType = BType::i64;
};
struct HMapNew : Expr {
  BType keyType = BType::i64;
  BType valType = BType::i64;
};
struct HSetNew : Expr {
  BType elemType = BType::i64;
};


enum class StmtKind {
  exprStmt, letStmt, returnStmt, ifStmt, whileStmt, forStmt,
  breakStmt, continueStmt, block,
};

struct Stmt {
  int line = 0;
  int col = 0;
  StmtKind kind;
  Stmt(StmtKind k) : kind(k) {}
  virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt { ExprPtr expr; ExprStmt() : Stmt(StmtKind::exprStmt) {} };
struct LetStmt : Stmt {
  bool isMut = false;
  std::string name;
  BType type = BType::i64;
  bool typeAnnotated = false;
  ExprPtr init;
  LetStmt() : Stmt(StmtKind::letStmt) {}
};
struct ReturnStmt : Stmt { ExprPtr value; ReturnStmt() : Stmt(StmtKind::returnStmt) {} };
struct IfStmt : Stmt {
  ExprPtr cond;
  std::vector<StmtPtr> then;
  std::vector<StmtPtr> else_;
  IfStmt() : Stmt(StmtKind::ifStmt) {}
};
struct WhileStmt : Stmt {
  ExprPtr cond;
  std::vector<StmtPtr> body;
  WhileStmt() : Stmt(StmtKind::whileStmt) {}
};
struct ForStmt : Stmt {
  std::string varName;
  ExprPtr start, end, step;
  bool inclusiveEnd = true;


  bool isForeach = false;
  ExprPtr iter;
  BType elemType = BType::i64;
  std::vector<StmtPtr> body;
  ForStmt() : Stmt(StmtKind::forStmt) {}
};
struct BreakStmt : Stmt { BreakStmt() : Stmt(StmtKind::breakStmt) {} };
struct ContinueStmt : Stmt { ContinueStmt() : Stmt(StmtKind::continueStmt) {} };
struct Block : Stmt {
  std::vector<StmtPtr> stmts;
  Block() : Stmt(StmtKind::block) {}
};

struct Param { std::string name; BType type; };


struct LambdaLit : Expr {
  std::vector<Param> params;
  BType retType = BType::void_;
  std::vector<StmtPtr> body;
  std::string loweredName;
  BType fnType = BType::void_;
};

struct FuncDecl {
  std::string name;
  std::vector<Param> params;
  BType retType = BType::void_;
  std::vector<StmtPtr> body;
  int line = 0;
  bool isExtern = false;


  std::vector<std::string> typeParams;
  bool isGeneric = false;


  std::string implStruct;
  bool hasSelf = false;
  bool selfByRef = false;
};


struct VarDecl {
  std::string name;
  BType type = BType::i64;
  bool typeAnnotated = false;
  bool isConst = false;
  bool isExtern = false;
  bool isMut = false;
  ExprPtr init;
  int line = 0;
};


struct TypedefDecl {
  std::string name;
  BType target = BType::i64;
  int line = 0;
};

struct StructDecl {
  std::string name;


  std::vector<std::tuple<std::string, BType, bool>> fields;
  int line = 0;


  std::vector<std::string> typeParams;
  bool isGeneric = false;
};

struct EnumDecl {
  std::string name;
  std::vector<std::string> variants;
  int line = 0;
};


struct ImplDecl {
  std::string structName;
  std::vector<std::unique_ptr<FuncDecl>> methods;
  int line = 0;
};

struct Program {
  std::vector<std::unique_ptr<FuncDecl>> funcs;
  std::vector<std::unique_ptr<StructDecl>> structs;
  std::vector<std::unique_ptr<EnumDecl>> enums;
  std::vector<std::unique_ptr<VarDecl>> globals;
  std::vector<std::unique_ptr<TypedefDecl>> typedefs;
  std::vector<std::unique_ptr<ImplDecl>> impls;
};
