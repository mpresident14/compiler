#include "src/parser/build_parser.hpp"

#include "src/parser/null_first.hpp"

#include <cstddef>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
#include <fstream>

#include <boost/dynamic_bitset.hpp>

#include <prez/print_stuff.hpp>

using namespace std;
using DFA_t = DFA<DFARuleSet, int, DFARuleSetHash>;
using BitSetToks = boost::dynamic_bitset<>;
using BitSetVars = boost::dynamic_bitset<>;

namespace {

  /***********
   * Logging *
   ***********/

  void streamRule(
      std::ostream& out,
      const DFARule& rule,
      const GrammarData& grammarData) {
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

  // TODO: Integrate this
  void printNullabilities(std::ostream& out, const BitSetVars& nullabilities, const GrammarData& gd) {
    vector<string> nullVarNames;
    for (size_t j = 0; j < nullabilities.size(); ++j) {
      if (nullabilities[j]) {
        nullVarNames.push_back(gd.variables[j].name);
      }
    }
    out << "NULLABILITIES:\n\n" << nullVarNames << "\n\n";
  }

  void printFirsts(std::ostream& out, const vector<BitSetToks>& firsts, const GrammarData& gd) {
    out << "FIRSTS" << endl;
    for (size_t i = 0; i < firsts.size(); ++i) {
      vector<string> lookaheadNames;
      for (size_t j = 0; j < firsts[i].size(); ++j) {
        if (firsts[i][j]) {
          lookaheadNames.push_back(gd.tokens[j].name);
        }
      }
      out << gd.variables[i].name << ": " << lookaheadNames << "\n\n";
    }
  }


  // TODO: Integrate this more nicely and add transitions
  void printDfa(std::ostream& out, const DFA_t& dfa, const GrammarData& gd) {
    using Node = DFA_t::Node;

    unordered_map<const Node*, size_t> nodeNumMap;

    std::queue<const Node*> q;
    q.push(dfa.getRoot());
    std::unordered_set<const Node*> visited = { dfa.getRoot() };
    size_t stateNum = 0;
    nodeNumMap.emplace(dfa.getRoot(), stateNum++);
    while (!q.empty()) {
      const Node* node = q.front();
      q.pop();
      out << "--------------\n";
      out << "STATE " << nodeNumMap.at(node) << '\n';
      out << "--------------\n";
      for (const DFARule& rule : node->getValue()) {
        streamRule(out, rule, gd);
        out << "\n\n";
      }
      out << '\n';
      for (const auto& [trans, successor] : node->getTransitions()) {
        if (!visited.contains(successor)) {
          nodeNumMap.emplace(successor, stateNum++);
          q.push(successor);
          visited.insert(successor);
        }
        out << '[' << symbolToString(trans, gd) << "] -> State " << nodeNumMap.at(successor) << '\n';
      }
      out << "\n\n";
    }
  }


  /********************
   * DFA Construction *
   ********************/

  /*
   * Adds to the ruleQueue and ruleSet and returns true if either
   * - the rule is not in the ruleSet yet
   * - the rule is in the ruleSet but has new tokens in its lookahead set
   * Since the queue is initially empty, using this function guarantees
   * that any rule in the queue is also in the set.
   */
  void addIfNewRule(DFARule&& rule, queue<DFARule>& ruleQueue, DFARuleSet& ruleSet) {
    auto iter = ruleSet.find(rule);
    if (iter == ruleSet.end()) {
      ruleSet.insert(rule);
      ruleQueue.push(move(rule));
    } else if (!rule.lookahead.is_subset_of(iter->lookahead)) {
      iter->lookahead |= rule.lookahead;
      ruleQueue.push(move(rule));
    }
  }

  /*
   * Given a rule "A -> .BC", add all new (subject to enqueueRuleIfNew())
   * "B -> .<rhs>" rules to ruleSet and ruleQueue,
   */
  void addRhses(
      const DFARule& fromRule,
      queue<DFARule>& ruleQueue,
      DFARuleSet& ruleSet,
      const GrammarData& grammarData,
      const BitSetVars& nulls,
      const vector<BitSetToks>& firsts) {
    // Nothing to expand if we are at the end of the rule or if the next symbol
    // is a token
    int nextSymbol = fromRule.nextSymbol();
    if (nextSymbol == NONE || isToken(nextSymbol)) {
      return;
    }

    // If the next variable is nullable, we need to consider the possibility
    // that we are on the nextNext variable
    if (nulls[nextSymbol]) {
      addRhses(fromRule.nextStep(), ruleQueue, ruleSet, grammarData, nulls, firsts);
    }

    // Construct the lookahead set for the new rules
    BitSetToks newLookahead(grammarData.tokens.size());
    // Start at the nextNextSymbol
    size_t i = fromRule.pos + 1;
    size_t ruleSize = fromRule.symbols.size();
    while (true) {
      // If we reached the end of the rule, then our lookahead set must
      // include the fromRule's lookahead set.
      if (i == ruleSize) {
        newLookahead |= fromRule.lookahead;
        break;
      }

      int symbol = fromRule.symbols[i];

      // Tokens are never nullable, so add it to the lookahead set and stop
      if (isToken(symbol)) {
        newLookahead[tokenToFromIndex(symbol)] = true;
        break;
      }

      // Add FIRSTS(var) to the lookahead
      newLookahead |= firsts[symbol];
      // If this variable is nullable, we need to continue through the rule
      // to see what other tokens could potentially come after this rule
      if (!nulls[symbol]) {
        break;
      }

      ++i;
    }

    for (int concreteType : grammarData.variables[nextSymbol].concreteTypes) {
      DFARule nextRule{ concreteType, grammarData.concretes[concreteType].argSymbols, 0, newLookahead };
      addIfNewRule(move(nextRule), ruleQueue, ruleSet);
    }
  }


  /*
   * Adds possible rules to node's state (ruleSet) via epsilon transition in
   * DFA. Ex: S -> A.B, then add all rules B -> ???
   */
  void epsilonTransition(
      DFARuleSet& ruleSet,
      const GrammarData& grammarData,
      const BitSetVars& nulls,
      const vector<BitSetToks>& firsts) {
    queue<DFARule> ruleQueue;

    // Expand variables (epsilon transition) in the initial set of rules.
    for (const DFARule& rule : ruleSet) {
      addRhses(rule, ruleQueue, ruleSet, grammarData, nulls, firsts);
    }

    // Keep expanding variables (epsilon transition) until we've determined all
    // the possible rule positions we could be in.
    while (!ruleQueue.empty()) {
      DFARule& rule = ruleQueue.front();
      addRhses(rule, ruleQueue, ruleSet, grammarData, nulls, firsts);
      ruleQueue.pop();
    }
  }

  /* For each rule of this node, construct the transitions to successors.
   * Return the successors that were newly added */
  vector<DFA_t::Node*> createTransitions(
      DFA_t& dfa,
      DFA_t::Node* node,
      const GrammarData& grammarData,
      const BitSetVars& nulls,
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
      epsilonTransition(transitionRules, grammarData, nulls, firsts);
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
      const BitSetVars& nulls,
      const vector<BitSetToks>& firsts) {
    int rootType = grammarData.variables[S].concreteTypes[0];
    DFARuleSet firstSet = { DFARule{ SCONC,
                                     grammarData.concretes[rootType].argSymbols,
                                     0,
                                     BitSetToks(grammarData.tokens.size()) } };
    epsilonTransition(firstSet, grammarData, nulls, firsts);
    DFA_t dfa(move(firstSet));
    return dfa;
  }


}  // namespace


/* Build the DFA */
DFA_t buildParserDFA(const GrammarData& grammarData) {
  auto nullFirstsPair = getNullsAndFirsts(grammarData);
  const vector<BitSetToks>& firsts = nullFirstsPair.second;
  const BitSetVars& nulls = nullFirstsPair.first;

  // ofstream desktop("/home/mpresident/Desktop/parse_info.log");
  // printNullabilities(desktop, nullabilities, grammarData);
  // printFirsts(desktop, firsts, grammarData);
  DFA_t dfa = initDFA(grammarData, nulls, firsts);
  queue<DFA_t::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    DFA_t::Node* node = q.front();
    q.pop();
    vector<DFA_t::Node*> addedNodes =
        createTransitions(dfa, node, grammarData, nulls, firsts);
    for (DFA_t::Node* newNode : addedNodes) {
      q.push(newNode);
    }
  }

