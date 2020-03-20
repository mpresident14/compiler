#ifndef LR1_GRAMMAR_HPP
#define LR1_GRAMMAR_HPP

#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>


/***********
 * GRAMMAR *
 ***********/

/* Terminals and nonterminals in the grammar */
enum class Symbol { S, EXPR, STARTTOKENS, PLUS, STAR, INT, EPSILON };
inline std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::S:
      out << "S";
      break;
    case Symbol::EXPR:
      out << "EXPR";
      break;
    case Symbol::STARTTOKENS:
      out << "STARTTOKENS";
      break;
    case Symbol::PLUS:
      out << "PLUS";
      break;
    case Symbol::STAR:
      out << "STAR";
      break;
    case Symbol::INT:
      out << "INT";
      break;
    case Symbol::EPSILON:
      out << "EPSILON";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { SCONC, EINT, EPLUS, ETIMES, NONE };
inline std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::SCONC:
      out << "SEXPR";
      break;
    case Concrete::EINT:
      out << "EINT";
      break;
    case Concrete::EPLUS:
      out << "EPLUS";
      break;
    case Concrete::ETIMES:
      out << "ETIMES";
      break;
    case Concrete::NONE:
      out << "NONE";
      break;
  }
  return out;
}


/*********
 * UTILS *
 *********/

constexpr Symbol concreteToSymbol[] = {Symbol::S, Symbol::EXPR, Symbol::EXPR, Symbol::EXPR};

constexpr Symbol toSymbol(Concrete concrete) {
  return concreteToSymbol[static_cast<int>(concrete)];
}
constexpr int toInt(Symbol symbol) { return static_cast<int>(symbol); }
constexpr Symbol toSymbol(int i) { return static_cast<Symbol>(i); }
constexpr int toIntTokenOffset(Symbol symbol) {
  return toInt(symbol) - toInt(Symbol::STARTTOKENS) - 1;
}
constexpr Symbol toTokenOffset(int i) {
  return static_cast<Symbol>(i + toInt(Symbol::STARTTOKENS) + 1);
}
constexpr bool isToken(Symbol symbol) { return toInt(symbol) > toInt(Symbol::STARTTOKENS); }
constexpr bool isVariable(Symbol symbol) { return !isToken(symbol); }

constexpr size_t numVariables = toInt(Symbol::STARTTOKENS);
constexpr size_t numTokens = toInt(Symbol::EPSILON) - toInt(Symbol::STARTTOKENS) - 1;
constexpr size_t numSymbols = toInt(Symbol::EPSILON);

using BitSetVars = std::bitset<numVariables>;
using BitSetToks = std::bitset<numTokens>;
using BitRef = BitSetVars::reference;

/* For printing only */
std::vector<Symbol> toVector(BitSetToks tokSet) {
  std::vector<Symbol> v;
  for (size_t i = 0; i < numTokens; ++i) {
    if (tokSet[i]) {
      v.push_back(toTokenOffset(i));
    }
  }
  return v;
}

/********************************
 * ASSOCIATIVITY AND PRECEDENCE *
 ********************************/
enum class Associativity {LEFT, RIGHT, NON, UNSPECIFIED};
/* 0 means unspecified precedence */
constexpr size_t tokenPrecedence[] = {1 /* PLUS */, 2 /* STAR */, 0 /* INT */};
constexpr Associativity tokenAssoc[] = {Associativity::LEFT /* PLUS */, Associativity::LEFT /* STAR */, Associativity::NON /* INT */};
constexpr size_t getPrecedence(Symbol symbol) { return tokenPrecedence[toIntTokenOffset(symbol)]; }
constexpr Associativity getAssociativity(Symbol symbol) { return tokenAssoc[toIntTokenOffset(symbol)]; }

/***********
 * OBJECTS *
 ***********/

/* Tokens with data */
struct Int {
  Int(const std::string& str) : i_(atoi(str.c_str())) {}
  operator int() const { return i_; }
  int i_;
};

// NOTE: getType() not required for parsing, but helpful for client
// TODO: Perhaps pass an argument to the generator that uses existing
// classes or creates them for you


/* Expr */
struct Expr {
  virtual ~Expr(){};
  virtual Concrete getType() const = 0;
  virtual int eval() const = 0;
};

struct EInt : Expr {
  EInt(int i) : i_(i) {}
  Concrete getType() const override { return Concrete::EINT; }
  int eval() const override { return i_; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~EPlus() { delete e1_; delete e2_; }
  Concrete getType() const override { return Concrete::EPLUS; }
  int eval() const override { return e1_->eval() + e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};

struct ETimes : Expr {
  ETimes(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~ETimes() { delete e1_; delete e2_; }
  Concrete getType() const override { return Concrete::EPLUS; }
  int eval() const override { return e1_->eval() * e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};

const Symbol ROOT_SYM = Symbol::EXPR;
using ROOT_TYPE = Expr;


/* S
 * We insert this into the grammar so that we guarantee the root symbol does
 * not appear on any right hand sides of rules. This way, we know the initial
 * lookahead set is empty.
 * */
struct Start {
  Start(ROOT_TYPE* r) : r_(r) {}
  // No deleter since we return the encapsulated pointer
  Concrete getType() const { return Concrete::SCONC; }
  ROOT_TYPE* r_;
};


/*****************************
 *   CONSTRUCTION/DELETION   *
 *****************************/

struct StackObj {
  // No deleter since pointers will be passed to the client
  void* obj;
  Symbol symbol;
  Concrete type;

  void deleteObj() const noexcept;
};

void StackObj::deleteObj() const noexcept {
  switch (symbol) {
    case Symbol::INT:
      delete (Int*)obj;
      break;
    case Symbol::EXPR:
      delete (Expr*)obj;
      break;
    default:
      return;
  }
}

// TODO: Remove throw and make noexcept when done
StackObj construct(Concrete type, StackObj* args) {
  switch (type) {
    case Concrete::EINT:
      return StackObj{new EInt(*(Int*)args[0].obj), toSymbol(type), type};
    case Concrete::EPLUS:
      return StackObj{new EPlus((Expr*)args[0].obj, (Expr*)args[2].obj), toSymbol(type), type};
    case Concrete::ETIMES:
      return StackObj{new ETimes((Expr*)args[0].obj, (Expr*)args[2].obj), toSymbol(type), type};
    case Concrete::SCONC:
      return StackObj{new Start((Expr*) args[0].obj), toSymbol(type), type};
    default:
      throw std::invalid_argument("Can't construct. Out of options.");
  }
}

#include "rules.hpp"

/* LR0 Grammar */
const Grammar GRAMMAR = {
    {Symbol::S,
        {
            GrammarRule{Concrete::SCONC, {ROOT_SYM}}
        }},
    {Symbol::EXPR,
        {
            GrammarRule{Concrete::EINT, {Symbol::INT}},
            GrammarRule{Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}},
            GrammarRule{Concrete::ETIMES, {Symbol::EXPR, Symbol::STAR, Symbol::EXPR}},
        },
    }};

#endif
