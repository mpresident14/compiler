#ifndef GRAMMAR1_HPP
#define GRAMMAR1_HPP

#include <cstddef>
#include <iostream>
#include <string>

/* Terminals and nonterminals in the grammar */
enum class Symbol { S, C, X, Y, STARTTOKENS, T, B, A, Z, EPSILON, NUMSYMBOLS };
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
    case Symbol::NUMSYMBOLS:
      out << "NUMSYMBOLS";
      break;
  }
  return out;
}

/* The concrete types that symbols in the grammar can be */
enum class Concrete { S1, S2, C1, C2, C3, X1, X2, Y1, Y2, Y3};
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

const Symbol concreteToSymbol[] = {
  Symbol::S, Symbol::S, Symbol::C, Symbol::C, Symbol::C, Symbol::X, Symbol::X, Symbol::Y, Symbol::Y, Symbol::Y};

constexpr Symbol toSymbol(Concrete concrete) { return concreteToSymbol[static_cast<int>(concrete)]; }
constexpr int toInt(Symbol symbol) { return static_cast<int>(symbol); }
constexpr int toIntTokenOffset(Symbol symbol) { return toInt(symbol) - toInt(Symbol::STARTTOKENS) - 1; }
constexpr bool isToken(Symbol symbol) { return toInt(symbol) > toInt(Symbol::STARTTOKENS); }
constexpr bool isVariable(Symbol symbol) { return !isToken(symbol); }

const Symbol ROOT_SYM = Symbol::S;

#include "rules.hpp"

const Grammar GRAMMAR = {
    {Symbol::S,
        {
            Rule{Concrete::S1, {Symbol::T, Symbol::Y}, 0},
            Rule{Concrete::S2, {Symbol::Y, Symbol::C, Symbol::X}, 0},
        }
    },
    {Symbol::C,
        {
            Rule{Concrete::C1, {Symbol::C, Symbol::X}, 0},
            Rule{Concrete::C2, {Symbol::Y}, 0},
            Rule{Concrete::C3, {Symbol::B}, 0},
        }
    },
    {Symbol::X,
        {
            Rule{Concrete::X1, {Symbol::X, Symbol::X}, 0},
            Rule{Concrete::X2, {Symbol::A}, 0},
        }
    },
    {Symbol::Y,
        {
            Rule{Concrete::Y1, {Symbol::EPSILON}, 0},
            Rule{Concrete::Y2, {Symbol::B}, 0},
            Rule{Concrete::Y3, {Symbol::Y, Symbol::Z}, 0},
        }
    }
};


#endif
