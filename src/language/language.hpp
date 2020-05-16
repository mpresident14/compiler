#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "src/intermediate/intermediate.hpp"

#include <vector>
#include <memory>
#include <string>

namespace language {


class Expr {
public:
  virtual ~Expr() {}
  virtual im::ExprPtr toImExpr() const = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

enum class UOp { NEG, NOT };
enum class BOp { PLUS, MUL, MINUS, DIV, EQ, NEQ, GT, LT, GTE, LTE, AND, OR, XOR};


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

private:
  std::string name_;
};


class UnaryOp : public Expr {
public:
  UnaryOp(ExprPtr&& e, UOp uOp);
  im::ExprPtr toImExpr() const override;

private:
  ExprPtr e_;
  UOp uOp_;
};


class BinaryOp : public Expr {
public:
  BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp);
  im::ExprPtr toImExpr() const override;

private:
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
