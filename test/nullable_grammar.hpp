#ifndef GRAMMAR1_HPP
#define GRAMMAR1_HPP

#include <bitset>
#include <cstddef>
#include <iostream>
#include <vector>

/* Terminals and nonterminals in the grammar */
enum class Symbol { S, C, X, Y, STARTTOKENS, T, B, A, Z, EPSILON };
inline std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::T:
      out << "t";
      break;
    case Symbol::B:
      out << "b";
      break;
    case Symbol::A:
      out << "a";
      break;
    case Symbol::Z:
      out << "z";
      break;
    case Symbol::EPSILON:
      out << "EPSILON";
      break;
    case Symbol::STARTTOKENS:
      out << "STARTTOKENS";
      break;
    case Symbol::S:
      out << "S";
      break;
    case Symbol::Y:
      out << "Y";
      break;
    case Symbol::C:
      out << "C";
      break;
    case Symbol::X:
      out << "X";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { S1, S2, C1, C2, C3, X1, X2, Y1, Y2, Y3 };
inline std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::S1:
      out << "S1";
      break;
    case Concrete::S2:
      out << "S2";
      break;
    case Concrete::C1:
      out << "C1";
      break;
    case Concrete::C2:
      out << "C2";
      break;
    case Concrete::C3:
      out << "C3";
      break;
    case Concrete::X1:
      out << "X1";
      break;
    case Concrete::X2:
      out << "X2";
      break;
    case Concrete::Y1:
      out << "Y1";
      break;
    case Concrete::Y2:
      out << "Y2";
      break;
    case Concrete::Y3:
      out << "Y3";
      break;
  }
  return out;
}

const Symbol concreteToSymbol[] = { Symbol::S,
  Symbol::S,
  Symbol::C,
  Symbol::C,
  Symbol::C,
  Symbol::X,
  Symbol::X,
  Symbol::Y,
  Symbol::Y,
  Symbol::Y };

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

#include "rules.hpp"

constexpr Symbol ROOT_SYM = Symbol::S;

const Grammar GRAMMAR = { { Symbol::S,
                              {
                                  GrammarRule{ Concrete::S1, { Symbol::T, Symbol::Y } },
                                  GrammarRule{ Concrete::S2, { Symbol::Y, Symbol::C, Symbol::X } },
                              } },
  { Symbol::C,
      {
          GrammarRule{ Concrete::C1, { Symbol::C, Symbol::X } },
          GrammarRule{ Concrete::C2, { Symbol::Y } },
          GrammarRule{ Concrete::C3, { Symbol::B } },
      } },
  { Symbol::X,
      {
          GrammarRule{ Concrete::X1, { Symbol::X, Symbol::X } },
          GrammarRule{ Concrete::X2, { Symbol::A } },
      } },
  { Symbol::Y,
      {
          GrammarRule{ Concrete::Y1, { Symbol::EPSILON } },
          GrammarRule{ Concrete::Y2, { Symbol::B } },
          GrammarRule{ Concrete::Y3, { Symbol::Y, Symbol::Z } },
      } } };

#endif