  return dfa;
}


/*************************
 * Write the DFA as code *
 *************************/
namespace {

void shiftReduceConflict(
    const DFARule& reduceRule,
    const DFARule& shiftRule,
    const GrammarData& grammarData) {
  cerr << "WARNING: Shift-reduce conflict for rules\n\t";
  streamRule(cerr, reduceRule, grammarData);
  cerr << "\n\t";
  streamRule(cerr, shiftRule, grammarData);
  cerr << endl;
}

void reduceReduceConflict(
    const DFARule& reduceRule1,
    const DFARule& reduceRule2,
    const GrammarData& grammarData) {
  cerr << "WARNING: Reduce-reduce conflict for rules\n\t";
  streamRule(cerr, reduceRule1, grammarData);
  cerr << "\n\t";
  streamRule(cerr, reduceRule2, grammarData);
  cerr << endl;
}

void findShiftReduceConflicts(
    const DFARule& reducibleRule,
    int rulePrecedence,
    const DFARuleSet& ruleSet,
    const GrammarData& grammarData) {
  const vector<Token>& tokens = grammarData.tokens;
  for (const DFARule& rule : ruleSet) {
    // Already found reducible rules
    if (rule.atEnd()) {
      continue;
    }
    int nextSymbol = rule.nextSymbol();
    // Shift-reduce conflicts not possible for variables
    if (!isToken(nextSymbol)) {
      continue;
    }
    int nextTokenIndex = tokenToFromIndex(nextSymbol);
    // No conflict if the next input token is not in the lookahead set (b/c
    // can't reduce)
    if (!reducibleRule.lookahead[nextTokenIndex]) {
      continue;
    }

    // Unspecified precedence -> shift-reduce conflict! (Will be resolved by
    // shifting)
    if (rulePrecedence == NONE && tokens[nextTokenIndex].precedence == NONE) {
      shiftReduceConflict(reducibleRule, rule, grammarData);
    }
  }
}


struct RuleData {
  std::optional<DFARule> reducibleRule;
  int precedence;

