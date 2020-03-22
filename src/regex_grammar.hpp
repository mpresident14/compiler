#ifndef REGEX_GRAMMAR_HPP
#define REGEX_GRAMMAR_HPP

#include "regex.hpp"

#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <cstdlib>


/***********
 * GRAMMAR *
 ***********/

/* CHAR { char :: str[0] } <DELETER OPTIONAL>
 *
 *
 * Regex { Regex* } { delete $; }
 * Regex := Alts                             { new Alt(std::move($0)) }
 *        | Concats                          { new Concat(std::move($0)) }
 *        | Regex STAR                       { new Star($0) }
 *        | CARET Regex                      { new Not($1) }
 *        | LBRACKET CHAR DASH CHAR RBRACKET { new Range($1, $3) }
 *        | LPAREN Regex RPAREN              { $1 }
 *        | CHAR                             { new Character($0) }
 *
 * Alts { RegexVector }
 * Alts := Regex BAR Regex { RegexVector($0, $2) }
 *       | Alts BAR Regex  { RegexVector(std::move($0), $2) }
 *
 * Concats { RegexVector* }
 * Concats := Regex Regex   { RegexVector($0, $1) }
 *          | Concats Regex { RegexVector(std::move($0), $1) }
 *
 *
 * */

/* Terminals and nonterminals in the grammar */
enum class Symbol {
  S,
  REGEX,
  ALTS,
  CONCATS,
  STARTTOKENS,
  BAR,
  STAR,
  CARET,
  LBRACKET,
  RBRACKET,
  LPAREN,
  RPAREN,
  DASH,
  CHAR,
  EPSILON
};
/* The concrete types that symbols in the grammar can be */
enum class Concrete {
  SCONC,
  RALT,
  RCONCAT,
  RSTAR,
  RNOT,
  RRANGE,
  RGROUP,
  RCHAR,
  AREGEX,
  AALT,
  CREGEX,
  CCONCAT,
  NONE
};
enum class Associativity { LEFT, RIGHT, NON, UNSPECIFIED };
/* 0 means unspecified precedence */
constexpr size_t overridePrecedence[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4 };
constexpr size_t tokenPrecedence[] = { 1, 5, 3, 7, 7, 8, 8, 0, 4 };
constexpr Associativity tokenAssoc[] = { Associativity::LEFT,
  Associativity::LEFT,
  Associativity::LEFT,
  Associativity::NON,
  Associativity::NON,
  Associativity::NON,
  Associativity::NON,
  Associativity::NON,
  Associativity::LEFT };
constexpr Symbol concreteToSymbol[] = { Symbol::S,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::REGEX,
  Symbol::ALTS,
  Symbol::ALTS,
  Symbol::CONCATS,
  Symbol::CONCATS };

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
    case Symbol::CONCATS:
      out << "CONCATS";
      break;
    case Symbol::STARTTOKENS:
      out << "STARTTOKENS";
      break;
    case Symbol::BAR:
      out << "BAR";
      break;
    case Symbol::STAR:
      out << "STAR";
      break;
    case Symbol::CARET:
      out << "CARET";
      break;
    case Symbol::LBRACKET:
      out << "LBRACKET";
      break;
    case Symbol::RBRACKET:
      out << "RBRACKET";
      break;
    case Symbol::LPAREN:
      out << "LPAREN";
      break;
    case Symbol::RPAREN:
      out << "RPAREN";
      break;
    case Symbol::DASH:
      out << "DASH";
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
    case Concrete::RCONCAT:
      out << "RCONCAT";
      break;
    case Concrete::RSTAR:
      out << "RSTAR";
      break;
    case Concrete::RNOT:
      out << "RNOT";
      break;
    case Concrete::RRANGE:
      out << "RRANGE";
      break;
    case Concrete::RGROUP:
      out << "RGROUP";
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
    case Concrete::CREGEX:
      out << "CREGEX";
      break;
    case Concrete::CCONCAT:
      out << "CCONCAT";
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
constexpr Associativity getAssociativity(Symbol symbol) {
  return tokenAssoc[toIntTokenOffset(symbol)];
}

constexpr size_t numVariables = toInt(Symbol::STARTTOKENS);
constexpr size_t numTokens = toInt(Symbol::EPSILON) - toInt(Symbol::STARTTOKENS) - 1;
constexpr size_t numSymbols = toInt(Symbol::EPSILON);

using BitSetVars = std::bitset<numVariables>;
using BitSetToks = std::bitset<numTokens>;
using BitRef = BitSetVars::reference;

/* For printing only */
inline std::vector<Symbol> toVector(BitSetToks tokSet) {
  std::vector<Symbol> v;
  for (size_t i = 0; i < numTokens; ++i) {
    if (tokSet[i]) {
      v.push_back(toTokenOffset(i));
    }
  }
  return v;
}

#include "rules.hpp"

constexpr size_t ruleOverridePrecedence(const DFARule& rule) {
  return overridePrecedence[static_cast<int>(rule.lhs)];
}


/***********
 * OBJECTS *
 ***********/


const Symbol ROOT_SYM = Symbol::REGEX;
using ROOT_TYPE = Regex*;


/* S
 * We insert this into the grammar so that we guarantee the root symbol does
 * not appear on any right hand sides of rules. This way, we know the initial
 * lookahead set is empty.
 * */
