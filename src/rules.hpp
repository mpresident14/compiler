#ifndef RULES_HPP
#define RULES_HPP

#include "nfa.hpp"

#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

/* Actual symbols in the grammar */
enum class Symbol { PLUS, DOLLAR, INT, EXPR, STMT };

/* The types that each symbol in the grammar can be */
enum class SymbolType { PLUS, DOLLAR, INT, EINT, EPLUS, SEXPR };

/***********
 *  RULES  *
 ***********/

/* The actual grammar */
using RuleRhs = vector<vector<Symbol>>;
using Grammar = unordered_map<Symbol, RuleRhs>;

// NOTE: Starting point of the grammar must have a special name so that we
// know where to start building the NFA from.
Grammar grammar = {{Symbol::STMT, RuleRhs{{Symbol::EXPR, Symbol::DOLLAR}}},
    {Symbol::EXPR, RuleRhs{{Symbol::INT}, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}}}};

/***********
 * OBJECTS *
 ***********/
struct SymbolObj {
  virtual ~SymbolObj() {}
  virtual Symbol getSymbol() = 0;
  virtual SymbolType getType() = 0;
  virtual bool isToken() = 0;
};

struct TokenObj : SymbolObj {
  virtual bool isToken() { return true; }
};

struct VariableObj : SymbolObj {
  virtual bool isToken() { return false; }
};

/* Tokens */
struct Plus : TokenObj {
  Symbol getSymbol() { return Symbol::PLUS; }
  SymbolType getType() { return SymbolType::PLUS; }
};
struct Dollar : TokenObj {
  Symbol getSymbol() { return Symbol::DOLLAR; }
  SymbolType getType() { return SymbolType::DOLLAR; }
};
// TODO: Not sure about how to handle tokens with info
struct Int : TokenObj {
  Int(const string& str) : i_(atoi(str.c_str())) {}
  Symbol getSymbol() { return Symbol::INT; }
  SymbolType getType() { return SymbolType::INT; }
  int i_;
};

/* Expr */
struct Expr : VariableObj {
  virtual ~Expr(){};
  Symbol getSymbol() { return Symbol::EXPR; }
};

struct EInt : VariableObj {
  EInt(int i) : i_(i) {}
  SymbolType getType() { return SymbolType::EINT; }
  int i_;
};

struct EPlus : VariableObj {
  EPlus(Expr* e1, Expr* e2) : e1_(e1), e2_(e2) {}
  ~EPlus() {
    delete e1_;
    delete e2_;
  }
  SymbolType getType() { return SymbolType::EPLUS; }
  Expr* e1_;
  Expr* e2_;
};

/* Stmt */
struct Stmt : VariableObj {
  virtual ~Stmt() {}
  Symbol getSymbol() { return Symbol::STMT; }
};

struct SExpr : VariableObj {
  SExpr(Expr* e1) : e1_(e1) {}
  ~SExpr() { delete e1_; }
  SymbolType getType() { return SymbolType::SEXPR; }
  Expr* e1_;
};

/**********************
 *  NFA CONSTRUCTION  *
 **********************/

/* Nodes of the NFA */
struct Rule {
  Symbol lhs;
  vector<Symbol> rhs;
  unsigned pos;

  bool atEnd() const { return pos == rhs.size(); }
  /* Given a rule "S -> A.B", returns B */
  Symbol nextSymbol() const { return rhs[pos]; }
  /* Given a rule "S -> A.B", returns "S -> AB." */
  Rule nextStep() const { return {lhs, rhs, pos + 1}; }
  bool operator==(const Rule& other) {
    return lhs == other.lhs && rhs == other.rhs && pos == other.pos;
  }
};
using RuleList = vector<Rule>;
using NFA_t = NFA<RuleList, Symbol>;

bool isVariable(Symbol symbol) { return grammar.contains(symbol); }

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleList& ruleList, Symbol symbol) {
  RuleRhs& rhses = grammar[symbol];
  for (const auto& rhs : rhses) {
    ruleList.push_back({symbol, rhs, 0});
  }
}

void epsilonTransition(NFA_t::Node* node) {
  // Keep track of the symbols whose rules we've already added to this rule list.
  unordered_set<Symbol> used;
  RuleList& ruleList = node->value_;
  size_t ruleNum = 0;

  // Keep expanding variables (epsilon transition) until we've determined all the
  // possible rule positions we could be in.
  while (ruleNum < ruleList.size()) {
    Symbol nextSymbol = ruleList[ruleNum].nextSymbol();
    if (isVariable(nextSymbol) && !used.contains(nextSymbol)) {
      addRhses(ruleList, nextSymbol);
      used.insert(nextSymbol);
    }
    ++ruleNum;
  }
}

NFA_t initNFA() {
  RuleList firstList;
  addRhses(firstList, Symbol::STMT);
  NFA_t nfa(firstList);
  epsilonTransition(nfa.getRoot());
  return nfa;
}

/* For each rule of this node, construct the transitions to successors.
 * We don't epsilon-transition the successors in the this function. */
void addTransitions(NFA_t::Node* node) {
  std::unordered_map<Symbol, NFA_t::Node*>& transitions = node->transitions_;
  for (const Rule& rule : node->value_) {
    if (rule.atEnd()) {
      continue;
    }

    Symbol nextSymbol = rule.nextSymbol();
    // If transition does not exist yet, create the successor node
    if (!transitions.contains(nextSymbol)) {
      node->addTransition(nextSymbol, { rule.nextStep() });
    } else {
      RuleList& nextRules = transitions[nextSymbol]->value_;
      nextRules.push_back(rule.nextStep());
    }
  }
}

// TODO: Make vectors pointers to vectors to avoid all the unnecessary copies
NFA_t buildNFA() {
  queue<NFA_t::Node*> q;
  NFA<RuleList, Symbol> nfa = initNFA();
  q.push(nfa.getRoot());

  while (!q.empty()) {
    NFA_t::Node* node = q.back();
    q.pop();
    addTransitions(node);
    for (auto& transSuccessor : node->transitions_) {
      epsilonTransition(transSuccessor.second);
      q.push(transSuccessor.second);
    }
  }

  return nfa;
}

#endif
