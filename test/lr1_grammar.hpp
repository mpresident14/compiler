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
      { "INT", NONE, Assoc::NONE, "", "", ""},
      { "PLUS", 1, Assoc::LEFT, "", "", ""},
      { "STAR", 2, Assoc::LEFT, "", "", ""},
    },

    /* concretes */ {
      {"SCONC", S, NONE, {EXPR}, {}, ""},
      {"EINT", EXPR, NONE, {INT}, {}, ""},
      {"EPLUS", EXPR, NONE, {EXPR, PLUS, EXPR}, {}, ""},
      {"ETIMES", EXPR, NONE, {EXPR, STAR, EXPR}, {}, ""},
    },

    /* variables */ {
      {"S", {SCONC}, ""},
      {"EXPR", {EINT, EPLUS, ETIMES}, ""}
    }
};


/***********
 * OBJECTS *
 ***********/

// NOTE: getType() not required for parsing, but helpful for client
// TODO: Perhaps pass an argument to the generator that uses existing
// classes or creates them for you

enum class ExprType { EINT, EPLUS, ETIMES };

/* Expr */
struct Expr {
  virtual ~Expr(){};
  virtual ExprType getType() const = 0;
  virtual int eval() const = 0;
};

struct EInt : Expr {
  EInt(int i) : i_(i) {}
  ExprType getType() const override { return ExprType::EINT; }
  int eval() const override { return i_; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~EPlus() {
    delete e1_;
    delete e2_;
  }
  ExprType getType() const override { return ExprType::EPLUS; }
  int eval() const override { return e1_->eval() + e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};

struct ETimes : Expr {
  ETimes(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~ETimes() {
    delete e1_;
    delete e2_;
  }
  ExprType getType() const override { return ExprType::EPLUS; }
  int eval() const override { return e1_->eval() * e2_->eval(); }
  Expr* e1_;
  Expr* e2_;
};


// using ROOT_TYPE = Expr*;


// /* S
//  * We insert this into the grammar so that we guarantee the root symbol does
//  * not appear on any right hand sides of rules. This way, we know the initial
//  * lookahead set is empty.
//  * */
// struct Start {
//   Start(ROOT_TYPE* r) : r_(r) {}
//   ~Start() { delete r_; }
//   ROOT_TYPE* r_;
// };


// /*****************************
//  *   CONSTRUCTION/DELETION   *
//  *****************************/

// struct StackObj {
//   // No deleter since pointers will be passed to the client
//   void* obj;
//   int symbol;
//   int concrete;

//   void deleteObj() const noexcept;
//   void deleteObjPtr() const noexcept;
// };

// template <typename T>
// inline void ptrDeleter(T* ptr) {
//   delete ptr;
// }

// void StackObj::deleteObj() const noexcept {
//   switch (symbol) {
//     case INT:
//       delete (int*)obj;
//       break;
//     case EXPR:
//       ptrDeleter(*(Expr**)obj);
//       delete (Expr**)obj;
//       break;
//     default:
//       return;
//   }
// }

// void StackObj::deleteObjPtr() const noexcept {
//   switch (symbol) {
//     case INT:
//       delete (int*)obj;
//       break;
//     case EXPR:
//       delete (Expr**)obj;
//       break;
//     default:
//       return;
//   }
// }

// // TODO: Remove throw and make noexcept when done
// void* constructObj(int concrete, StackObj* args) {
//   switch (concrete) {
//     case EINT:
//       return new Expr*(new EInt(*(int*)args[0].obj));
//     case EPLUS:
//       return new Expr*(new EPlus(*(Expr**)args[0].obj, *(Expr**)args[2].obj));
//     case ETIMES:
//       return new Expr*(new ETimes(*(Expr**)args[0].obj, *(Expr**)args[2].obj));
//     case 0: /* SCONC */
//       return new Start((ROOT_TYPE*)args[0].obj);
//     default:
//       throw std::invalid_argument("Can't construct. Out of options.");
//   }
// }

// StackObj construct(int concrete, StackObj* args, std::vector<int> concToSym) {
//   return StackObj{ constructObj(concrete, args),
//                    concToSym[concrete],
//                    concrete };
// }


#endif
