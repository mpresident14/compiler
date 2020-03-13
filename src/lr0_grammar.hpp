#ifndef SLR1_GRAMMAR_HPP
#define SLR1_GRAMMAR_HPP

#include <iostream>
#include <cstddef>
#include <string>

/* Terminals and nonterminals in the grammar */
enum class Symbol { PLUS, DOLLAR, INT, EXPR, S };
std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::PLUS:
      out << "PLUS";
      break;
    case Symbol::DOLLAR:
      out << "DOLLAR";
      break;
    case Symbol::INT:
      out << "INT";
      break;
    case Symbol::EXPR:
      out << "EXPR";
      break;
    case Symbol::S:
      out << "S";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { EINT, EPLUS, SEXPR, NONE };
std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::EINT:
      out << "EINT";
      break;
    case Concrete::EPLUS:
      out << "EPLUS";
      break;
    case Concrete::SEXPR:
      out << "SEXPR";
      break;
    case Concrete::NONE:
      out << "NONE";
      break;
  }
  return out;
}

/***********
 * OBJECTS *
 ***********/

struct Obj {
  virtual ~Obj() {}
  virtual Symbol getSymbol() = 0;
  virtual bool isToken() = 0;
};

struct TokenObj : Obj {
  // TODO: Remove isToken() method
  bool isToken() override { return true; }
};

struct VariableObj : Obj {
  bool isToken() override { return false; }
  virtual Concrete getType() = 0;
};

/* Tokens */
struct Plus : TokenObj {
  Symbol getSymbol() override { return Symbol::PLUS; }
};
struct Dollar : TokenObj {
  Symbol getSymbol() override { return Symbol::DOLLAR; }
};
// TODO: Not sure about how to handle tokens with info
struct Int : TokenObj {
  Int(const std::string& str) : i_(atoi(str.c_str())) {}
  Symbol getSymbol() override { return Symbol::INT; }
  operator int() const { return i_; }
  int i_;
};

/* Expr */
struct Expr : VariableObj {
  virtual ~Expr(){};
  Symbol getSymbol() override { return Symbol::EXPR; }
};

struct EInt : Expr {
  EInt(Obj* i) : i_(*(Int*) i) {}
  Concrete getType() override { return Concrete::EINT; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Obj* e1, Obj* e2) : e1_((Expr*) e1), e2_((Expr*) e2) {}
  ~EPlus() {
    delete e1_;
    delete e2_;
  }
  Concrete getType() override { return Concrete::EPLUS; }
  Expr* e1_;
  Expr* e2_;
};

/* Start */
struct Start : VariableObj {
  virtual ~Start() {}
  Symbol getSymbol() override { return Symbol::S; }
};

struct SExpr : Start {
  SExpr(Obj* e1) : e1_((Expr*) e1) {}
  ~SExpr() { delete e1_; }
  Concrete getType() override { return Concrete::SEXPR; }
  Expr* e1_;
};

Obj* construct(Concrete type, Obj** args) {
  switch (type) {
    case Concrete::EINT: return new EInt(args[0]);
    case Concrete::EPLUS: return new EPlus(args[0], args[2]);
    case Concrete::SEXPR: return new SExpr(args[0]);
    default: throw std::invalid_argument("Out of options.");
  }
}

#endif
