#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "src/assembly/assembly.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace language {


class Decl {
public:
  enum class Category { CLASS, FUNC };
  constexpr Decl(size_t line) : line_(line) {}
  virtual ~Decl() {}
  virtual void toImDecls(std::vector<im::DeclPtr>&, Ctx&) = 0;
  virtual void addToCtx(Ctx& ctx) = 0;
  virtual Category getCategory() const noexcept = 0;

  size_t line_;
};

class Stmt {
public:
  constexpr Stmt(size_t line) : line_(line) {}
  virtual ~Stmt() {}
  /* Retrieves the last statement within this statement. Return nullptr if this
   * statement does not contain any statements */
  virtual const std::unique_ptr<Stmt>* lastStmt() const;
  /* If the statement typechecks, generate the corresponding intermediate
   * statements */
  virtual void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) = 0;

  size_t line_;
};


enum class UOp { NEG, NOT };
enum class BOp {
  PLUS,
  MUL,
  MINUS,
  DIV,
  MOD,
  BIT_AND,
  BIT_OR,
  XOR,
  LSHIFT,
  ARSHIFT,
  EQ,
  NEQ,
  GT,
  LT,
  GTE,
  LTE,
  AND,
  OR
};


struct ExprInfo {
  im::ExprPtr imExpr;
  TypePtr type;
};


class Expr {
public:
  enum class Category {
    CONST_INT,
    CONST_CHAR,
    CONST_BOOL,
    VAR,
    UNARY_OP,
    BINARY_OP,
    TERNARY_OP,
    CALL_EXPR,
    CAST,
    NEW_ARRAY,
    NEW_OBJECT,
    ARRAY_ACCESS,
    MEMBER_ACCESS,
    METHOD_INVOCATION,
    INC_DEC,
    IM_WRAPPER,
  };

  constexpr Expr(size_t line) : line_(line) {}
  virtual ~Expr() {}
  virtual Category getCategory() const noexcept = 0;
  virtual bool isLValue() const noexcept;

  /* This and all related functions invalidate the Expr that calls it */
  virtual ExprInfo toImExpr(Ctx& ctx) = 0;
  /* If this typechecks to a bool, add statements to jump to ifTrue it
   * evaluates to true and ifFalse if it evaluates to false. If flipEquiv
   * is true, it will evaluate the opposite operation and jump to the
   * opposite labels. This allows us to eliminate consecutive jumps */
  virtual void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx);
  virtual std::unique_ptr<Expr> clone() const = 0;

  template <typename F>
  ExprInfo toImExprAssert(F&& condFn, std::string_view errMsg, Ctx& ctx);
  im::ExprPtr toImExprAssert(const Type& type, Ctx& ctx);

  size_t line_;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;


/********
 * Decl *
 ********/

class Block;
class Func : public Decl {
public:
  Func(
      TypePtr&& returnType,
      std::string_view name,
      std::vector<std::pair<TypePtr, std::string>>&& params,
      std::unique_ptr<Block>&& body,
      size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;
  void addToCtx(Ctx& ctx) override;
  Category getCategory() const noexcept override { return Category::FUNC; }
  void checkTypes(Ctx& ctx) const;

  TypePtr returnType_;
  std::string name_;
  std::vector<TypePtr> paramTypes_;
  std::vector<std::string> paramNames_;
  std::unique_ptr<Block> body_;

protected:
  virtual std::vector<im::StmtPtr> paramsToImStmts(Ctx& ctx);

private:
  /* Check if for no return at end of function and handle accordingly */
  void checkForReturn(Ctx& ctx);
};

class Constructor : public Func {
public:
  Constructor(
      std::string_view name,
      std::vector<std::pair<TypePtr, std::string>>&& params,
      std::unique_ptr<Block>&& body,
      size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;

  size_t objSize_ = 0;
};


class ClassDecl : public Decl {
public:
  struct Field {
    TypePtr type;
    std::string name;
    size_t line;
  };

  struct ClassElem {
    enum class Type { FIELD, CTOR, METHOD };

    Type type;
    std::variant<Field, Constructor, std::unique_ptr<Func>> elem;
  };

  static std::string mangleMethod(std::string_view className, std::string_view fnName);

