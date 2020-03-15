#ifndef PARSE_HPP
#define PARSE_HPP

#include "dfa.hpp"
#include "lr0_grammar.hpp"
#include "lr0_rules.hpp"
// #include "slr1_grammar.hpp"

#include <cstddef>
#include <iostream>
#include <string>

// NOTE: Starting point of the grammar must have a special name ROOT_SYM so that we
// know where to start building the DFA from.

/* SLR1 Grammar */
// const Grammar GRAMMAR = {
//   {ROOT_SYM,
//       {
//         Rule{Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 0}
//       }
//   },
//   {Symbol::EXPR,
//       {
//         Rule{Concrete::EINT, {Symbol::INT}, 0},
//         Rule{Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0}
//       },
//   }
// };

/* LR0 Grammar */
const Grammar GRAMMAR = {
    {ROOT_SYM,
        {
            Rule{Concrete::ETERM, {Symbol::TERM}, 0},
            Rule{Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 0},
        }},
    {
        Symbol::TERM,
        {
            Rule{Concrete::TINT, {Symbol::INT}, 0},
        },
    }};

/**********************
 *  DFA CONSTRUCTION  *
 **********************/
using DFA_t = DFA<RuleSet, Symbol>;

bool isVariable(Symbol symbol) { return GRAMMAR.contains(symbol); }

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleSet& ruleSet, Symbol symbol) {
  const vector<Rule>& rules = GRAMMAR.at(symbol);
  for (const Rule& rule : rules) {
    ruleSet.insert(rule);
  }
}

void addRhses(vector<Rule>& ruleList, Symbol symbol) {
  const vector<Rule>& rules = GRAMMAR.at(symbol);
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
  // NOTE: ROOT_SYM must be a specified starting point
  addRhses(firstSet, ROOT_SYM);
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

/**********************
 *   SHIFT-REDUCING   *
 **********************/

struct StackObj {
  // No deleter since pointers will be passed to the client
  void* obj;
  Symbol symbol;
  Concrete type;

  void deleteObj() const {
    switch (symbol) {
      case Symbol::INT:
        delete (Int*)obj;
        break;
      case Symbol::PLUS:
        delete (Plus*)obj;
        break;
      case Symbol::TERM:
        delete (Term*)obj;
        break;
      case Symbol::EXPR:
        delete (Expr*)obj;
        break;
      default:
        throw std::invalid_argument("Can't delete. Out of options.");
    }
  }
};

StackObj construct(Concrete type, StackObj* args) {
  switch (type) {
    case Concrete::TINT:
      return StackObj{new TInt(*(Int*)args[0].obj), toSymbol(type), type};
    case Concrete::ETERM:
      return StackObj{new ETerm((Term*)args[0].obj), toSymbol(type), type};
    case Concrete::EPLUS:
      return StackObj{new EPlus((Expr*)args[0].obj, (Term*)args[2].obj), toSymbol(type), type};
    default:
      throw std::invalid_argument("Can't construct. Out of options.");
  }
}

Concrete tryReduce(const DFA_t::Node* node, const vector<StackObj>& stk, size_t* reduceStart) {
  Concrete retType = Concrete::NONE;
  for (const Rule& rule : node->getValue()) {
    // Make sure we have completed the rule
    if (!rule.atEnd()) {
      continue;
    }

    // Match the rule with the top of the stack
    size_t i = rule.rhs.size() - 1;
    size_t j = stk.size() - 1;
    while (j >= 0) {
      if (rule.rhs[i] != stk[j].symbol) {
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

/* Clear from StackObj::obj starting at index i */
void cleanPtrsFrom(const vector<StackObj>& stackObjs, size_t i) {
  size_t size = stackObjs.size();
  for (; i < size; ++i) {
    stackObjs[i].deleteObj();
  }
}

unique_ptr<ROOT_TYPE> parse(const DFA_t& dfa, const vector<StackObj>& inputTokens) {
  StackObj firstToken = inputTokens[0];
  vector<StackObj> stk = {firstToken};
  const DFA_t::Node* currentNode = dfa.step(dfa.getRoot(), firstToken.symbol);
  if (currentNode == nullptr) {
    cleanPtrsFrom(inputTokens, 0);
    return nullptr;
  }

  size_t i = 1;
  size_t inputSize = inputTokens.size();

  // Stop when we have consumed all the input and the root of grammar
  // is the only thing on the stack
  while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == ROOT_SYM)) {
    size_t reduceStart;
    Concrete type = tryReduce(currentNode, stk, &reduceStart);
    if (type != Concrete::NONE) {
      // Construct the new object, pop the arguments off the stack,
      // and push the new object onto it.
      StackObj newObj = construct(type, &stk.data()[reduceStart]);
      size_t stkSize = stk.size();
      for (size_t j = 0; j < stkSize - reduceStart; ++j) {
        // Tokens are not encapsulated within the underlying object, so the
        // pointers need to be deleted
        if (static_cast<int>(stk.back().symbol) < static_cast<int>(Symbol::ENDTOKENS)) {
          stk.back().deleteObj();
        }
        stk.pop_back();
      }
      stk.push_back(newObj);

      // Restart the DFA.
      // TODO: Only backtrack as far as the reduction (store path)
      vector<Symbol> stkSymbols;
      std::transform(stk.begin(), stk.end(), std::back_inserter(stkSymbols), [](StackObj stkObj) {
        return stkObj.symbol;
      });
      currentNode = dfa.run(stkSymbols);
    } else {
      // No more tokens, didn't reduce to S
      if (i == inputSize) {
        cleanPtrsFrom(stk, 0);
        return nullptr;
      }
      StackObj token = inputTokens[i];
      stk.push_back(token);
      currentNode = dfa.step(currentNode, token.symbol);
      // No transition for this token
      if (currentNode == nullptr) {
        cleanPtrsFrom(stk, 0);
        cleanPtrsFrom(inputTokens, i + 1);
        return nullptr;
      }
      ++i;
    }
  }

  return unique_ptr<ROOT_TYPE>((ROOT_TYPE*)(stk[0].obj));
}

#endif
