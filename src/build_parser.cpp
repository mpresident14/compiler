#include "build_parser.hpp"
#include "null_first.hpp"

#include <cstddef>
#include <string>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <sstream>

#include <prez/print_stuff.hpp>

using namespace std;
using DFA_t = DFA<DFARuleSet, int>;
using BitSetToks = vector<bool>;

namespace {

  /* Given a rule "A -> .BC", add all "B -> .<rhs>" rules to rule queue,
   * updating the lookahead set */
  void addRhses(
      queue<DFARule>& ruleQueue,
      const DFARule& fromRule,
      const GrammarData& grammarData,
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
        newLookahead[tokenToFromIndex(nextNextSymbol)] = true;
      } else {  // Union with the first of the nextNextSymbol
        bitOrEquals(newLookahead, firsts[nextNextSymbol]);
      }
    }

    for (int concreteType : grammarData.variables[nextSymbol].concreteTypes) {
      ruleQueue.push(DFARule{ concreteType,
                              grammarData.concretes[concreteType].argSymbols,
                              0,
                              newLookahead });
    }
  }


  /* Adds possible rules to node's state via epsilon transition in DFA.
   * Ex: S -> A.B, then add all rules B -> ??? */
  void epsilonTransition(
      DFARuleSet& ruleSet,
      const GrammarData& grammarData,
      const vector<BitSetToks>& firsts) {
    queue<DFARule> ruleQueue;

    // Expand variables (epsilon transition) in the initial set of rules.
    for (const DFARule& rule : ruleSet) {
      addRhses(ruleQueue, rule, grammarData, firsts);
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
        addRhses(ruleQueue, rule, grammarData, firsts);
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
        addRhses(ruleQueue, rule, grammarData, firsts);
      }

      ruleQueue.pop();
    }
  }


  /* For each rule of this node, construct the transitions to successors.
   * Return the successors that were newly added */
  vector<DFA_t::Node*> createTransitions(
      DFA_t& dfa,
      DFA_t::Node* node,
      const GrammarData& grammarData,
      const vector<BitSetToks>& firsts) {
    // Get all the valid transition symbols and map each of them to a new set of
    // rules
    size_t numVars = grammarData.variables.size();
    size_t numSymbols = grammarData.tokens.size() + numVars;
    vector<DFARuleSet> newTransitions(numSymbols);


    for (const DFARule& rule : node->getValue()) {
      if (rule.atEnd()) {
        continue;
      }
      newTransitions[symbolIndex(rule.nextSymbol(), numVars)].insert(
          rule.nextStep());
    }

    // Apply epsilon transitions and create the transition
    vector<DFA_t::Node*> addedNodes;
    for (size_t i = 0; i < numSymbols; ++i) {
      DFARuleSet& transitionRules = newTransitions[i];
      // No valid transition
      if (transitionRules.empty()) {
        continue;
      }
      epsilonTransition(transitionRules, grammarData, firsts);
      DFA_t::Node* newNode = dfa.addTransition(
          node, indexToSymbol(i, numVars), move(transitionRules));
      if (newNode) {
        addedNodes.push_back(newNode);
      }
    }
    return addedNodes;
  }


  /* Constructs the starting node of the DFA */
  DFA_t initDFA(
      const GrammarData& grammarData,
      const vector<BitSetToks>& firsts) {
    int rootType = grammarData.variables[S].concreteTypes[0];
    DFARuleSet firstSet = { DFARule{ SCONC,
                                     grammarData.concretes[rootType].argSymbols,
                                     0,
                                     BitSetToks(grammarData.tokens.size()) } };
    epsilonTransition(firstSet, grammarData, firsts);
    DFA_t dfa(move(firstSet));
    return dfa;
  }
}  // namespace


/* Build the DFA */
DFA_t buildParserDFA(const GrammarData& grammarData) {
  vector<BitSetToks> firsts = getFirsts(grammarData);
  DFA_t dfa = initDFA(grammarData, firsts);
  queue<DFA_t::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    DFA_t::Node* node = q.front();
    q.pop();
    vector<DFA_t::Node*> addedNodes =
        createTransitions(dfa, node, grammarData, firsts);
    for (DFA_t::Node* newNode : addedNodes) {
      q.push(newNode);
    }
  }

  return dfa;
}

void streamRule(std::ostream& out, const DFARule& rule, const GrammarData& grammarData) {
  const vector<Variable>& variables = grammarData.variables;
  const vector<Token>& tokens = grammarData.tokens;

  out << grammarData.concretes[rule.concrete].name << " -> ";
  size_t len = rule.symbols.size();
  for (size_t i = 0; i < len; ++i) {
    if (i == rule.pos) {
      out << '.';
    }
    int symbol = rule.symbols[i];
    if (isToken(symbol)) {
      out << tokens[tokenToFromIndex(symbol)].name << ' ';
    } else {
      out << variables[symbol].name << ' ';
    }
  }
  if (rule.pos == len) {
    out << '.';
  }
  vector<string> lookaheadNames;
  for (size_t i = 0; i < rule.lookahead.size(); ++i) {
    if (rule.lookahead[i]) {
      lookaheadNames.push_back(tokens[i].name);
    }
  }
  out << " :: " << lookaheadNames;
}

void shiftReduceConflict(const DFARule& reduceRule, const DFARule& shiftRule, const GrammarData& grammarData) {
  cerr << "WARNING: Shift-reduce conflict for rules\n\t";
  streamRule(cerr, reduceRule, grammarData);
  cerr << "\n\t";
  streamRule(cerr, shiftRule, grammarData);
  cerr << endl;
}

