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
#include <memory>

#include <prez/print_stuff.hpp>

using namespace std;

/* Terminals and nonterminals in the grammar */
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

/* The concrete types that symbols in the grammar can be */
enum class Concrete { EINT, EPLUS, SEXPR, NONE };
std::ostream& operator<<(std::ostream& out, const Concrete& type) {
  switch (type) {
    case Concrete::EINT:
      out << "EINT";
      break;
    case Concrete::EPLUS:
      out << "EPLUS";
      break;
    case Concrete::SEXPR:
      out << "SEXPR";
      break;
    case Concrete::NONE:
      out << "NONE";
      break;
  }
  return out;
}

/***********
 *  RULES  *
 ***********/

struct Rule {
  const Concrete lhs;
  const vector<Symbol> rhs;
  const size_t pos;

  bool atEnd() const { return pos == rhs.size(); }
  /* Given a rule "S -> A.B", returns B */
  // TODO: Remove throw when done
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
      size_t h1 = std::hash<Concrete>()(rule.lhs);
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

using Grammar = unordered_map<Symbol, vector<Rule>>;

// NOTE: Starting point of the grammar must have a special name so that we
// know where to start building the DFA from.
Grammar grammar = {
  {Symbol::STMT,
      {
        Rule{Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 0}
      }
  },
  {Symbol::EXPR,
      {
        Rule{Concrete::EINT, {Symbol::INT}, 0},
        Rule{Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0}
      },
  }
};

/**********************
 *  DFA CONSTRUCTION  *
 **********************/
using DFA_t = DFA<RuleSet, Symbol>;

bool isVariable(Symbol symbol) { return grammar.contains(symbol); }

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleSet& ruleSet, Symbol symbol) {
  const vector<Rule>& rules = grammar.at(symbol);
  for (const Rule& rule : rules) {
    ruleSet.insert(rule);
  }
}

void addRhses(vector<Rule>& ruleList, Symbol symbol) {
  const vector<Rule>& rules = grammar.at(symbol);
  for (const Rule& rule : rules) {
    ruleList.push_back(rule);
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

// TODO: We shouldn't have to build the DFA every time someone calls parse().
// Instead, we should just hard code all the nodes after building the first time
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


/***********
 * OBJECTS *
 ***********/

struct Obj {
  virtual ~Obj() {}
  virtual Symbol getSymbol() = 0;
  virtual bool isToken() = 0;
};

struct TokenObj : Obj {
  // TODO: Remove isToken() method
  bool isToken() override { return true; }
};

struct VariableObj : Obj {
  bool isToken() override { return false; }
  virtual Concrete getType() = 0;
};

/* Tokens */
struct Plus : TokenObj {
  Symbol getSymbol() override { return Symbol::PLUS; }
};
struct Dollar : TokenObj {
  Symbol getSymbol() override { return Symbol::DOLLAR; }
};
// TODO: Not sure about how to handle tokens with info
struct Int : TokenObj {
  Int(const string& str) : i_(atoi(str.c_str())) {}
  Symbol getSymbol() override { return Symbol::INT; }
  operator int() const { return i_; }
  int i_;
};

/* Expr */
struct Expr : VariableObj {
  virtual ~Expr(){};
  Symbol getSymbol() override { return Symbol::EXPR; }
};

struct EInt : Expr {
  EInt(Obj* i) : i_(*(Int*) i) {}
  Concrete getType() override { return Concrete::EINT; }
  int i_;
};

struct EPlus : Expr {
  EPlus(Obj* e1, Obj* e2) : e1_((Expr*) e1), e2_((Expr*) e2) {}
  ~EPlus() {
    delete e1_;
    delete e2_;
  }
  Concrete getType() override { return Concrete::EPLUS; }
  Expr* e1_;
  Expr* e2_;
};

/* Stmt */
struct Stmt : VariableObj {
  virtual ~Stmt() {}
  Symbol getSymbol() override { return Symbol::STMT; }
};

struct SExpr : Stmt {
  SExpr(Obj* e1) : e1_((Expr*) e1) {}
  ~SExpr() { delete e1_; }
  Concrete getType() override { return Concrete::SEXPR; }
  Expr* e1_;
};

/**********************
 *   SHIFT-REDUCING   *
 **********************/

Obj* construct(Concrete type, Obj** args) {
  switch (type) {
    case Concrete::EINT: return new EInt(args[0]);
    case Concrete::EPLUS: return new EPlus(args[0], args[2]);
    case Concrete::SEXPR: return new SExpr(args[0]);
    default: throw std::invalid_argument("Out of options.");
  }
}

Concrete tryReduce(const DFA_t::Node* node, const vector<Obj*>& stk, size_t *reduceStart) {
  Concrete retType = Concrete::NONE;
  for (const auto& rule : node->getValue()) {
    if (!rule.atEnd()) {
      continue;
    }

    size_t i = rule.rhs.size() - 1;
    size_t j = stk.size() - 1;
    while (j >= 0) {
      if (rule.rhs[i] != stk[j]->getSymbol()) {
        break;
      }

      // TODO: For now, just always reduce
      if (i == 0) {
        *reduceStart = j;
        // retType = rule.lhs;
        return rule.lhs;
      }
      --i;
      --j;
    }
  }
  return retType;
}

unique_ptr<Stmt> parse(vector<TokenObj*> inputTokens) {
  DFA_t dfa = buildDFA();
  TokenObj* tok = inputTokens[0];
  vector<Obj*> stk = { tok };
  const DFA_t::Node* currentNode = dfa.step(dfa.getRoot(), tok->getSymbol());

  size_t i = 1;
  size_t inputSize = inputTokens.size();

  // Stop when root of grammar is the only thing on the stack
  while (!(stk.size() == 1 && stk[0]->getSymbol() == Symbol::STMT)) {
    size_t reduceStart;
    Concrete type = tryReduce(currentNode, stk, &reduceStart);
    if (type != Concrete::NONE) {
      // Construct the new object, pop the arguments off the stack,
      // and push the new object onto it.
      Obj* newObj = construct(type, &stk.data()[reduceStart]);
      stk.erase(stk.begin() + reduceStart, stk.end());
      stk.push_back(newObj);
      // Restart the DFA.
      // TODO: Only backtrack as far as the reduction (store path)
      vector<Symbol> stkSymbols;
      std::transform(stk.begin(), stk.end(), std::back_inserter(stkSymbols),
        [](Obj* ptr) { return ptr->getSymbol(); });
      currentNode = dfa.run(stkSymbols);
    } else {
      // Didn't reduce to S
      if (i == inputSize) {
        return nullptr;
      }
      TokenObj* token = inputTokens[i];
      stk.push_back(token);
      currentNode = dfa.step(currentNode, token->getSymbol());
      // No transition
      if (currentNode == nullptr) {
        return nullptr;
      }
      ++i;
    }
  }

  return unique_ptr<Stmt>((Stmt*) (stk[0]));
}


#endif
