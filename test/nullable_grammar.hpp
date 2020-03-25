#ifndef GRAMMAR1_HPP
#define GRAMMAR1_HPP

// FOR TESTING ONLY
#include "utils.hpp"

#include <cstddef>
#include <iostream>
#include <vector>
#include <string>

/*
 * S -> tY | YCX
 * C -> CX | Y | b
 * X -> XX | a
 * Y -> ε | b | Yz
 */


/* Variables */
static constexpr int S = 0; // Fixed
static constexpr int C = 1;
static constexpr int X = 2;
static constexpr int Y = 3;

/* Tokens */
static constexpr int t = 4;
static constexpr int b = 5;
static constexpr int a = 6;
static constexpr int z = 7;


inline std::string symbolToString(int symbol) {
  switch (symbol) {
    case S: return "S";
    case Y: return "Y";
    case C: return "C";
    case X: return "X";
    case t: return "t";
    case b: return "b";
    case a: return "a";
    case z: return "z";
  }
  return "";
}

/* Concrete classes of Symbols */
// static constexpr int NONE = 0;
static constexpr int S1 = 1;
static constexpr int S2 = 2;
static constexpr int C1 = 3;
static constexpr int C2 = 4;
static constexpr int C3 = 5;
static constexpr int X1 = 6;
static constexpr int X2 = 7;
static constexpr int Y1 = 8;
static constexpr int Y2 = 9;
static constexpr int Y3 = 10;


static constexpr int concreteToSymbol[] = { S, S, C, C, C, X, X, Y, Y, Y };
constexpr int toSymbol(int concrete) { return concreteToSymbol[concrete]; }

// FOR TESTING ONLY
const Grammar GRAMMAR = { {
                              GrammarRule{ S1, { t, Y } },
                              GrammarRule{ S2, { Y, C, X } },
                          },
                          {
                              GrammarRule{ C1, { C, X } },
                              GrammarRule{ C2, { Y } },
                              GrammarRule{ C3, { b } },
                          },
                          {
                              GrammarRule{ X1, { X, X } },
                              GrammarRule{ X2, { a } },
                          },
                          {
                              GrammarRule{ Y1, { EPSILON } },
                              GrammarRule{ Y2, { b } },
                              GrammarRule{ Y3, { Y, z } },
                          } };

#endif
