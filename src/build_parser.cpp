#include "build_parser.hpp"

using namespace std;
using DFA_t = DFA<DFARuleSet, int>;
using BitSetToks = vector<bool>;

namespace {

  /* Given a rule "A -> .BC", add all "B -> .<rhs>" rules to rule queue,
   * updating the lookahead set */
  void addRhses(
      queue<DFARule>& ruleQueue,
      const DFARule& fromRule,
      const Grammar& grammar,
      const vector<BitSetToks>& firsts) {
    // Nothing to expand if we are at the end of the rule or if the next symbol
    // is a token
    int nextSymbol = fromRule.nextSymbol();
    if (nextSymbol == NONE || isToken(nextSymbol)) {
      return;
    }

    // Construct the new lookahead set, starting with the lookahead set of
    // the rule from which we are expanding
    BitSetToks newLookahead = fromRule.lookahead;
    int nextNextSymbol = fromRule.nextNextSymbol();
    if (nextNextSymbol != NONE) {
      // FIRST() of a token is just the token
      if (isToken(nextNextSymbol)) {
        newLookahead[tokensIndex(nextNextSymbol)] = true;
      } else {  // Union with the first of the nextNextSymbol
        bitOrEquals(newLookahead, firsts[nextNextSymbol]);
      }
    }

    const vector<GrammarRule>& rules = grammar[nextSymbol];
    for (const GrammarRule& rule : rules) {
      ruleQueue.push(DFARule{ rule.concrete, rule.symbols, 0, newLookahead });
    }
  }


  /* Adds possible rules to node's state via epsilon transition in DFA.
   * Ex: S -> A.B, then add all rules B -> ??? */
  void epsilonTransition(
      DFARuleSet& ruleSet,
      const Grammar& grammar,
      const vector<BitSetToks>& firsts) {
    queue<DFARule> ruleQueue;

    // Expand variables (epsilon transition) in the initial set of rules.
    for (const DFARule& rule : ruleSet) {
      addRhses(ruleQueue, rule, grammar, firsts);
    }

    // Keep expanding variables (epsilon transition) until we've determined all
    // the possible rule positions we could be in.
    while (!ruleQueue.empty()) {
      DFARule& rule = ruleQueue.front();
      auto iter = ruleSet.find(rule);
      // If rule is not yet in the set, add it

      // TODO: Should we check for duplicate rules in the queue as well (like
      // use a set and just pop with set.begin())
      if (iter == ruleSet.end()) {
        addRhses(ruleQueue, rule, grammar, firsts);
        ruleSet.insert(move(rule));
        ruleQueue.pop();
        continue;
      }

      // If rule is already in the set, check to see if the new rule has any new
      // members its lookahead set. If so, replace existing rule with new one.
      const DFARule& existingRule = *iter;
      BitSetToks unionToks = bitOr(rule.lookahead, existingRule.lookahead);
      if (existingRule.lookahead != unionToks) {
        // TODO: Might be faster to just do operator|= again
        existingRule.lookahead = move(unionToks);
        // Only add RHSes if we insert the rule into the set because everything
        // in the set has already been expanded
        addRhses(ruleQueue, rule, grammar, firsts);
      }

      ruleQueue.pop();
    }
  }


  /* For each rule of this node, construct the transitions to successors.
   * Return the successors that were newly added */
  vector<const DFA_t::Node*> createTransitions(
      DFA_t& dfa,
      const DFA_t::Node* node,
      const Grammar& grammar,
      const vector<BitSetToks>& firsts,
      size_t numSymbols) {
    // Get all the valid transition symbols and map each of them to a new set of
    // rules
    vector<DFARuleSet> newTransitions(numSymbols);
    size_t numVars = grammar.size();

    for (const DFARule& rule : node->getValue()) {
      if (rule.atEnd()) {
        continue;
      }
      newTransitions[symbolIndex(rule.nextSymbol(), numVars)].insert(
          rule.nextStep());
    }

    // Apply epsilon transitions and create the transition
    vector<const DFA_t::Node*> addedNodes;
    for (size_t i = 0; i < numSymbols; ++i) {
      DFARuleSet& transitionRules = newTransitions[i];
      // No valid transition
      if (transitionRules.empty()) {
        continue;
      }
      epsilonTransition(transitionRules, grammar, firsts);
      const DFA_t::Node* newNode = dfa.addTransition(
          node, indexToSymbol(i, numVars), move(transitionRules));
      if (newNode) {
        addedNodes.push_back(newNode);
      }
    }
    return addedNodes;
  }


  /* Constructs the starting node of the DFA */
  DFA_t initDFA(
      const Grammar& grammar,
      const vector<BitSetToks>& firsts,
      size_t numTokens) {
    DFARuleSet firstSet = { DFARule{
        SCONC, { grammar[S][0].symbols[0] }, 0, BitSetToks(numTokens) } };
    epsilonTransition(firstSet, grammar, firsts);
    DFA_t dfa(move(firstSet));
    return dfa;
  }
}  // namespace


/* Build the DFA */
DFA_t buildParserDFA(const Grammar& grammar, size_t numTokens) {
  size_t numSymbols = numTokens + grammar.size();
  vector<BitSetToks> firsts = getFirsts(grammar, numTokens);
  DFA_t dfa = initDFA(grammar, firsts, numTokens);
  queue<const DFA_t::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    const DFA_t::Node* node = q.front();
    q.pop();
    vector<const DFA_t::Node*> addedNodes =
        createTransitions(dfa, node, grammar, firsts, numSymbols);
    for (const DFA_t::Node* newNode : addedNodes) {
      q.push(newNode);
    }
  }

  return dfa;
}
