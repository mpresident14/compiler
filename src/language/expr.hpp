#ifndef EXPR_HPP
#define EXPR_HPP

#include "src/assembly/assembly.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/typecheck/type.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>


class Ctx;

namespace language {

class Expr {
public:
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

  struct Info {
    im::ExprPtr imExpr;
    TypePtr type;
  };

  constexpr Expr(size_t line) : line_(line) {}
  virtual ~Expr() {}

  /* This and all related functions invalidate the Expr that calls it */
  virtual Info toImExpr(Ctx& ctx) = 0;
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
  Info toImExprAssert(F&& condFn, std::string_view errMsg, Ctx& ctx);
  im::ExprPtr toImExprAssert(const Type& type, Ctx& ctx);

  size_t line_;
};

using ExprPtr = std::unique_ptr<Expr>;

class ConstInt : public Expr {
public:
  constexpr explicit ConstInt(long n, size_t line) : Expr(line), n_(n) {}
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  long n_;
};

class ConstChar : public Expr {
public:
  constexpr explicit ConstChar(char c, size_t line) : Expr(line), c_(c) {}
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  char c_;
};


class ConstBool : public Expr {
public:
  constexpr explicit ConstBool(bool b, size_t line) : Expr(line), b_(b) {}
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  bool b_;
};


class StrLit : public Expr {
public:
  StrLit(std::string_view sv, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  std::string str_;
};


/* Variable in the program */
class Var : public Expr {
public:
  Var(std::string_view name, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp, size_t line);
  Info toImExpr(Ctx& ctx) override;
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
  Info toImExpr(Ctx& ctx) override;
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
  Info toImExprArith(im::BOp op, Ctx& ctx);
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
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr boolE_;
  ExprPtr e1_;
  ExprPtr e2_;
};


class Call : public Expr {
public:
  Call(
      std::vector<std::string>&& qualifiers,
      std::string_view name,
      std::vector<ExprPtr>&& params,
      size_t line,
      bool ctorAlloc = true);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  std::vector<std::string> qualifiers_;
  std::string name_;
  std::vector<ExprPtr> params_;
  bool ctorAlloc_;
};


class Cast : public Expr {
public:
  Cast(TypePtr&& toType, ExprPtr&& expr, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  TypePtr toType_;
  ExprPtr expr_;
};


class NewArray : public Expr {
public:
  NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line);
  NewArray(TypePtr&& type, std::vector<ExprPtr>&& elems, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;


  TypePtr type_;
  ExprPtr numElems_;
  std::vector<ExprPtr> elems_;

private:
  /* Returns the statements to create the array as well as the temp containing
   * the address of the new array */
  std::pair<std::vector<im::StmtPtr>, int> makeArrayStmts(Type& type, ExprPtr&& numElems, Ctx& ctx);
  Info toImExprLen(Ctx& ctx);
  Info toImExprElems(Ctx& ctx);
};


class ArrayAccess : public Expr {
public:
  ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr arrExpr_;
  ExprPtr index_;
};


class MemberAccess : public Expr {
public:
  MemberAccess(ExprPtr&& objExpr, std::string_view member);
  MemberAccess(std::string_view varName, std::string_view member, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr objExpr_;
  std::string member_;
};


class MethodInvocation : public Expr {
public:
  MethodInvocation(
      ExprPtr&& objExpr, std::string_view methodName, std::vector<ExprPtr>&& params, size_t line);
  MethodInvocation(
      std::string_view ident,
      std::string_view methodName,
      std::vector<ExprPtr>&& params,
      size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  ExprPtr objExpr_;
  std::string ident_;
  std::string methodName_;
  std::vector<ExprPtr> params_;

private:
  std::pair<Info, bool> resolveObjExpr(Ctx& ctx);
};


class QualifiedInvocation : public Expr {
public:
  QualifiedInvocation(
      std::vector<std::string>&& qualifiers,
      std::string_view className,
      std::string_view methodName,
      std::vector<ExprPtr>&& params,
      size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  Class classTy_;
  std::string methodName_;
  std::vector<ExprPtr> params_;
};


class IncDec : public Expr {
public:
  IncDec(ExprPtr&& lValue, bool inc, bool pre, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;


  ExprPtr lValue_;
  bool inc_;
  bool pre_;
};


class ImWrapper : public Expr {
public:
  ImWrapper(im::ExprPtr&& imExpr, TypePtr type, size_t line);
  Info toImExpr(Ctx& ctx) override;
  ExprPtr clone() const override;

  im::ExprPtr imExpr_;
  TypePtr type_;
};


}  // namespace language

#endif  // EXPR_HPP
