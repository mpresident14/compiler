#ifndef PARSE_HPP
#define PARSE_HPP

#include "dfa.hpp"
#include "null_first.hpp"

#include <bitset>
#include <cstddef>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

// NOTE: Starting point of the grammar is always Symbol::S

/**********************
 *  DFA CONSTRUCTION  *
 **********************/
using DFA_t = DFA<RuleSet, Symbol>;

/* Given a rule "A -> .BC", add all "B -> .<rhs>" rules to rule queue,
 * updating the lookahead set
 * */

std::vector<BitSetToks> firsts = getFirsts();

void addRhses(std::queue<DFARule>& ruleQueue, const DFARule& fromRule) {
  using namespace std;

  // Nothing to expand if we are at the end of the rule or if the next symbol
  // is a token
  Symbol nextSymbol = fromRule.nextSymbol();
  if (nextSymbol == Symbol::EPSILON || isToken(nextSymbol)) {
    return;
  }

  // Construct the new lookahead set, starting with the lookahead set of
  // the rule from which we are expanding
  BitSetToks newLookahead = fromRule.lookahead;
  Symbol nextNextSymbol = fromRule.nextNextSymbol();
  if (nextNextSymbol != Symbol::EPSILON) {
    // FIRST() of a token is just the token
    if (isToken(nextNextSymbol)) {
      newLookahead[toIntTokenOffset(nextNextSymbol)] = true;
    } else {  // Union with the first of the nextNextSymbol
      newLookahead |= firsts[toInt(nextNextSymbol)];
    }
  }

  const vector<GrammarRule>& rules = GRAMMAR.at(nextSymbol);
  for (const GrammarRule& rule : rules) {
    ruleQueue.push(DFARule{ rule.lhs, rule.rhs, 0, newLookahead });
  }
}


/* Adds possible rules to node's state via epsilon transition in DFA.
 * Ex: S -> A.B, then add all rules B -> ??? */
void epsilonTransition(RuleSet& ruleSet) {
  using namespace std;

  queue<DFARule> ruleQueue;

  // Expand variables (epsilon transition) in the initial set of rules.
  for (const DFARule& rule : ruleSet) {
    addRhses(ruleQueue, rule);
  }

  // Keep expanding variables (epsilon transition) until we've determined all the
  // possible rule positions we could be in.
  while (!ruleQueue.empty()) {
    DFARule& rule = ruleQueue.front();
    auto iter = ruleSet.find(rule);
    // If rule is not yet in the set, add it

    // TODO: Should we check for duplicate rules in the queue as well (like use a set and just pop with set.begin())
    if (iter == ruleSet.end()) {
      addRhses(ruleQueue, rule);
      ruleSet.insert(move(rule));
      ruleQueue.pop();
      continue;
    }

    // If rule is already in the set, check to see if the new rule has any new
    // members its lookahead set. If so, replace existing rule with new one.
    const DFARule& existingRule = *iter;
    BitSetToks unionToks = rule.lookahead | existingRule.lookahead;
    if (existingRule.lookahead != unionToks) {
      // TODO: Might be faster to just do operator|= again
      existingRule.lookahead = move(unionToks);
      // Only add RHSes if we insert the rule into the set because everything in the
      // set has already been expanded
      addRhses(ruleQueue, rule);
    }

    ruleQueue.pop();
  }
}

/* For each rule of this node, construct the transitions to successors.
 * Return the successors that were newly added */
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
  RuleSet firstSet{ DFARule{ Concrete::SCONC, { ROOT_SYM }, 0, BitSetToks() } };
  epsilonTransition(firstSet);
  DFA_t dfa(firstSet);
  return dfa;
}

