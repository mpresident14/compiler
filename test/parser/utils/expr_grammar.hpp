#ifndef LR1_GRAMMAR_HPP
#define LR1_GRAMMAR_HPP

#include "src/parser/utils.hpp"

namespace test::expr_grammar {
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
        {"INT", "int", NONE, Assoc::NONE, "stoi(string(#str))", "",
         "[1-9][0-9]*"},
        {"PLUS", "", 1, Assoc::LEFT, "", "", "\\+"},
        {"STAR", "", 2, Assoc::LEFT, "", "", "\\*"},
        {"whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+"},
    },

    /* concretes */
    {
        {"SCONC", S, NONE, {EXPR}, "Start(#0*)"},
        {"EINT", EXPR, NONE, {INT}, "new EInt(#0)"},
        {"EPLUS", EXPR, NONE, {EXPR, PLUS, EXPR}, "new EBinOp(#0, PLUS, #2)"},
        {"ETIMES", EXPR, NONE, {EXPR, STAR, EXPR}, "new EBinOp(#0, TIMES, #2)"},
    },

    /* variables */
    {{"S", "Start", {SCONC}, ""},
     {"Expr", "Expr*", {EINT, EPLUS, ETIMES}, "delete #obj;"}}};

/* Will result in conflict because no operator precedence */
GrammarData BAD_GRAMMAR_DATA = {
    /* tokens */ {
        {"INT", "int", NONE, Assoc::NONE, "stoi(string(#str))", "",
         "[1-9][0-9]*"},
        {"PLUS", "", NONE, Assoc::LEFT, "", "", "\\+"},
        {"STAR", "", NONE, Assoc::LEFT, "", "", "\\*"},
        {"whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+"},
    },

    /* concretes */
    {
        {"SCONC", S, NONE, {EXPR}, "Start(#0*)"},
        {"EINT", EXPR, NONE, {INT}, "new EInt(#0)"},
        {"EPLUS", EXPR, NONE, {EXPR, PLUS, EXPR}, "new EBinOp(#0, PLUS, #2)"},
        {"ETIMES", EXPR, NONE, {EXPR, STAR, EXPR}, "new EBinOp(#0, TIMES, #2)"},
    },

    /* variables */
    {{"S", "Start", {SCONC}, ""},
     {"Expr", "Expr*", {EINT, EPLUS, ETIMES}, "delete #obj;"}}};

} // namespace test::expr_grammar
#endif