  static const std::string THIS;

  ClassDecl(std::string_view name, std::vector<ClassElem>&& classElems, size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;
  void addToCtx(Ctx& ctx) override;
  Category getCategory() const noexcept override { return Category::CLASS; }

  std::string name_;
  std::vector<Field> fields_;
  std::vector<Constructor> ctors_;
  std::vector<std::unique_ptr<Func>> methods_;
  size_t id_;
};

/***********
 * Program *
 ***********/
struct Import {
  std::string filename;
  size_t line;
};

class Program {
public:
  static void setImportPath(std::string_view importPath);

  /* Directory for imports provided by the language */
  static std::string importDir;
  static std::vector<std::string> importPathParts;

  Program(std::vector<Import>&& imports, std::vector<DeclPtr>&& decls);
  assem::Program toAssemProg() const;
  void initContext(
      std::string_view filename,
      std::unordered_map<std::string, std::unique_ptr<Program>>& initializedProgs,
      std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds);

  im::Program toImProg() const;

  std::vector<Import> imports_;
  std::vector<std::unique_ptr<ClassDecl>> classes_;
  std::vector<std::unique_ptr<Func>> funcs_;
  std::shared_ptr<Ctx> ctx_;
};

/********
 * Stmt *
 ********/

class VarDecl : public Stmt {
public:
  VarDecl(TypePtr&& type, std::string_view name, ExprPtr&& e, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  TypePtr type_;
  std::string name_;
  ExprPtr e_;
};

class Block : public Stmt {
public:
  Block(std::vector<StmtPtr>&& stmts, size_t line);
  const StmtPtr* lastStmt() const override;
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  std::vector<StmtPtr> stmts_;
};


class If : public Stmt {
public:
  If(ExprPtr&& boolE, std::unique_ptr<Block>&& ifE, StmtPtr&& elseE, size_t line);
  const StmtPtr* lastStmt() const override;
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr boolE_;
  std::unique_ptr<Block> ifE_;
  StmtPtr elseE_;
};


class While : public Stmt {
public:
  While(ExprPtr&& boolE, std::unique_ptr<Block>&& body, size_t line);
  const StmtPtr* lastStmt() const override;
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr boolE_;
  std::unique_ptr<Block> body_;
};

class For : public Stmt {
public:
  For(std::unique_ptr<VarDecl>&& varDecl,
      ExprPtr&& boolE,
      StmtPtr&& update,
      std::unique_ptr<Block>&& body,
      size_t line);
  const StmtPtr* lastStmt() const override;
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  std::unique_ptr<VarDecl> varDecl_;
  ExprPtr boolE_;
  std::unique_ptr<Block> body_;
  StmtPtr update_;
};


class ExprStmt : public Stmt {
public:
  ExprStmt(ExprPtr expr, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr expr_;
};


class Return : public Stmt {
public:
  Return(std::optional<ExprPtr>&& retValue, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  std::optional<ExprPtr> retValue_;
};


class Assign : public Stmt {
public:
  Assign(ExprPtr&& lValue, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr lValue_;
  ExprPtr rhs_;
};


class Update : public Stmt {
public:
  static std::unique_ptr<im::MemDeref> derefTemp(int temp, u_char numBytes);
  static std::unique_ptr<im::Assign> assignAddr(int temp, im::MemDeref* memDeref);

  Update(ExprPtr&& lValue, BOp bOp, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr lValue_;
  ExprPtr rhs_;
  BOp bOp_;
};


class Print : public Stmt {
public:
  Print(ExprPtr&& expr, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr expr_;
};


/********
 * Expr *
 ********/

class ConstInt : public Expr {
public:
  constexpr explicit ConstInt(long n, size_t line) : Expr(line), n_(n) {}
  Category getCategory() const noexcept override { return Category::CONST_INT; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  long n_;
};

class ConstChar : public Expr {
public:
  constexpr explicit ConstChar(char c, size_t line) : Expr(line), c_(c) {}
  Category getCategory() const noexcept override { return Category::CONST_CHAR; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  char c_;
};


class ConstBool : public Expr {
public:
  constexpr explicit ConstBool(bool b, size_t line) : Expr(line), b_(b) {}
  Category getCategory() const noexcept override { return Category::CONST_BOOL; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  bool b_;
};


/* Variable in the program */
class Var : public Expr {
public:
  Var(std::string_view name, size_t line);
  Category getCategory() const noexcept override { return Category::VAR; }
  bool isLValue() const noexcept override;
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  const std::string& getName() const noexcept { return name_; }

  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp, size_t line);
  Category getCategory() const noexcept override { return Category::UNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) override;

