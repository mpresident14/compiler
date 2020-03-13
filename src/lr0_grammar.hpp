#ifndef LR0_GRAMMAR_HPP
#define LR0_GRAMMAR_HPP

#include <iostream>
#include <cstddef>
#include <string>

/* Terminals and nonterminals in the grammar */
enum class Symbol { PLUS, INT, TERM, EXPR };
std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::PLUS:
      out << "PLUS";
      break;
    case Symbol::INT:
      out << "INT";
      break;
    case Symbol::TERM:
      out << "TERM";
      break;
    case Symbol::EXPR:
      out << "EXPR";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { ETERM, EPLUS, TINT, NONE };
std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::ETERM:
      out << "ETERM";
      break;
    case Concrete::EPLUS:
      out << "EPLUS";
      break;
    case Concrete::TINT:
      out << "TINT";
      break;
    case Concrete::NONE:
      out << "NONE";
      break;
  }
  return out;
}

const Symbol AST_ROOT = Symbol::EXPR;

/***********
 * OBJECTS *
 ***********/

struct Obj {
  virtual ~Obj() {}
  virtual Symbol getSymbol() = 0;
};

struct TokenObj : Obj {};

struct VariableObj : Obj {
  virtual Concrete getType() = 0;
};

/* Tokens */
struct Plus : TokenObj {
  Symbol getSymbol() override { return Symbol::PLUS; }
};

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

struct TInt : Term {
  TInt(Obj* i) : i_(*(Int*) i) {}
  Concrete getType() override { return Concrete::TINT; }
  int i_;
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
