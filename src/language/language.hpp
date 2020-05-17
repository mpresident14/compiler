#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "src/intermediate/intermediate.hpp"

#include <vector>
#include <memory>
#include <string>

namespace language {


class Decl {
public:
  virtual ~Decl() {}
  virtual std::vector<im::StmtPtr> toImStmts() const = 0;
};

class Stmt {
public:
  virtual ~Stmt() {}
  /* If the statement typechecks, generate the corresponding intermediate statements */
  virtual void toImStmts(std::vector<im::StmtPtr>& imStmts) = 0;
  virtual void assertOk() const = 0;
};

enum class ExprType {CONST_INT, CONST_BOOL, VAR, UOP, BOP, CALL};

struct ExprInfo {
  im::ExprPtr imExpr;
  TypePtr type;
};

class Expr {
public:
  virtual ~Expr() {}
  virtual ExprType getType() const noexcept = 0;

  virtual ExprInfo toImExpr() const = 0;
  virtual im::ExprPtr toImExprAssert(const TypePtr& type) const = 0;
  /* If this typechecks to a bool, add statements to jump to ifTrue it
   * evaluates to true and ifFalse if it evaluates to false. */
  virtual void asBool(vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const;
};


using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

/********
 * Stmt *
 ********/

class Block : public Stmt {
public:
  Block(std::vector<StmtPtr>&& stmts);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  std::vector<StmtPtr> stmts_;
};


class If : public Stmt {
public:
  If(ExprPtr&& boolE, std::unique_ptr<Block>&& ifE, std::unique_ptr<Block>&& elseE);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> ifE_;
  std::unique_ptr<Block> elseE_;
};


class While : public Stmt {
public:
  While(ExprPtr&& boolE, std::unique_ptr<Block> body);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> body_;
};

class CallExpr;
class CallStmt : public Stmt {
public:
  CallStmt(const std::string& name, std::vector<ExprPtr>&& params);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  std::string name_;
  std::vector<ExprPtr> params_;
};


class Return : public Stmt {
public:
  Return(std::optional<ExprPtr>&& retValue);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  std::optional<ExprPtr> retValue_;
};


class Assign : public Stmt {
public:
  Assign(ExprPtr&& lhs, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);

private:
  ExprPtr lhs_;
  ExprPtr rhs_;
};


class VarDecl : public Stmt {
public:
  VarDecl(TypePtr&& type, const std::string& name, ExprPtr&& e);
  void toImStmts(std::vector<im::StmtPtr>& imStmts);


private:
  TypePtr type_;
  std::string name_;
  ExprPtr e_;
};


/********
 * Expr *
 ********/

enum class UOp { NEG, NOT };
enum class BOp { PLUS, MUL, MINUS, DIV, EQ, NEQ, GT, LT, GTE, LTE, AND, OR, BIT_AND, BIT_OR, XOR};


class ConstInt : public Expr {
public:
  constexpr explicit ConstInt(int n) : n_(n) {}
  im::ExprPtr toImExpr() const override;

private:
  int n_;
};


class ConstBool : public Expr {
public:
  constexpr explicit ConstBool(bool b) : b_(b) {}
  im::ExprPtr toImExpr() const override;

private:
  bool b_;
};


class Var : public Expr {
public:
  Var(const std::string& name);
  im::ExprPtr toImExpr() const override;

  const std::string& getName() const noexcept { return name_; }

private:
  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp);
  virtual ExprType getType() const noexcept override { return ExprType::UOP; };
  virtual ExprInfo toImExpr() const override;
  virtual im::ExprPtr toImExprAssert(TypePtr&& type) const override;
  void asBool(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const override;

private:
  ExprPtr e_;
  UOp uOp_;
};


class BinaryOp : public Expr {
public:
  BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
  im::ExprPtr toImExpr() const override;
  void asBool(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const override;

  const ExprPtr& getExpr1() const noexcept { return e1_; }
  const ExprPtr& getExpr2() const noexcept { return e2_; }
  BOp getBOp() const noexcept { return bOp_; }

private:
  void asBoolComp(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse, im::ROp rOp) const;
  void asBoolAnd(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const;
  void asBoolOr(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const;
  ExprPtr e1_;
  ExprPtr e2_;
  BOp bOp_;
};


class CallExpr : public Expr {
public:
  CallExpr(const std::string& name, std::vector<ExprPtr>&& params);
  im::ExprPtr toImExpr() const override;

private:
  std::string name_;
  std::vector<ExprPtr> params_;
};

}

#endif // LANGUAGE_HPP
