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
#include <tuple>
#include <utility>
#include <vector>

namespace language {


class Decl {
public:
  constexpr Decl(size_t line) : line_(line) {}
  virtual ~Decl() {}
  virtual void toImDecls(std::vector<im::DeclPtr>&, Ctx&) = 0;
  virtual void addToContext(Ctx& ctx) = 0;
  constexpr size_t getLine() const noexcept { return line_; }

protected:
  size_t line_;
};

class Stmt {
public:
  constexpr Stmt(size_t line) : line_(line) {}
  virtual ~Stmt() {}
  /* If the statement typechecks, generate the corresponding intermediate
   * statements */
  virtual void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) = 0;
  constexpr size_t getLine() const noexcept { return line_; }

protected:
  size_t line_;
};

// TODO: Benchmark virtual function call for getType() vs a <BaseClass>Type data
// member
enum class ExprType {
  CONST_INT,
  CONST_BOOL,
  VAR,
  TEMP,
  UNARY_OP,
  BINARY_OP,
  TERNARY_OP,
  CALL_EXPR,
  NEW_ARRAY,
  ARRAY_ACCESS
};


struct ExprInfo {
  im::ExprPtr imExpr;
  TypePtr type;
};


class Expr {
public:
  constexpr Expr(size_t line) : line_(line) {}
  virtual ~Expr() {}
  virtual ExprType getType() const noexcept = 0;

  virtual ExprInfo toImExpr(Ctx& ctx) = 0;
  /* If this typechecks to a bool, add statements to jump to ifTrue it
   * evaluates to true and ifFalse if it evaluates to false. */
  virtual void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      Ctx& ctx);

  im::ExprPtr toImExprAssert(const Type& type, Ctx& ctx);
  constexpr size_t getLine() const noexcept { return line_; }

protected:
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
  void addToContext(Ctx& ctx) override;
  /* Check if for no return at end of function and handle accordingly */
  void checkForReturn(Ctx& ctx);

private:
  TypePtr returnType_;
  std::string name_;
  std::optional<std::string> mangledName_;
  std::vector<TypePtr> paramTypes_;
  std::vector<std::string> paramNames_;
  std::unique_ptr<Block> body_;
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
  Program(std::vector<Import>&& imports, std::vector<DeclPtr>&& decls);
  assem::Program toAssemProg() const;
  void initContext(
      std::string_view filename,
      std::unordered_map<std::string, std::unique_ptr<Program>>& initializedProgs,
      std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds,
      std::shared_ptr<std::unordered_map<std::string, std::string>> typeIds);
  const Ctx& getCtx() const noexcept { return *ctx_; }

private:
  im::Program toImProg() const;

  std::vector<Import> imports_;
  std::vector<DeclPtr> decls_;
  std::shared_ptr<Ctx> ctx_;
};

/********
 * Stmt *
 ********/

class Block : public Stmt {
public:
  Block(std::vector<StmtPtr>&& stmts, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);
  friend class Func;

private:
  std::vector<StmtPtr> stmts_;
};


class If : public Stmt {
public:
  If(ExprPtr&& boolE,
     std::unique_ptr<Block>&& ifE,
     StmtPtr&& elseE,
     size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> ifE_;
  StmtPtr elseE_;
};


class While : public Stmt {
public:
  While(ExprPtr&& boolE, std::unique_ptr<Block> body, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> body_;
};

class CallExpr;
class CallStmt : public Stmt {
public:
  CallStmt(
      std::vector<std::string>&& qualifiers,
      std::string_view name,
      std::vector<ExprPtr>&& params,
      size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

private:
  std::vector<std::string> qualifiers_;
  std::string name_;
  std::vector<ExprPtr> params_;
};


class Return : public Stmt {
public:
  Return(std::optional<ExprPtr>&& retValue, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

private:
  std::optional<ExprPtr> retValue_;
};


class Assign : public Stmt {
public:
  Assign(ExprPtr&& lhs, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

private:
  ExprPtr lhs_;
  ExprPtr rhs_;
};


class VarDecl : public Stmt {
public:
  VarDecl(TypePtr&& type, std::string_view name, ExprPtr&& e, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx);

  const std::string& getName() const noexcept { return name_; }


private:
  TypePtr type_;  // TODO Have a reference here???
  std::string name_;
  ExprPtr e_;
};


/********
 * Expr *
 ********/

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
  RSHIFT,
  EQ,
  NEQ,
  GT,
  LT,
  GTE,
  LTE,
  AND,
  OR
};


class ConstInt : public Expr {
public:
  constexpr explicit ConstInt(int n, size_t line) : Expr(line), n_(n) {}
  ExprType getType() const noexcept override { return ExprType::CONST_INT; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
  int n_;
};


class ConstBool : public Expr {
public:
  constexpr explicit ConstBool(bool b, size_t line) : Expr(line), b_(b) {}
  ExprType getType() const noexcept override { return ExprType::CONST_BOOL; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
  bool b_;
};


/* Variable in the program */
class Var : public Expr {
public:
  Var(std::string_view name, size_t line);
  ExprType getType() const noexcept override { return ExprType::VAR; }
  ExprInfo toImExpr(Ctx& ctx) override;

  const std::string& getName() const noexcept { return name_; }

private:
  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp, size_t line);
  ExprType getType() const noexcept override { return ExprType::UNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      Ctx& ctx) override;

private:
  ExprPtr e_;
  UOp uOp_;
};


class BinaryOp : public Expr {
public:
  BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
  ExprType getType() const noexcept override { return ExprType::BINARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      Ctx& ctx) override;

  const ExprPtr& getExpr1() const noexcept { return e1_; }
  const ExprPtr& getExpr2() const noexcept { return e2_; }
  BOp getBOp() const noexcept { return bOp_; }

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
      Ctx& ctx);
  void asBoolOr(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      Ctx& ctx);
  void asBoolXor(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      Ctx& ctx);

  ExprPtr e1_;
  ExprPtr e2_;
  BOp bOp_;
};


class TernaryOp : public Expr {
public:
  TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2);
  ExprType getType() const noexcept override { return ExprType::TERNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
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
  ExprType getType() const noexcept override { return ExprType::CALL_EXPR; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
  std::vector<std::string> qualifiers_;
  std::string name_;
  std::vector<ExprPtr> params_;
};


class NewArray : public Expr {
public:
  NewArray(TypePtr&& type, size_t numElems, size_t line);
  ExprType getType() const noexcept override { return ExprType::NEW_ARRAY; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
  TypePtr type_;
  size_t numElems_;
};


class ArrayAccess : public Expr {
public:
  ArrayAccess(ExprPtr&& expr, size_t index, size_t line);
  ExprType getType() const noexcept override { return ExprType::ARRAY_ACCESS; }
  ExprInfo toImExpr(Ctx& ctx) override;

private:
  ExprPtr expr_;
  size_t index_;
};


std::string newLabel();
std::optional<std::tuple<std::string, std::vector<im::ExprPtr>, TypePtr>>
argsToImExprs(
    const std::vector<std::string>& qualifiers,
    const std::string& fnName,
    const std::vector<ExprPtr>& params,
    size_t line,
    Ctx& ctx);

}  // namespace language


#endif  // LANGUAGE_HPP
