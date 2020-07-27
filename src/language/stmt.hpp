#ifndef STMT_HPP
#define STMT_HPP

#include "src/assembly/assembly.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/expr.hpp"
#include "src/language/typecheck/type.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class Ctx;
namespace language {


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

using StmtPtr = std::unique_ptr<Stmt>;


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

  Update(ExprPtr&& lValue, Expr::BOp bOp, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr lValue_;
  ExprPtr rhs_;
  Expr::BOp bOp_;
};


class Print : public Stmt {
public:
  Print(ExprPtr&& expr, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  ExprPtr expr_;
};


}  // namespace language

#endif  // STMT_HPP