  bool operator==(const RuleData rhs) const noexcept {
    return reducibleRule == rhs.reducibleRule;
  }
};

/*
 * Remove the pieces of the ruleSet we do not need to actually run the DFA.
 * Also find any shift- or reduce-reduce conflicts
 */
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
    return RuleData{ {}, NONE };
  }

  // Find the last token, if any
  int rulePrecedence =
      grammarData.concretes[reducibleRule->concrete].precedence;
  auto ruleIter = find_if(
      reducibleRule->symbols.crbegin(),
      reducibleRule->symbols.crend(),
      isToken);
  // If no override precedence and the rule has a token, check precedence of
  // last token
  int lastToken = NONE;
  if (rulePrecedence == NONE && ruleIter != reducibleRule->symbols.crend()) {
    lastToken = *ruleIter;
    rulePrecedence = grammarData.tokens[tokenToFromIndex(lastToken)].precedence;
  }

  // Check for shift-reduce conflicts
  findShiftReduceConflicts(
      *reducibleRule, rulePrecedence, ruleSet, grammarData);

  // Reducible rule contains no tokens
  if (lastToken == NONE) {
    return RuleData{ optional(*reducibleRule), rulePrecedence };
  }

  // If there is rule precedence, get associativity
  if (rulePrecedence == NONE) {
    return RuleData{ optional(*reducibleRule), NONE };
  } else {
    return RuleData{ optional(*reducibleRule), rulePrecedence };
  }
}


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
    const boost::dynamic_bitset<>& lookahead = rule.lookahead;
    size_t lookaheadLen = lookahead.size();
    for (size_t i = 0; i < lookaheadLen; ++i) {
      code << to_string(lookahead[i]) << ',';
    }
    code << "}}},";
  } else {
    code << "optional<DFARule>{},";
  }

  // RuleData::precedence
  code << to_string(ruleData.precedence) << '}';

  return code.str();
}


}  // namespace


void condensedDFAToCode(ostream& out, const GrammarData& grammarData) {
  auto dfa = buildParserDFA(grammarData);
  // ofstream desktop("/home/mpresident/Desktop/parse.log");
  // printDfa(desktop, dfa, grammarData);
      dfa.streamAsCode(
          out,
          "RuleData",
          "int",
          [&grammarData](const DFARuleSet& ruleSet) {
            return condenseRuleSet(ruleSet, grammarData);
          },
          ruleDataToCode,
          [](int n) { return to_string(n); });
}
