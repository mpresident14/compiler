#include "src/language/expr.hpp"

using namespace std;


namespace language {

ExprPtr ConstInt::clone() const { return make_unique<ConstInt>(n_, line_); }

ExprPtr ConstChar::clone() const { return make_unique<ConstChar>(c_, line_); }

ExprPtr ConstBool::clone() const { return make_unique<ConstBool>(b_, line_); }

StrLit::StrLit(std::string_view sv, size_t line) : Expr(line), str_(sv) {}
ExprPtr StrLit::clone() const { return make_unique<StrLit>(str_, line_); }

Var::Var(string_view name, size_t line) : Expr(line), name_(name) {}
ExprPtr Var::clone() const { return make_unique<Var>(name_, line_); }

UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp, size_t line) : Expr(line), e_(move(e)), uOp_(uOp) {}
ExprPtr UnaryOp::clone() const { return make_unique<UnaryOp>(e_->clone(), uOp_, line_); }

BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
    : Expr(e1->line_), e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}
ExprPtr BinaryOp::clone() const { return make_unique<BinaryOp>(e1_->clone(), e2_->clone(), bOp_); }

TernaryOp::TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2)
    : Expr(boolE->line_), boolE_(move(boolE)), e1_(move(e1)), e2_(move(e2)) {}
ExprPtr TernaryOp::clone() const {
  return make_unique<TernaryOp>(boolE_->clone(), e1_->clone(), e2_->clone());
}

Call::Call(
    vector<string>&& qualifiers,
    string_view name,
    vector<ExprPtr>&& params,
    size_t line,
    bool ctorAlloc)
    : Expr(line),
      qualifiers_(move(qualifiers)),
      name_(name),
      params_(move(params)),
      ctorAlloc_(ctorAlloc) {}
ExprPtr Call::clone() const {
  vector<ExprPtr> paramsClone;
  for (const ExprPtr& expr : params_) {
    paramsClone.push_back(expr->clone());
  }
  return make_unique<Call>(
      vector<string>(qualifiers_), name_, move(paramsClone), line_, ctorAlloc_);
}

Cast::Cast(TypePtr&& toType, ExprPtr&& expr, size_t line)
    : Expr(line), toType_(move(toType)), expr_(move(expr)) {}
ExprPtr Cast::clone() const { return make_unique<Cast>(TypePtr(toType_), expr_->clone(), line_); }

NewArray::NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line)
    : Expr(line), type_(move(type)), numElems_(move(numElems)) {}

NewArray::NewArray(TypePtr&& type, vector<ExprPtr>&& elems, size_t line)
    : Expr(line), type_(move(type)), numElems_(nullptr), elems_(move(elems)) {}

ExprPtr NewArray::clone() const {
  vector<ExprPtr> elemsClone;
  for (const ExprPtr& expr : elems_) {
    elemsClone.push_back(expr->clone());
  }
  return make_unique<NewArray>(TypePtr(type_), numElems_ ? numElems_->clone() : nullptr, line_);
}


ArrayAccess::ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line)
    : Expr(line), arrExpr_(move(arrExpr)), index_(move(index)) {}
ExprPtr ArrayAccess::clone() const {
  return make_unique<ArrayAccess>(arrExpr_->clone(), index_->clone(), line_);
}

MemberAccess::MemberAccess(ExprPtr&& objExpr, string_view member)
    : Expr(objExpr->line_), objExpr_(move(objExpr)), member_(member) {}
MemberAccess::MemberAccess(std::string_view varName, string_view member, size_t line)
    : Expr(line), objExpr_(make_unique<Var>(varName, line)), member_(member) {}
ExprPtr MemberAccess::clone() const {
  return make_unique<MemberAccess>(objExpr_->clone(), member_);
}

MethodInvocation::MethodInvocation(
    ExprPtr&& objExpr, std::string_view methodName, std::vector<ExprPtr>&& params, size_t line)
    : Expr(line), objExpr_(move(objExpr)), methodName_(methodName), params_(move(params)) {}
MethodInvocation::MethodInvocation(
    std::string_view ident, std::string_view methodName, std::vector<ExprPtr>&& params, size_t line)
    : Expr(line),
      objExpr_(nullptr),
      ident_(ident),
      methodName_(methodName),
      params_(move(params)) {}
ExprPtr MethodInvocation::clone() const {
  vector<ExprPtr> paramsClone;
  for (const ExprPtr& expr : params_) {
    paramsClone.push_back(expr->clone());
  }
  if (objExpr_) {
    return make_unique<MethodInvocation>(objExpr_->clone(), methodName_, move(paramsClone), line_);
  }
  return make_unique<MethodInvocation>(ident_, methodName_, move(paramsClone), line_);
}

QualifiedInvocation::QualifiedInvocation(
    std::vector<std::string>&& qualifiers,
    std::string_view className,
    std::string_view methodName,
    std::vector<ExprPtr>&& params,
    size_t line)
    : Expr(line),
      classTy_(Class(move(qualifiers), className)),
      methodName_(methodName),
      params_(move(params)) {}
ExprPtr QualifiedInvocation::clone() const {
  vector<ExprPtr> paramsClone;
  for (const ExprPtr& expr : params_) {
    paramsClone.push_back(expr->clone());
  }
  return make_unique<QualifiedInvocation>(
      vector<string>(classTy_.qualifiers),
      classTy_.className,
      methodName_,
      move(paramsClone),
      line_);
}


IncDec::IncDec(ExprPtr&& expr, bool inc, bool pre, size_t line)
    : Expr(line), lValue_(move(expr)), inc_(inc), pre_(pre) {}
ExprPtr IncDec::clone() const { return make_unique<IncDec>(lValue_->clone(), inc_, pre_, line_); }


ImWrapper::ImWrapper(im::ExprPtr&& imExpr, TypePtr type, bool isLValue, size_t line)
    : Expr(line), imExpr_(move(imExpr)), type_(move(type)), isLValue_(isLValue) {}
ExprPtr ImWrapper::clone() const { throw invalid_argument("No clone for ImWrapper"); }

}  // namespace language