void reduceReduceConflict(const DFARule& reduceRule1, const DFARule& reduceRule2, const GrammarData& grammarData) {
  cerr << "WARNING: Shift-reduce conflict for rules\n\t";
  streamRule(cerr, reduceRule1, grammarData);
  cerr << "\n\t";
  streamRule(cerr, reduceRule2, grammarData);
  cerr << endl;
}


void findConflicts(
    const DFARule& reducibleRule,
    int rulePrecedence,
    Assoc lastTokenAssoc,
    const DFARuleSet& ruleSet,
    const GrammarData& grammarData) {

  const vector<Token>& tokens = grammarData.tokens;
  for (const DFARule& rule : ruleSet) {
    // Already found reducible rules
    if (rule.atEnd()) {
      continue;
    }
    int nextSymbol = rule.nextSymbol();
    // No conflicts for variables
    if (!isToken(nextSymbol)) {
      continue;
    }
    int nextTokenIndex = tokenToFromIndex(nextSymbol);
    // No conflict if the next input token is not in the lookahead set (b/c can't reduce)
    if (!reducibleRule.lookahead[nextTokenIndex]) {
      continue;
    }

    int shiftPrecedence = tokens[nextTokenIndex].precedence;
    // Unspecified precedence -> shift-reduce conflict! (Will be resolved by shifting)
    if (rulePrecedence == NONE && shiftPrecedence == NONE) {
      shiftReduceConflict(reducibleRule, rule, grammarData);
    } else if (rulePrecedence == shiftPrecedence && lastTokenAssoc == Assoc::NONE) {
      // Same precedence and unspecified associativity -> shift-reduce conflict!
      // (Will be resolved by shifting)
      shiftReduceConflict(reducibleRule, rule, grammarData);
    }
  }
}


RuleData condenseRuleSet(
    const DFARuleSet& ruleSet,
    const GrammarData& grammarData) {

  const DFARule* reducibleRule = nullptr;
  for (const DFARule& rule : ruleSet) {
    if (rule.atEnd()) {
      // Reduce-reduce conflict! (Will be resolved by first come, first served)
      if (reducibleRule) {
        reduceReduceConflict(*reducibleRule, rule, grammarData);
      } else {
        reducibleRule = &rule;
      }
    }
  }

  // No reducible rules
  if (reducibleRule == nullptr) {
    return RuleData{ {}, NONE, Assoc::NONE };
  }

  // Find the last token, if any
  int rulePrecedence = grammarData.concretes[reducibleRule->concrete].precedence;
  auto ruleIter =
      find_if(reducibleRule->symbols.crbegin(), reducibleRule->symbols.crend(), isToken);
  // If no override precedence and the rule has a token, check precedence of last token
  int lastToken = NONE;
  if (rulePrecedence == NONE && ruleIter != reducibleRule->symbols.crend()) {
    lastToken = *ruleIter;
    rulePrecedence = grammarData.tokens[tokenToFromIndex(lastToken)].precedence;
  }

  // Check for shift-reduce conflicts
  Assoc lastTokenAssoc = lastToken == NONE ? Assoc::NONE : grammarData.tokens[lastToken].assoc;
  findConflicts(*reducibleRule, rulePrecedence, lastTokenAssoc, ruleSet, grammarData);

  // Reducible rule contains no tokens
  if (lastToken == NONE) {
    return RuleData{ optional(*reducibleRule), rulePrecedence, Assoc::NONE };
  }

  // If there is rule precedence, get associativity
  if (rulePrecedence == NONE) {
    return RuleData{ optional(*reducibleRule), NONE, Assoc::NONE };
  } else {
    return RuleData{ optional(*reducibleRule),
                     rulePrecedence,
                     lastTokenAssoc };
  }
}


namespace {
  // TODO: Replace vector code with toCode()
  string ruleDataToCode(const RuleData& ruleData) {
    stringstream code;
    code << "RuleData{";
    if (ruleData.reducibleRule.has_value()) {
      const DFARule& rule = *ruleData.reducibleRule;
      // RuleData::reducibleRule::concrete
      code << "optional<DFARule>{{" << to_string(rule.concrete) << ',';

      // RuleData::reducibleRule::symbols
      code << '{';
      for_each(rule.symbols.cbegin(), rule.symbols.cend(), [&code](int n) {
        code << to_string(n) << ',';
      });
      code << "},";

      // RuleData::reducibleRule::pos
      code << to_string(rule.pos) << ',';

      // RuleData::reducibleRule::lookahead
      code << '{';
      for_each(rule.lookahead.cbegin(), rule.lookahead.cend(), [&code](bool b) {
        code << to_string(b) << ',';
      });
      code << "}}},";
    } else {
      code << "optional<DFARule>{},";
    }

    // RuleData::precedence
    code << to_string(ruleData.precedence) << ',';

    // RuleData::assoc
    code << "Assoc::" << ruleData.assoc << '}';

    return code.str();
  }
}  // namespace


void condensedDFAToCode(ostream& out, const GrammarData& grammarData) {
  buildParserDFA(grammarData)
      .streamAsCode(
          out,
          "RuleData",
          "int",
          [&grammarData](const DFARuleSet& ruleSet) {
            return condenseRuleSet(ruleSet, grammarData);
          },
          ruleDataToCode,
          [](int n) { return to_string(n); });
}
