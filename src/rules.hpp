#ifndef RULES_HPP
#define RULES_HPP

#include "dfa.hpp"

#include <cstddef>
#include <iostream>
#include <iterator>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

using namespace std;

/* Actual symbols in the grammar */
enum class Symbol { PLUS, DOLLAR, INT, EXPR, STMT };
std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::PLUS:
      out << "PLUS";
      break;
    case Symbol::DOLLAR:
      out << "DOLLAR";
      break;
    case Symbol::INT:
      out << "INT";
      break;
    case Symbol::EXPR:
      out << "EXPR";
      break;
    case Symbol::STMT:
      out << "STMT";
      break;
  }
  return out;
}

/* The types that each symbol in the grammar can be */
enum class SymbolType { PLUS, DOLLAR, INT, EINT, EPLUS, SEXPR };

/***********
 *  RULES  *
 ***********/

/* The actual grammar */
using RuleRhs = vector<vector<Symbol>>;
// TODO: Multimap
using Grammar = unordered_map<Symbol, RuleRhs>;

// NOTE: Starting point of the grammar must have a special name so that we
// know where to start building the DFA from.
Grammar grammar = {{Symbol::STMT, RuleRhs{{Symbol::EXPR, Symbol::DOLLAR}}},
    {Symbol::EXPR, RuleRhs{{Symbol::INT}, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}}}};

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
 *  DFA CONSTRUCTION  *
 **********************/

struct Rule {
  Symbol lhs;
  vector<Symbol> rhs;
  unsigned pos;

  bool atEnd() const { return pos == rhs.size(); }
  /* Given a rule "S -> A.B", returns B */
  // TODO: Throw on illegal access?
  Symbol nextSymbol() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return rhs[pos];
  }
  /* Given a rule "S -> A.B", returns "S -> AB." */
  Rule nextStep() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return {lhs, rhs, pos + 1};
  }
  // TODO: Make appropriate functions noexcept and const(expr)
  bool operator==(const Rule& other) const {
    return lhs == other.lhs && rhs == other.rhs && pos == other.pos;
  }
  friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
    out << "( " << rule.lhs << " -> ";
    size_t len = rule.rhs.size();
    for (size_t i = 0; i < len; ++i) {
      if (i == rule.pos) {
        out << '.';
      }
      out << rule.rhs[i] << ' ';
    }
    if (rule.pos == len) {
      out << '.';
    }
    out << ')';
    return out;
  }
};

// TODO: Fix these hash functions
namespace std {
  template <>
  struct hash<Rule> {
    size_t operator()(const Rule& rule) const noexcept {
      size_t h1 = std::hash<Symbol>()(rule.lhs);
      size_t h2 = 0;
      std::hash<Symbol> hasher;
      for (const Symbol& symbol : rule.rhs) {
        h2 += hasher(symbol);
      }
      size_t h3 = std::hash<size_t>()(rule.pos);
      return h3 + h1 ^ (h2 << 1);
    }
  };
}  // namespace std

/* Nodes of the DFA. Has to be a set, not a vector, because two groups of rules
 * should be equal if they contain the same rules (in any order) */
using RuleSet = unordered_set<Rule>;
using DFA_t = DFA<RuleSet, Symbol>;

namespace std {
  template <>
  struct hash<RuleSet> {
    size_t operator()(const RuleSet& ruleSet) const noexcept {
      std::hash<Rule> hasher;
      size_t h = 0;
      for (const Rule& rule : ruleSet) {
        h += hasher(rule);
      }
      return h;
    }
  };
}  // namespace std

bool isVariable(Symbol symbol) { return grammar.contains(symbol); }

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleSet& ruleSet, Symbol symbol) {
  const RuleRhs& rhses = grammar.at(symbol);
  for (const auto& rhs : rhses) {
    ruleSet.insert({ symbol, rhs, 0 });
  }
}

void addRhses(vector<Rule>& ruleList, Symbol symbol) {
  const RuleRhs& rhses = grammar.at(symbol);
  for (const auto& rhs : rhses) {
    ruleList.push_back({ symbol, rhs, 0 });
  }
}

/* Adds possible rules to node's state via epsilon transition in DFA.
 * Ex: S -> A.B, then add all rules B -> ??? */
void epsilonTransition(RuleSet& ruleSet) {
  // Keep track of the symbols whose rules we've already added to this rule list.
  unordered_set<Symbol> used;
  vector<Rule> newRules;

  // Keep expanding variables (epsilon transition) until we've determined all the
  // possible rule positions we could be in.
  for (const Rule& rule : ruleSet) {
    if (rule.atEnd()) {
      continue;
    }

    Symbol nextSymbol = rule.nextSymbol();
    if (isVariable(nextSymbol) && !used.contains(nextSymbol)) {
      addRhses(newRules, nextSymbol);
      used.insert(nextSymbol);
    }
  }
  ruleSet.insert(
      std::make_move_iterator(newRules.begin()), std::make_move_iterator(newRules.end()));
}

/* For each rule of this node, construct the transitions to successors. */
std::vector<const DFA_t::Node*> createTransitions(DFA_t& dfa, const DFA_t::Node* node) {
  // Create map of transitions to new nodes
  std::unordered_map<Symbol, RuleSet> newTransitions;
  for (const Rule& rule : node->getValue()) {
    if (rule.atEnd()) {
      continue;
    }

    Symbol nextSymbol = rule.nextSymbol();
    // If transition does not exist yet, create the successor node
    if (!newTransitions.contains(nextSymbol)) {
      newTransitions.emplace(nextSymbol, RuleSet{rule.nextStep()});
    } else {  // Otherwise, add it to the existing rule list
      newTransitions.at(nextSymbol).insert(rule.nextStep());
    }
  }

  // Apply epsilon transitions and create the transition
  std::vector<const DFA_t::Node*> addedNodes;
  for (auto& transitionRules : newTransitions) {
    epsilonTransition(transitionRules.second);
    const DFA_t::Node* newNode =
        dfa.addTransition(node, transitionRules.first, transitionRules.second);
    if (newNode) {
      addedNodes.push_back(newNode);
    }
  }
  return addedNodes;
}

/* Constructs the starting node of the DFA */
DFA_t initDFA() {
  RuleSet firstSet;
  // NOTE: Symbol::STMT must be a specified starting point
  addRhses(firstSet, Symbol::STMT);
  epsilonTransition(firstSet);
  DFA_t dfa(firstSet);
  return dfa;
}

DFA_t buildDFA() {
  queue<const DFA_t::Node*> q;
  DFA_t dfa = initDFA();
  q.push(dfa.getRoot());

  while (!q.empty()) {
    const DFA_t::Node* node = q.front();
    q.pop();
    std::vector<const DFA_t::Node*> addedNodes = createTransitions(dfa, node);
    for (const DFA_t::Node* newNode : addedNodes) {
      q.push(newNode);
    }
  }

  return dfa;
}

#endif
