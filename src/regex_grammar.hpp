#ifndef REGEX_GRAMMAR_HPP
#define REGEX_GRAMMAR_HPP

#include "regex.hpp"

#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>


/***********
 * GRAMMAR *
 ***********/

/* Regex {Regex}
 * Regex := Alts  { Alt($0) }
 *        | CHAR  { Character($0) }
 *
 * Alts {RegexVector}
 * Alts := Regex BAR Regex { RegexVector($0, $2) }
 *       | Alts BAR Regex  { RegexVector($0, $2) }
 *
 *
 * */

/* Terminals and nonterminals in the grammar */
enum class Symbol { S, REGEX, ALTS, STARTTOKENS, BAR, CHAR, EPSILON };
/* The concrete types that symbols in the grammar can be */
enum class Concrete { SCONC, RALT, RCHAR, AREGEX, AALT, NONE };
enum class Associativity {LEFT, RIGHT, NON, UNSPECIFIED};
/* 0 means unspecified precedence */
constexpr size_t tokenPrecedence[] = {8, 0};
constexpr Associativity tokenAssoc[] = {Associativity::LEFT, Associativity::UNSPECIFIED};
constexpr Symbol concreteToSymbol[] = {Symbol::S, Symbol::REGEX, Symbol::REGEX, Symbol::ALTS, Symbol::ALTS};

inline std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::S:
      out << "S";
      break;
    case Symbol::REGEX:
      out << "REGEX";
      break;
    case Symbol::ALTS:
      out << "ALTS";
      break;
    case Symbol::STARTTOKENS:
      out << "STARTTOKENS";
      break;
    case Symbol::BAR:
      out << "BAR";
      break;
    case Symbol::CHAR:
      out << "CHAR";
      break;
    case Symbol::EPSILON:
      out << "EPSILON";
      break;
  }
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::SCONC:
      out << "SCONC";
      break;
    case Concrete::RALT:
      out << "RALT";
      break;
    case Concrete::RCHAR:
      out << "RCHAR";
      break;
    case Concrete::AREGEX:
      out << "AREGEX";
      break;
    case Concrete::AALT:
      out << "AALT";
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
constexpr size_t getPrecedence(Symbol symbol) { return tokenPrecedence[toIntTokenOffset(symbol)]; }
constexpr Associativity getAssociativity(Symbol symbol) { return tokenAssoc[toIntTokenOffset(symbol)]; }

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


/***********
 * OBJECTS *
 ***********/


const Symbol ROOT_SYM = Symbol::REGEX;
using ROOT_TYPE = Regex;


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
    case Symbol::REGEX:
      delete (Regex*)obj;
      break;
    case Symbol::ALTS:
      delete (RegexVector*)obj;
      break;
    case Symbol::CHAR:
      delete (char*)obj;
      break;
    default:
      return;
  }
}

// TODO: Remove throw and make noexcept when done
void* constructObj(Concrete type, StackObj* args) {
  switch (type) {
    case Concrete::RALT:
      return new Alt((RegexVector*)args[0].obj);
    case Concrete::RCHAR:
      return new Character(*(char*)args[0].obj);
    case Concrete::AREGEX:
      return new RegexVector((Regex*)args[0].obj, (Regex*)args[2].obj);
    case Concrete::AALT:
      return new RegexVector((RegexVector*)args[0].obj, (Regex*)args[2].obj);
    case Concrete::SCONC:
      return new Start((ROOT_TYPE*) args[0].obj);
    default:
      throw std::invalid_argument("Can't construct. Out of options.");
  }
}

StackObj construct(Concrete type, StackObj* args) {
  return StackObj{constructObj(type, args), toSymbol(type), type};
}

#include "rules.hpp"

/* LR0 Grammar */
const Grammar GRAMMAR = {
    {Symbol::S,
        {
            GrammarRule{Concrete::SCONC, {ROOT_SYM}}
        }},
    {Symbol::REGEX,
        {
            GrammarRule{Concrete::RALT, {Symbol::ALTS}},
            GrammarRule{Concrete::RCHAR, {Symbol::CHAR}},
        }},
    {Symbol::ALTS,
        {
            GrammarRule{Concrete::AREGEX, {Symbol::REGEX, Symbol::BAR, Symbol::REGEX}},
            GrammarRule{Concrete::AALT, {Symbol::ALTS, Symbol::BAR, Symbol::REGEX}},
        }}
};

#endif
