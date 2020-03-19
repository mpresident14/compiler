#ifndef SLR1_GRAMMAR_HPP
#define SLR1_GRAMMAR_HPP

#include <cstddef>
#include <iostream>
#include <string>

/* Terminals and nonterminals in the grammar */
enum class Symbol { PLUS, DOLLAR, INT, EXPR, STMT };
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
    case Symbol::STMT:
      out << "STMT";
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
};

struct TokenObj : Obj {};

struct VariableObj : Obj {
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
  EInt(Obj* i) : i_(*(Int*)i) {}
  Concrete getType() override { return Concrete::EINT; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Obj* i, Obj* e) : i_(*(Int*)i), e_((Expr*)e) {}
  ~EPlus() { delete e_; }
  Concrete getType() override { return Concrete::EPLUS; }
  int i_;
  Expr* e_;
};

/* Stmt */
struct Stmt : VariableObj {
  virtual ~Stmt() {}
  Symbol getSymbol() override { return Symbol::STMT; }
};

struct RExpr : Stmt {
  RExpr(Obj* e1) : e1_((Expr*)e1) {}
  ~RExpr() { delete e1_; }
  Concrete getType() override { return Concrete::SEXPR; }
  Expr* e1_;
};

Obj* construct(Concrete type, Obj** args) {
  switch (type) {
    case Concrete::EINT:
      return new EInt(args[0]);
    case Concrete::EPLUS:
      return new EPlus(args[0], args[2]);
    case Concrete::SEXPR:
      return new RExpr(args[0]);
    default:
      throw std::invalid_argument("Out of options.");
  }
}

const Symbol ROOT_SYM = Symbol::STMT;
using ROOT_TYPE = Stmt;

#include "rules.hpp"

/* SLR1 Grammar */
const Grammar GRAMMAR = {{ROOT_SYM, {GrammarRule{Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}}}},
    {
        Symbol::EXPR,
        {GrammarRule{Concrete::EINT, {Symbol::INT}},
            GrammarRule{Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}}},
    }};

#endif
