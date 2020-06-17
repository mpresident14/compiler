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
  CONST_CHAR,
  CONST_BOOL,
  VAR,
  TEMP,
  UNARY_OP,
  BINARY_OP,
  TERNARY_OP,
  CALL_EXPR,
  CAST,
  NEW_ARRAY,
  ARRAY_ACCESS,
  MEMBER_ACCESS
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
  constexpr Expr(size_t line) : line_(line) {}
  virtual ~Expr() {}
  virtual ExprType getType() const noexcept = 0;

  virtual ExprInfo toImExpr(Ctx& ctx) const = 0;
  /* If this typechecks to a bool, add statements to jump to ifTrue it
   * evaluates to true and ifFalse if it evaluates to false. If flipEquiv
   * is true, it will evaluate the opposite operation and jump to the
   * opposite labels. This allows us to eliminate consecutive jumps */
  virtual void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const;
  virtual std::unique_ptr<Expr> clone() const = 0;

  template <typename F>
  ExprInfo toImExprAssert(F&& condFn, std::string_view errMsg, Ctx& ctx) const;
  im::ExprPtr toImExprAssert(const Type& type, Ctx& ctx) const;
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
      std::unordered_map<std::string, std::unique_ptr<Program>>&
          initializedProgs,
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
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;
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
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> ifE_;
  StmtPtr elseE_;
};


class While : public Stmt {
public:
  While(ExprPtr&& boolE, std::unique_ptr<Block> body, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr boolE_;
  std::unique_ptr<Block> body_;
};


class ExprStmt : public Stmt {
public:
  ExprStmt(ExprPtr expr, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr expr_;
};


class Return : public Stmt {
public:
  Return(std::optional<ExprPtr>&& retValue, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  std::optional<ExprPtr> retValue_;
};


class Assign : public Stmt {
public:
  Assign(ExprPtr&& lhs, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr lhs_;
  ExprPtr rhs_;
};


class Update : public Stmt {
public:
  Update(ExprPtr&& lhs, BOp bOp, ExprPtr&& rhs);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr lhs_;
  ExprPtr rhs_;
  BOp bOp_;
};

class VarDecl : public Stmt {
public:
  VarDecl(TypePtr&& type, std::string_view name, ExprPtr&& e, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

  const std::string& getName() const noexcept { return name_; }


private:
  TypePtr type_;  // TODO Have a reference here???
  std::string name_;
  ExprPtr e_;
};


class Print : public Stmt {
public:
  Print(ExprPtr&& expr, size_t line);
  void toImStmts(std::vector<im::StmtPtr>& imStmts, Ctx& ctx) override;

private:
  ExprPtr expr_;
};


/********
 * Expr *
 ********/

class ConstInt : public Expr {
public:
  constexpr explicit ConstInt(long n, size_t line) : Expr(line), n_(n) {}
  ExprType getType() const noexcept override { return ExprType::CONST_INT; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;
  constexpr long getInt() const noexcept { return n_; }

private:
  long n_;
};

class ConstChar : public Expr {
public:
  constexpr explicit ConstChar(char c, size_t line) : Expr(line), c_(c) {}
  ExprType getType() const noexcept override { return ExprType::CONST_CHAR; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;
  constexpr char getChar() const noexcept { return c_; }

private:
  char c_;
};


class ConstBool : public Expr {
public:
  constexpr explicit ConstBool(bool b, size_t line) : Expr(line), b_(b) {}
  ExprType getType() const noexcept override { return ExprType::CONST_BOOL; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  bool b_;
};


/* Variable in the program */
class Var : public Expr {
public:
  Var(std::string_view name, size_t line);
  ExprType getType() const noexcept override { return ExprType::VAR; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

  const std::string& getName() const noexcept { return name_; }

private:
  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp, size_t line);
  ExprType getType() const noexcept override { return ExprType::UNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const override;

private:
  ExprPtr e_;
  UOp uOp_;
};


class BinaryOp : public Expr {
public:
  BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
  ExprType getType() const noexcept override { return ExprType::BINARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

  void asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const override;

  const ExprPtr& getExpr1() const noexcept { return e1_; }
  const ExprPtr& getExpr2() const noexcept { return e2_; }
  BOp getBOp() const noexcept { return bOp_; }

private:
  ExprInfo toImExprArith(im::BOp op, Ctx& ctx) const;
  void asBoolComp(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      im::ROp rOp,
      Ctx& ctx) const;
  void asBoolAnd(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const;
  void asBoolOr(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const;
  void asBoolXor(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse,
      bool flipEquiv,
      Ctx& ctx) const;

  ExprPtr e1_;
  ExprPtr e2_;
  BOp bOp_;
};


class TernaryOp : public Expr {
public:
  TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2);
  ExprType getType() const noexcept override { return ExprType::TERNARY_OP; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

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
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  std::vector<std::string> qualifiers_;
  std::string name_;
  std::vector<ExprPtr> params_;
};


class Cast : public Expr {
public:
  Cast(TypePtr&& toType, ExprPtr&& expr, size_t line);
  ExprType getType() const noexcept override { return ExprType::CAST; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  TypePtr toType_;
  ExprPtr expr_;
};


class NewArray : public Expr {
public:
  NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line);
  NewArray(TypePtr&& type, std::vector<ExprPtr>&& elems, size_t line);
  ExprType getType() const noexcept override { return ExprType::NEW_ARRAY; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  ExprInfo toImExprLen(Ctx& ctx) const;
  ExprInfo toImExprElems(Ctx& ctx) const;

  TypePtr type_;
  ExprPtr numElems_;
  std::vector<ExprPtr> elems_;
};


class ArrayAccess : public Expr {
public:
  ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line);
  ExprType getType() const noexcept override { return ExprType::ARRAY_ACCESS; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  ExprPtr arrExpr_;
  ExprPtr index_;
};


class MemberAccess : public Expr {
public:
  MemberAccess(ExprPtr&& objExpr, std::string_view member, size_t line);
  ExprType getType() const noexcept override { return ExprType::MEMBER_ACCESS; }
  ExprInfo toImExpr(Ctx& ctx) const override;
  ExprPtr clone() const override;

private:
  ExprPtr objExpr_;
  std::string member_;
};


/* For my own convenience */
// class InfoHolder : public Expr {
// public:
//   InfoHolder(ExprInfo&& exprInfo, ExprType exprType, size_t line);
//   ExprType getType() const noexcept override { return exprType_; }
//   /* NOTE: This is not actually const (exprInfo_ is moved), do not call
//    * this method more than once (no use in While loops */
//   ExprInfo toImExpr(Ctx& ctx) const override;
//   ExprPtr clone() const override;

// private:
//   mutable ExprInfo exprInfo_;
//   ExprType exprType_;
// };


inline std::string newLabel() {
  static int i = 0;
  return "L" + std::to_string(i++);
}

}  // namespace language


#endif  // LANGUAGE_HPP
