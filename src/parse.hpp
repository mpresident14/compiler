#ifndef PARSE_HPP
#define PARSE_HPP

#include "dfa.hpp"

#include <bitset>
#include <cstddef>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>
#include <stdexcept>

#include <prez/print_stuff.hpp>

// NOTE: Starting point of the grammar must have a special name ROOT_SYM so that we
// know where to start building the DFA from.

/**********************
 *  DFA CONSTRUCTION  *
 **********************/
using DFA_t = DFA<RuleSet, Symbol>;

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleSet& ruleSet, Symbol symbol) {
  using namespace std;

  const vector<GrammarRule>& rules = GRAMMAR.at(symbol);
  for (const GrammarRule& rule : rules) {
    ruleSet.insert(DFARule{rule.lhs, rule.rhs, 0, BitSetToks()});
  }
}

void addRhses(std::queue<DFARule>& ruleQueue, Symbol symbol) {
  using namespace std;

  const vector<GrammarRule>& rules = GRAMMAR.at(symbol);
  for (const GrammarRule& rule : rules) {
    ruleQueue.push(DFARule{rule.lhs, rule.rhs, 0, BitSetToks()});
  }
}

/* Adds possible rules to node's state via epsilon transition in DFA.
 * Ex: S -> A.B, then add all rules B -> ??? */
void epsilonTransition(RuleSet& ruleSet) {
  using namespace std;

  // Keep track of the symbols (variables only) whose rules we've already added to this rule list.
  bitset<toInt(Symbol::STARTTOKENS)> used;
  queue<DFARule> ruleQueue;

  // Expand variables (epsilon transition) in the initial rules.
  for (const DFARule& rule : ruleSet) {
    if (rule.atEnd()) {
      continue;
    }

    Symbol nextSymbol = rule.nextSymbol();
    if (isVariable(nextSymbol) && !used[toInt(nextSymbol)]) {
      addRhses(ruleQueue, nextSymbol);
      used[toInt(nextSymbol)] = true;
    }
  }

  // Keep expanding variables (epsilon transition) until we've determined all the
  // possible rule positions we could be in.
  while (!ruleQueue.empty()) {
    const DFARule& rule = *ruleSet.insert(move(ruleQueue.front())).first;
    ruleQueue.pop();
    if (rule.atEnd()) {
      continue;
    }

    Symbol nextSymbol = rule.nextSymbol();
    if (isVariable(nextSymbol) && !used[toInt(nextSymbol)]) {
      addRhses(ruleQueue, nextSymbol);
      used[toInt(nextSymbol)] = true;
    }
  }
}

/* For each rule of this node, construct the transitions to successors. */
std::vector<const DFA_t::Node*> createTransitions(DFA_t& dfa, const DFA_t::Node* node) {
  using namespace std;

  // Get all the valid transition symbols and map each of them to a new set of rules
  RuleSet newTransitions[numSymbols];

  for (const DFARule& rule : node->getValue()) {
    if (rule.atEnd()) {
      continue;
    }
    newTransitions[toInt(rule.nextSymbol())].insert(rule.nextStep());
  }

  // Apply epsilon transitions and create the transition
  vector<const DFA_t::Node*> addedNodes;
  for (size_t i = 0; i < numSymbols; ++i) {
    RuleSet& transitionRules = newTransitions[i];
    // No valid transition
    if (transitionRules.empty()) {
      continue;
    }
    epsilonTransition(transitionRules);
    const DFA_t::Node* newNode =
        dfa.addTransition(node, static_cast<Symbol>(i), move(transitionRules));
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
  using namespace std;

  queue<const DFA_t::Node*> q;
  DFA_t dfa = initDFA();
  q.push(dfa.getRoot());

  while (!q.empty()) {
    const DFA_t::Node* node = q.front();
    q.pop();
    vector<const DFA_t::Node*> addedNodes = createTransitions(dfa, node);
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

  void deleteObj() const;
};

// TODO: Remove throw and make noexcept
void StackObj::deleteObj() const {
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

Concrete tryReduce(const DFA_t::Node* node, const std::vector<StackObj>& stk, size_t* reduceStart) {
  Concrete retType = Concrete::NONE;
  for (const DFARule& rule : node->getValue()) {
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
void cleanPtrsFrom(const std::vector<StackObj>& stackObjs, size_t i) {
  size_t size = stackObjs.size();
  for (; i < size; ++i) {
    stackObjs[i].deleteObj();
  }
}

std::unique_ptr<ROOT_TYPE> parse(const DFA_t& dfa, const std::vector<StackObj>& inputTokens) {
  using namespace std;

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
        if (isToken(stk.back().symbol)) {
          stk.back().deleteObj();
        }
        stk.pop_back();
      }
      stk.push_back(newObj);

      // Restart the DFA.
      // TODO: Only backtrack as far as the reduction (store path)
      vector<Symbol> stkSymbols;
      transform(stk.begin(), stk.end(), back_inserter(stkSymbols), [](StackObj stkObj) {
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