struct Start {
  Start(ROOT_TYPE* r) : r_(r) {}
  ~Start() { delete r_; }
  Concrete getType() const { return Concrete::SCONC; }
  ROOT_TYPE* r_;
};


/*****************************
 *   CONSTRUCTION/DELETION   *
 *****************************/

struct StackObj {
  // Can't delete from here since it is a void*, see constructObj
  void* obj;
  Symbol symbol;
  Concrete type;

  void deleteObj() const noexcept;
  void shallowDeleteObj() const noexcept;
};


/* Apply this automatically if the type to which the symbol refers to is a raw pointer
 * since it is such a common thing to want
 * */
template <typename T>
inline void ptrDeleter(T* ptr) { delete ptr; }


inline void StackObj::deleteObj() const noexcept {
  switch (symbol) {
    case Symbol::REGEX:
      ptrDeleter(*(Regex**)obj);
      delete (Regex**)obj;
      break;
    case Symbol::ALTS:
      delete (RegexVector*)obj;
      break;
    case Symbol::CONCATS:
      delete (RegexVector*)obj;
      break;
    case Symbol::CHAR:
      delete (char*)obj;
      break;
    default:
      return;
  }
}

inline void StackObj::shallowDeleteObj() const noexcept {
  switch (symbol) {
    case Symbol::REGEX:
      delete (Regex**)obj;
      break;
    case Symbol::ALTS:
      delete (RegexVector*)obj;
      break;
    case Symbol::CONCATS:
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
/* Construct a concrete object */
inline void* constructObj(Concrete type, StackObj* args) {
  switch (type) {
    case Concrete::RALT:
      return new Regex*(new Alt(std::move(*(RegexVector*)args[0].obj)));
    case Concrete::RCONCAT:
      return new Regex*(new Concat(std::move(*(RegexVector*)args[0].obj)));
    case Concrete::RSTAR:
      return new Regex*(new Star(*(Regex**)args[0].obj));
    case Concrete::RNOT:
      return new Regex*(new Not(*(Regex**)args[1].obj));
    case Concrete::RRANGE:
      return new Regex*(new Range(*(char*)args[1].obj, *(char*)args[3].obj));
    case Concrete::RGROUP:
      return new Regex*(*(Regex**)args[1].obj);
    case Concrete::RCHAR:
      return new Regex*(new Character(*(char*)args[0].obj));
    case Concrete::AREGEX:
      return new RegexVector(RegexVector(*(Regex**)args[0].obj, *(Regex**)args[2].obj));
    case Concrete::AALT:
      return new RegexVector(RegexVector(std::move(*(RegexVector*)args[0].obj), *(Regex**)args[2].obj));
    case Concrete::CREGEX:
      return new RegexVector(RegexVector(*(Regex**)args[0].obj, *(Regex**)args[1].obj));
    case Concrete::CCONCAT:
      return new RegexVector(RegexVector(std::move(*(RegexVector*)args[0].obj), *(Regex**)args[1].obj));
    case Concrete::SCONC:
      return new Start((ROOT_TYPE*)args[0].obj);
    default:
      throw std::invalid_argument("Can't construct. Out of options.");
  }
}

inline StackObj construct(Concrete type, StackObj* args) {
  return StackObj{ constructObj(type, args), toSymbol(type), type };
}

/* NOT REAL, JUST FOR TESTING. ACTUAL LEXING IS DONE BY regex_lexer.hpp */
inline StackObj constructTokenObj(Symbol token, const std::string_view& str) {
  switch(token) {
    case Symbol::CHAR:
      return { new char(str[0]), token, Concrete::NONE };
    case Symbol::DASH:
      // NOTE: Generally a bad idea to pass string_view::data because it
      // it might not be null-terminated, but since we always consume from
      // left to right, we always have null-termination
      return { new int(atoi(str.data())), token, Concrete::NONE };
    default:
      return { nullptr, token, Concrete::NONE };
  }
}


/* LR0 Grammar */
const Grammar GRAMMAR = { { Symbol::S, { GrammarRule{ Concrete::SCONC, { ROOT_SYM } } } },
  { Symbol::REGEX,
      {
          GrammarRule{ Concrete::RALT, { Symbol::ALTS } },
          GrammarRule{ Concrete::RCONCAT, { Symbol::CONCATS } },
          GrammarRule{ Concrete::RSTAR, { Symbol::REGEX, Symbol::STAR } },
          GrammarRule{ Concrete::RNOT, { Symbol::CARET, Symbol::REGEX } },
          GrammarRule{ Concrete::RRANGE,
              { Symbol::LBRACKET, Symbol::CHAR, Symbol::DASH, Symbol::CHAR, Symbol::RBRACKET } },
           GrammarRule{ Concrete::RGROUP, { Symbol::LPAREN, Symbol::REGEX, Symbol::RPAREN } },
          GrammarRule{ Concrete::RCHAR, { Symbol::CHAR } },
      } },
  { Symbol::ALTS,
      {
          GrammarRule{ Concrete::AREGEX, { Symbol::REGEX, Symbol::BAR, Symbol::REGEX } },
          GrammarRule{ Concrete::AALT, { Symbol::ALTS, Symbol::BAR, Symbol::REGEX } },
      } },
  { Symbol::CONCATS,
      {
          GrammarRule{ Concrete::CREGEX, { Symbol::REGEX, Symbol::REGEX } },
          GrammarRule{ Concrete::CCONCAT, { Symbol::CONCATS, Symbol::REGEX } },
      } } };

#endif
