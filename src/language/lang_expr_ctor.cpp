#include "src/language/language.hpp"

#include <sstream>

using namespace std;


namespace language {

Var::Var(string_view name, size_t line) : Expr(line), name_(name) {}

UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp, size_t line)
    : Expr(line), e_(move(e)), uOp_(uOp) {}

BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
    : Expr(e1->getLine()), e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}

TernaryOp::TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2)
    : Expr(boolE->getLine()),
      boolE_(move(boolE)),
      e1_(move(e1)),
      e2_(move(e2)) {}

CallExpr::CallExpr(
    vector<string>&& qualifiers,
    string_view name,
    vector<ExprPtr>&& params,
    size_t line)
    : Expr(line),
      qualifiers_(move(qualifiers)),
      name_(name),
      params_(move(params)) {}

Cast::Cast(TypePtr&& toType, ExprPtr&& expr, size_t line)
    : Expr(line), toType_(move(toType)), expr_(move(expr)) {}

NewArray::NewArray(TypePtr&& type, ExprPtr&& numElems, size_t line)
    : Expr(line), type_(move(type)), numElems_(move(numElems)) {}

NewArray::NewArray(TypePtr&& type, vector<ExprPtr>&& elems, size_t line)
    : Expr(line), type_(move(type)), numElems_(nullptr), elems_(move(elems)) {}

ArrayAccess::ArrayAccess(ExprPtr&& arrExpr, ExprPtr&& index, size_t line)
    : Expr(line), arrExpr_(move(arrExpr)), index_(move(index)) {}

MemberAccess::MemberAccess(
    ExprPtr&& objExpr,
    std::string_view member,
    size_t line)
    : Expr(line), objExpr_(move(objExpr)), member_(member) {}

InfoHolder::InfoHolder(ExprInfo&& exprInfo, ExprType exprType, size_t line)
    : Expr(line), exprInfo_(move(exprInfo)), exprType_(exprType) {}

}  // namespace language
