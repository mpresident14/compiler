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

/***********
 * OBJECTS *
 ***********/

// TODO: Association enum and Predence for TokenObjs, just NONE for VariableObjs

struct Obj {
  virtual ~Obj() {}
  virtual Symbol getSymbol() const = 0;
  virtual bool isToken() const = 0;
};

struct TokenObj : Obj {
  bool isToken() const override { return true; };
};

struct VariableObj : Obj {
  virtual Concrete getType() const = 0;
  bool isToken() const override { return false; };
};

/* Tokens */
struct Plus : TokenObj {
  Symbol getSymbol() const override { return Symbol::PLUS; }
};

struct Int : TokenObj {
  Int(const std::string& str) : i_(atoi(str.c_str())) {}
  Symbol getSymbol() const override { return Symbol::INT; }
  operator int() const { return i_; }
  int i_;
};

/* Term */
struct Term : VariableObj {
  virtual ~Term(){};
  Symbol getSymbol() const override { return Symbol::TERM; }
};

struct TInt : Term {
  TInt(Obj* i) : i_(*(Int*) i) {}
  Concrete getType() const override { return Concrete::TINT; }
  int i_;
};

/* Expr */
struct Expr : VariableObj {
  virtual ~Expr(){};
  Symbol getSymbol() const override { return Symbol::EXPR; }
};

struct ETerm : Expr {
  ETerm(Obj* t) : t_((Term*) t) {}
  ~ETerm() { delete t_; }
  Concrete getType() const override { return Concrete::ETERM; }
  Term* t_;
};

struct EPlus : Expr {
  EPlus(Obj* e, Obj* t) : e_((Expr*) e), t_((Term*) t) {}
  ~EPlus() {
    delete e_;
    delete t_;
  }
  Concrete getType() const override { return Concrete::EPLUS; }
  Expr* e_;
  Term* t_;
};


Obj* construct(Concrete type, Obj** args) {
  switch (type) {
    case Concrete::TINT: return new TInt(args[0]);
    case Concrete::ETERM: return new ETerm(args[0]);
    case Concrete::EPLUS: return new EPlus(args[0], args[2]);
    default: throw std::invalid_argument("Out of options.");
  }
}

const Symbol ROOT_SYM = Symbol::EXPR;
using ROOT_TYPE = Expr;

#endif