// TODO: We shouldn't have to build the DFA every time someone calls parse().
// Instead, we should just hard code all the nodes after building the first time
DFA_t buildParserDFA() {
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

// /**********************
//  *   SHIFT-REDUCING   *
//  **********************/

/* Rule precedence equals precedence of last token */
Symbol getLastToken(const DFARule& rule) {
  for (auto iter = rule.rhs.crbegin(); iter != rule.rhs.crend(); ++iter) {
    Symbol symbol = *iter;
    if (isToken(symbol)) {
      return symbol;
    }
  }
  return Symbol::EPSILON;
}

void conflictWarning(const DFARule& rule1, const DFARule& rule2) {
  std::cerr << "WARNING: Shift reduce conflict with\n\t" << rule1 << "\n\t" << rule2 << std::endl;
}

// TODO: Is it possible to have more than one rule able to be reduced? If so, handle it.
Concrete tryReduce(const DFA_t::Node* node,
    Symbol nextToken,
    const std::vector<StackObj>& stk,
    size_t* reduceStart) {
  Concrete retType = Concrete::NONE;
  const DFARule* shiftableRule = nullptr;
  size_t shiftPrecedence = getPrecedence(nextToken);

  // Check if shifting is an option
  if (nextToken != Symbol::EPSILON) {
    for (const DFARule& rule : node->getValue()) {
      if (rule.nextSymbol() == nextToken) {
        shiftableRule = &rule;
        break;
      }
    }
  }

  // Check if we should reduce
  for (const DFARule& rule : node->getValue()) {
    // Make sure we have completed the rule and either
    // the next token is in the lookahead set or there is no more input
    if (!(rule.atEnd() &&
            (nextToken == Symbol::EPSILON || rule.lookahead[toIntTokenOffset(nextToken)]))) {
      continue;
    }

    // Try to match the rule with the top of the stack
    size_t i = rule.rhs.size() - 1;
    size_t j = stk.size() - 1;
    while (j >= 0) {
      // If symbols don't match, move on
      if (rule.rhs[i] != stk[j].symbol) {
        break;
      }

      // Entire rule was matched
      if (i == 0) {
        // Potential shift-reduce conflict
        if (shiftableRule) {
          // Check to see if user has overridden the token-specified precedence of this rule
          // Otherwise rule precedence is that of last token in rule
          size_t rulePrecedence = ruleOverridePrecedence(rule);
          Symbol rulesLastToken = getLastToken(rule);
          if (rulePrecedence == 0 && rulesLastToken != Symbol::EPSILON) {
            rulePrecedence = getPrecedence(rulesLastToken);
          }
          // Unspecified precedence -> conflict! (Resolve by shifting)
          if (rulePrecedence == 0 && shiftPrecedence == 0) {
            goto conflict;
          }
          // TODO: Handle nonassociativity
          // Reduce if one of the following is true:
          // - precedence of rule is higher than that of next token
          // - precedence of rule is the same of that of next token and the rule's
          //   last token is left-associative
          if (rulePrecedence > shiftPrecedence) {
            goto shouldReduce;
          } else if (rulePrecedence == shiftPrecedence) {
            Associativity assoc = getAssociativity(rulesLastToken);
            if (assoc == Associativity::LEFT) {
              goto shouldReduce;
            } else if (assoc == Associativity::UNSPECIFIED) {
              // Unspecified associativity -> conflict! (Resolve by shifting)
              goto conflict;
            }
          } else {
            // shift precedence is higher, so shift
            return Concrete::NONE;
          }
        }

      shouldReduce:
        *reduceStart = j;
        return rule.lhs;
      conflict:
        conflictWarning(*shiftableRule, rule);
        return Concrete::NONE;
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

ROOT_TYPE parse(const DFA_t& dfa, const std::vector<StackObj>& inputTokens) {
  using namespace std;

  StackObj firstToken = inputTokens[0];
  vector<StackObj> stk = { firstToken };
  const DFA_t::Node* currentNode = dfa.step(dfa.getRoot(), firstToken.symbol);
  if (currentNode == nullptr) {
    cleanPtrsFrom(inputTokens, 0);
    return nullptr;
  }

  size_t i = 1;
  size_t inputSize = inputTokens.size();

  // Stop when we have consumed all the input and the root of grammar
  // is the only thing on the stack
  while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == Symbol::S)) {
    size_t reduceStart;
    Symbol nextInputToken = i == inputSize ? Symbol::EPSILON : inputTokens[i].symbol;
    Concrete type = tryReduce(currentNode, nextInputToken, stk, &reduceStart);
    // Reduce
    if (type != Concrete::NONE) {
      // Construct the new object, pop the arguments off the stack,
      // and push the new object onto it.
      StackObj newObj = construct(type, &stk.data()[reduceStart]);
      if (newObj.symbol == Symbol::S) {
        stk.pop_back();
      } else {
        size_t stkSize = stk.size();
        for (size_t j = 0; j < stkSize - reduceStart; ++j) {
          stk.back().shallowDeleteObj();
          stk.pop_back();
        }
      }

      stk.push_back(newObj);

      // Restart the DFA.
      // TODO: Only backtrack as far as the reduction (store path)
      vector<Symbol> stkSymbols;
      transform(stk.begin(), stk.end(), back_inserter(stkSymbols), [](StackObj stkObj) {
        return stkObj.symbol;
      });
      currentNode = dfa.run(stkSymbols);
    } else {  // Shift
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

  // Remove the actual grammar root from the fake root we encapsulated it with
  Start* start = (Start*)(stk[0].obj);
  ROOT_TYPE root = *start->r_;
  delete start;
  return root;
}

#endif