  ExprPtr e_;
  UOp uOp_;
};


class BinaryOp : public Expr {
public:
  BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
  Category getCategory() const noexcept override { return Category::BINARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) override;


  ExprPtr e1_;
  ExprPtr e2_;
  BOp bOp_;

private:
  ExprInfo toImExprArith(im::BOp op, Ctx& ctx);
  void asBoolComp(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      im::ROp rOp,
      Ctx& ctx);
  void asBoolAnd(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx);
  void asBoolOr(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx);
  void asBoolXor(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx);
};


class TernaryOp : public Expr {
public:
  TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2);
  Category getCategory() const noexcept override { return Category::TERNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr boolE_;
  ExprPtr e1_;
  ExprPtr e2_;
};


class CallExpr : public Expr {
public:
  CallExpr(
      std::vector<std::string>&& qualifiers,
      std::string_view name,
      std::vector<ExprPtr>&& params,
      size_t line);
  Category getCategory() const noexcept override { return Category::CALL_EXPR; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  std::vector<std::string> qualifiers_;
  std::string name_;
  std::vector<ExprPtr> params_;
};


class Cast : public Expr {
public:
  Cast(TypePtr&& toType, ExprPtr&& expr, size_t line);
  bool isLValue() const noexcept override;
  Category getCategory() const noexcept override { return Category::CAST; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  TypePtr toType_;
  ExprPtr expr_;
};


class NewArray : public Expr {
public:
  NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line);
  NewArray(TypePtr&& type, std::vector<ExprPtr>&& elems, size_t line);
  Category getCategory() const noexcept override { return Category::NEW_ARRAY; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;


  TypePtr type_;
  ExprPtr numElems_;
  std::vector<ExprPtr> elems_;

private:
  /* Returns the statements to create the array as well as the temp containing
   * the address of the new array */
  std::pair<std::vector<im::StmtPtr>, int> makeArrayStmts(Type& type, ExprPtr&& numElems, Ctx& ctx);
  ExprInfo toImExprLen(Ctx& ctx);
  ExprInfo toImExprElems(Ctx& ctx);
};


class ArrayAccess : public Expr {
public:
  ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line);
  bool isLValue() const noexcept override;
  Category getCategory() const noexcept override { return Category::ARRAY_ACCESS; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr arrExpr_;
  ExprPtr index_;
};


class MemberAccess : public Expr {
public:
  MemberAccess(ExprPtr&& objExpr, std::string_view member, size_t line);
  bool isLValue() const noexcept override;
  Category getCategory() const noexcept override { return Category::MEMBER_ACCESS; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr objExpr_;
  std::string member_;
};


class MethodInvocation : public Expr {
public:
  MethodInvocation(
      ExprPtr&& objExpr,
      std::string_view methodName,
      std::vector<ExprPtr>&& params,
      size_t line);
  Category getCategory() const noexcept override { return Category::METHOD_INVOCATION; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr objExpr_;
  std::string methodName_;
  std::vector<ExprPtr> params_;
};


class IncDec : public Expr {
public:
  IncDec(ExprPtr&& lValue, bool inc, bool pre, size_t line);
  Category getCategory() const noexcept override { return Category::INC_DEC; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;


  ExprPtr lValue_;
  bool inc_;
  bool pre_;
};


class ImWrapper : public Expr {
public:
  ImWrapper(im::ExprPtr&& imExpr, TypePtr type, bool isLValue, size_t line);
  bool isLValue() const noexcept override;
  Category getCategory() const noexcept override { return Category::IM_WRAPPER; }
  ExprInfo toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  im::ExprPtr imExpr_;
  TypePtr type_;
  bool isLValue_;
};


}  // namespace language


#endif  // LANGUAGE_HPP
