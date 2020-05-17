#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "src/intermediate/intermediate.hpp"
#include "src/language/typecheck/type.hpp"

#include <memory>
#include <string>
#include <vector>

namespace language {


  class Decl {
  public:
    virtual ~Decl() {}
    virtual std::vector<im::StmtPtr> toImStmts() const = 0;
  };

  class Stmt {
  public:
    virtual ~Stmt() {}
    /* If the statement typechecks, generate the corresponding intermediate
     * statements */
    virtual void toImStmts(std::vector<im::StmtPtr>& imStmts) = 0;
  };

  enum class ExprType {
    CONST_INT,
    CONST_BOOL,
    VAR,
    TEMP,
    UNARY_OP,
    BINARY_OP,
    TERNARY_OP,
    CALL
  };

  struct ExprInfo {
    im::ExprPtr imExpr;
    Type type;
  };

  class Expr {
  public:
    virtual ~Expr() {}
    virtual ExprType getType() const noexcept = 0;

    virtual ExprInfo toImExpr() = 0;
    virtual im::ExprPtr toImExprAssert(const Type& type);
    /* If this typechecks to a bool, add statements to jump to ifTrue it
     * evaluates to true and ifFalse if it evaluates to false. */
    virtual void
    asBool(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse);
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
    If(ExprPtr&& boolE, StmtPtr&& ifE, StmtPtr&& elseE);
    void toImStmts(std::vector<im::StmtPtr>& imStmts);

  private:
    ExprPtr boolE_;
    StmtPtr ifE_;
    StmtPtr elseE_;
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
    VarDecl(const Type& type, const std::string& name, ExprPtr&& e);
    void toImStmts(std::vector<im::StmtPtr>& imStmts);

    const std::string& getName() const noexcept { return name_; }


  private:
    Type type_;  // TODO Have a reference here???
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
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
    AND,
    OR /* TODO: , BIT_AND, BIT_OR, XOR */
  };


  class ConstInt : public Expr {
  public:
    constexpr explicit ConstInt(int n) : n_(n) {}
    ExprType getType() const noexcept override { return ExprType::CONST_INT; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;

  private:
    int n_;
  };


  class ConstBool : public Expr {
  public:
    constexpr explicit ConstBool(bool b) : b_(b) {}
    ExprType getType() const noexcept override { return ExprType::CONST_BOOL; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;

  private:
    bool b_;
  };


  /* Variable in the program */
  class Var : public Expr {
  public:
    Var(const std::string& name);
    ExprType getType() const noexcept override { return ExprType::VAR; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;

    const std::string& getName() const noexcept { return name_; }

  private:
    std::string name_;
  };


  /* Temporary created by me for convenience. */
  class Temp : public Expr {
  public:
    explicit constexpr Temp(int temp) : temp_(temp) {}
    ExprType getType() const noexcept override { return ExprType::TEMP; }
    ExprInfo toImExpr() override;


    const std::string& getName() const noexcept { return temp_; }

  private:
    int temp_;
  };


  class UnaryOp : public Expr {
  public:
    UnaryOp(ExprPtr&& e, UOp uOp);
    ExprType getType() const noexcept override { return ExprType::UNARY_OP; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;
    void asBool(
        std::vector<im::StmtPtr>& imStmts,
        Label* ifTrue,
        Label* ifFalse) override;

  private:
    ExprPtr e_;
    UOp uOp_;
  };


  class BinaryOp : public Expr {
  public:
    BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
    ExprType getType() const noexcept override { return ExprType::BINARY_OP; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;
    void asBool(
        std::vector<im::StmtPtr>& imStmts,
        Label* ifTrue,
        Label* ifFalse) override;

    const ExprPtr& getExpr1() const noexcept { return e1_; }
    const ExprPtr& getExpr2() const noexcept { return e2_; }
    BOp getBOp() const noexcept { return bOp_; }

  private:
    ExprInfo toImExprArith(im::BOp op);
    void asBoolComp(
        std::vector<im::StmtPtr>& imStmts,
        Label* ifTrue,
        Label* ifFalse,
        im::ROp rOp);
    void
    asBoolAnd(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse);
    void
    asBoolOr(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse);
    ExprPtr e1_;
    ExprPtr e2_;
    BOp bOp_;
  };


  class TernaryOp : public Expr {
  public:
    TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2);
    ExprType getType() const noexcept override { return ExprType::TERNARY_OP; }
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;

  private:
    ExprPtr boolE_;
    ExprPtr e1_;
    ExprPtr e2_;
  };


  class CallExpr : public Expr {
  public:
    CallExpr(const std::string& name, std::vector<ExprPtr>&& params);
    ExprInfo toImExpr() override;
    im::ExprPtr toImExprAssert(const Type& type) override;

  private:
    std::string name_;
    std::vector<ExprPtr> params_;
  };

}  // namespace language

#endif  // LANGUAGE_HPP
