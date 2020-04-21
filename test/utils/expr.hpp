#ifndef EXPR_HPP
#define EXPR_HPP

enum class ExprType { EINT, EPLUS, ETIMES };

/* Expr */
struct Expr {
  virtual ~Expr(){};
  virtual ExprType getType() const = 0;
  virtual int eval() const = 0;
};

struct EInt : Expr {
  EInt(int i) : i_(i) {}
  ExprType getType() const override { return ExprType::EINT; }
  int eval() const override { return i_; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~EPlus() {
    delete e1_;
    delete e2_;
  }
  ExprType getType() const override { return ExprType::EPLUS; }
  int eval() const override { return e1_->eval() + e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};

struct ETimes : Expr {
  ETimes(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~ETimes() {
    delete e1_;
    delete e2_;
  }
  ExprType getType() const override { return ExprType::EPLUS; }
  int eval() const override { return e1_->eval() * e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};

#endif
