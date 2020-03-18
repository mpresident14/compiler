#ifndef LR0_GRAMMAR_HPP
#define LR0_GRAMMAR_HPP

#include <cstddef>
#include <iostream>
#include <string>

/* Terminals and nonterminals in the grammar */
enum class Symbol { PLUS, INT, ENDTOKENS, TERM, EXPR };
inline std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
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
    case Symbol::ENDTOKENS:
      out << "ENDTOKENS";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { ETERM, EPLUS, TINT, NONE };
inline std::ostream& operator<<(std::ostream& out, const Concrete& type) {
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

const Symbol concreteToSymbol[] = {Symbol::EXPR, Symbol::EXPR, Symbol::TERM};
inline Symbol toSymbol(Concrete concrete) { return concreteToSymbol[static_cast<int>(concrete)]; }

/***********
 * OBJECTS *
 ***********/

// TODO: Association enum and Predence for TokenObjs, just NONE for VariableObjs

/* Tokens */
struct Plus {};

struct Int {
  Int(const std::string& str) : i_(atoi(str.c_str())) {}
  operator int() const { return i_; }
  int i_;
};

// NOTE: getType() not required for parsing, but helpful for client
// TODO: Perhaps pass an argument to the generator that uses existing
// classes or creates them for you

/* Term */
struct Term {
  virtual ~Term(){};
  virtual Concrete getType() const = 0;
};

struct TInt : Term {
  TInt(int i) : i_(i) {}
  Concrete getType() const override { return Concrete::TINT; }
  int i_;
};

/* Expr */
struct Expr {
  virtual ~Expr(){};
  virtual Concrete getType() const = 0;
};

struct ETerm : Expr {
  ETerm(Term* t) : t_(t) {}
  ~ETerm() { delete t_; }
  Concrete getType() const override { return Concrete::ETERM; }
  Term* t_;
};

struct EPlus : Expr {
  EPlus(Expr* e, Term* t) : e_(e), t_(t) {}
  ~EPlus() {
    delete e_;
    delete t_;
  }
  Concrete getType() const override { return Concrete::EPLUS; }
  Expr* e_;
  Term* t_;
};

const Symbol ROOT_SYM = Symbol::EXPR;
using ROOT_TYPE = Expr;


#include "rules.hpp"

/* LR0 Grammar */
const Grammar GRAMMAR = {
    {ROOT_SYM,
        {
            Rule{Concrete::ETERM, {Symbol::TERM}, 0},
            Rule{Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 0},
        }
    },
    {
        Symbol::TERM,
        {
            Rule{Concrete::TINT, {Symbol::INT}, 0},
        },
    }
};

#endif
