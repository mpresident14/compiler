#include "src/language/language.hpp"

#include <sstream>

using namespace std;


namespace language {

ExprPtr ConstInt::clone() const { return make_unique<ConstInt>(n_, line_); }

ExprPtr ConstChar::clone() const { return make_unique<ConstChar>(c_, line_); }

ExprPtr ConstBool::clone() const { return make_unique<ConstBool>(b_, line_); }

Var::Var(string_view name, size_t line) : Expr(line), name_(name) {}
ExprPtr Var::clone() const { return make_unique<Var>(name_, line_); }

UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp, size_t line)
    : Expr(line), e_(move(e)), uOp_(uOp) {}
ExprPtr UnaryOp::clone() const {
  return make_unique<UnaryOp>(e_->clone(), uOp_, line_);
}

BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
    : Expr(e1->line_), e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}
ExprPtr BinaryOp::clone() const {
  return make_unique<BinaryOp>(e1_->clone(), e2_->clone(), bOp_);
}

TernaryOp::TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2)
    : Expr(boolE->line_), boolE_(move(boolE)), e1_(move(e1)), e2_(move(e2)) {}
ExprPtr TernaryOp::clone() const {
  return make_unique<TernaryOp>(boolE_->clone(), e1_->clone(), e2_->clone());
}

CallExpr::CallExpr(
    vector<string>&& qualifiers,
    string_view name,
    vector<ExprPtr>&& params,
    size_t line)
    : Expr(line),
      qualifiers_(move(qualifiers)),
      name_(name),
      params_(move(params)) {}
ExprPtr CallExpr::clone() const {
  vector<ExprPtr> paramsClone;
  for (const ExprPtr& expr : params_) {
    paramsClone.push_back(expr->clone());
  }
  return make_unique<CallExpr>(
      vector<string>(qualifiers_), name_, move(paramsClone), line_);
}

Cast::Cast(TypePtr&& toType, ExprPtr&& expr, size_t line)
    : Expr(line), toType_(move(toType)), expr_(move(expr)) {}
ExprPtr Cast::clone() const {
  return make_unique<Cast>(TypePtr(toType_), expr_->clone(), line_);
}

NewArray::NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line)
    : Expr(line), type_(move(type)), numElems_(move(numElems)) {}

NewArray::NewArray(TypePtr&& type, vector<ExprPtr>&& elems, size_t line)
    : Expr(line), type_(move(type)), numElems_(nullptr), elems_(move(elems)) {}

ExprPtr NewArray::clone() const {
  vector<ExprPtr> elemsClone;
  for (const ExprPtr& expr : elems_) {
    elemsClone.push_back(expr->clone());
  }
  return make_unique<NewArray>(
      TypePtr(type_), numElems_ ? numElems_->clone() : nullptr, line_);
}


ArrayAccess::ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line)
    : Expr(line), arrExpr_(move(arrExpr)), index_(move(index)) {}
ExprPtr ArrayAccess::clone() const {
  return make_unique<ArrayAccess>(arrExpr_->clone(), index_->clone(), line_);
}

MemberAccess::MemberAccess(ExprPtr&& objExpr, string_view member, size_t line)
    : Expr(line), objExpr_(move(objExpr)), member_(member) {}
ExprPtr MemberAccess::clone() const {
  return make_unique<MemberAccess>(objExpr_->clone(), member_, line_);
}

IncDec::IncDec(ExprPtr&& expr, bool inc, bool pre, size_t line)
    : Expr(line), expr_(move(expr)), inc_(inc), pre_(pre) {}
ExprPtr IncDec::clone() const {
  return make_unique<IncDec>(expr_->clone(), inc_, pre_, line_);
}


ImWrapper::ImWrapper(im::ExprPtr&& imExpr, TypePtr type, size_t line)
    : Expr(line), imExpr_(move(imExpr)), type_(move(type)) {}
ExprPtr ImWrapper::clone() const {
  throw invalid_argument("No clone for ImWrapper");
}

}  // namespace language
