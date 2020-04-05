#ifndef LR1_GRAMMAR_HPP
#define LR1_GRAMMAR_HPP

#include "utils.hpp"

#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>


/*
 * S    -> Expr
 * Expr -> INT
 *       | Expr PLUS Expr
 *       | Expr STAR Expr
 */

/* Variables are positive */
static constexpr int EXPR = 1;

/* ExprType classes of Symbols */
static constexpr int EINT = 1;
static constexpr int EPLUS = 2;
static constexpr int ETIMES = 3;

/* Tokens are negative */
static constexpr int INT = -1;
static constexpr int PLUS = -2;
static constexpr int STAR = -3;


GrammarData GRAMMAR_DATA = {
  /* tokens */ {
      { "INT",
        "int",
        NONE,
        Assoc::NONE,
        "stoi(string(str))",
        "",
        "[1-9][0-9]*" },
      { "PLUS", "", 1, Assoc::LEFT, "", "", "\\+" },
      { "STAR", "", 2, Assoc::LEFT, "", "", "\\*" },
  },

  /* concretes */
  {
      { "SCONC", S, NONE, { EXPR }, "Start(#0*)" },
      { "EINT", EXPR, NONE, { INT }, "new EInt(#0)" },
      { "EPLUS", EXPR, NONE, { EXPR, PLUS, EXPR }, "new EPlus(#0, #2)" },
      { "ETIMES", EXPR, NONE, { EXPR, STAR, EXPR }, "new ETimes(#0, #2)" },
  },

  /* variables */
  { { "S", "Start", { SCONC }, "" },
    { "Expr", "Expr*", { EINT, EPLUS, ETIMES }, "delete #0;" } }
};


#endif
