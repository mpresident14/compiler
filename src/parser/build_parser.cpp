#include "src/parser/build_parser.hpp"

#include "src/misc/logger.hpp"
#include "src/parser/null_first.hpp"
#include "src/parser/queue_set.hpp"

#include <errno.h>
#include <string.h>

#include <cstddef>
#include <fstream>
#include <future>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/container_hash/hash.hpp>
#include <boost/dynamic_bitset.hpp>
#include <prez/print_stuff.hpp>
#include <prez/timeit.hpp>

using namespace std;
using DFA_t = DFA<DFARuleSet, int, DFARuleSetHash>;
using BitSetToks = boost::dynamic_bitset<>;
using BitSetVars = boost::dynamic_bitset<>;

namespace {

/***********
 * Logging *
 ***********/

void streamRule(std::ostream& out, const DFARule& rule, const GrammarData& gd) {
  const vector<Token>& tokens = gd.tokens;

  out << gd.concretes[rule.concrete].name << " -> ";
  size_t len = rule.symbols.size();
  for (size_t i = 0; i < len; ++i) {
    if (i == rule.pos) {
      out << '.';
    }
    int symbol = rule.symbols[i];
    out << symName(symbol, gd) << ' ';
  }
  if (rule.pos == len) {
    out << '.';
  }
  vector<string> lookaheadNames;
  if (rule.lookahead[0]) {
    lookaheadNames.push_back("EPSILON");
  }
  for (size_t i = 1; i < rule.lookahead.size(); ++i) {
    if (rule.lookahead[i]) {
      lookaheadNames.push_back(tokens[i - 1].name);
    }
  }
  out << " :: " << lookaheadNames;
}


void printNullabilities(std::ostream& out, const BitSetVars& nullabilities, const GrammarData& gd) {
  vector<string> nullVarNames;
  for (size_t j = 0; j < nullabilities.size(); ++j) {
    if (nullabilities[j]) {
      nullVarNames.push_back(gd.variables[j].name);
    }
  }
  out << "*************\n"
      << "* NULLABLES *\n"
      << "*************\n"
      << nullVarNames << "\n\n\n";
}

void printFirsts(std::ostream& out, const vector<BitSetToks>& firsts, const GrammarData& gd) {
  out << "**********\n"
      << "* FIRSTS *\n"
      << "**********\n";
  for (size_t i = 0; i < firsts.size(); ++i) {
    vector<string> lookaheadNames;
    for (size_t j = 0; j < firsts[i].size(); ++j) {
      if (firsts[i][j]) {
        lookaheadNames.push_back(gd.tokens[j].name);
      }
    }
    out << gd.variables[i].name << ": " << lookaheadNames << "\n\n";
  }
  out << '\n';
}


void printDfa(std::ostream& out, const DFA_t& dfa, const GrammarData& gd) {
  using Node = DFA_t::Node;

  out << "**********\n"
      << "* STATES *\n"
      << "**********\n\n";

  unordered_map<const Node*, size_t> nodeNumMap;
  std::queue<const Node*> q;
  q.push(dfa.getRoot());
  std::unordered_set<const Node*> visited = { dfa.getRoot() };
  size_t stateNum = 0;
  nodeNumMap.emplace(dfa.getRoot(), stateNum++);
  while (!q.empty()) {
    const Node* node = q.front();
    q.pop();
    out << "-----------\n";
    out << "- State " << nodeNumMap.at(node) << '\n';
    out << "-----------\n";
    for (const DFARule& rule : node->getValue()) {
      streamRule(out, rule, gd);
      out << "\n\n";
    }
    out << '\n';
    for (const auto& [trans, successor] : node->getTransitions()) {
      bool notVisited = visited.insert(successor).second;
      if (notVisited) {
        nodeNumMap.emplace(successor, stateNum++);
        q.push(successor);
      }
      out << "[" << symName(trans, gd) << "] -> State " << nodeNumMap.at(successor) << '\n';
    }
    out << "\n\n\n";
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
void addIfNewRule(DFARule&& rule, QueueSet<const DFARule*>& ruleQueue, DFARuleSet& ruleSet) {
  auto p = ruleSet.insert(rule);
  const DFARule& ruleRef = *p.first;
  if (p.second) {
    ruleQueue.push(&ruleRef);
  } else if (!rule.lookahead.is_subset_of(ruleRef.lookahead)) {
    ruleRef.lookahead |= rule.lookahead;
    ruleQueue.push(&ruleRef);
  }
}

/*
 * Given a rule "A -> .BC", add all new (subject to enqueueRuleIfNew())
 * "B -> .<rhs>" rules to ruleSet and ruleQueue,
 */
void addRhses(
    const DFARule& fromRule,
    QueueSet<const DFARule*>& ruleQueue,
    DFARuleSet& ruleSet,
    const GrammarData& gd,
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
    addRhses(fromRule.nextStep(), ruleQueue, ruleSet, gd, nulls, firsts);
  }

  // Construct the lookahead set for the new rules, include EPSILON
  BitSetToks newLookahead(gd.tokens.size() + 1);
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
      newLookahead[lookaheadInd(symbol)] = true;
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

  for (int concreteType : gd.variables[nextSymbol].concreteTypes) {
    addIfNewRule(
        { concreteType, gd.concretes[concreteType].argSymbols, 0, newLookahead },
        ruleQueue,
        ruleSet);
  }
}


/*
 * Adds possible rules to node's state (ruleSet) via epsilon transition in
 * DFA. Ex: S -> A.B, then add all rules B -> ???
 */
void epsilonTransition(
    DFARuleSet& ruleSet,
    const GrammarData& gd,
    const BitSetVars& nulls,
    const vector<BitSetToks>& firsts) {
  QueueSet<const DFARule*> ruleQueue;

  // Add the initial set of rules.
  for (const DFARule& rule : ruleSet) {
    ruleQueue.push(&rule);
  }

  // Keep expanding variables (epsilon transition) until we've determined all
  // the possible rule positions we could be in.
  while (!ruleQueue.empty()) {
    addRhses(*ruleQueue.front(), ruleQueue, ruleSet, gd, nulls, firsts);
    ruleQueue.pop();
  }
}

/* For each rule of this node, construct the transitions to successors.
 * Return the successors that were newly added */
vector<DFA_t::Node*> createTransitions(
    DFA_t& dfa,
    DFA_t::Node* node,
    const GrammarData& gd,
    const BitSetVars& nulls,
    const vector<BitSetToks>& firsts) {
  // Get all the valid transition symbols and map each of them to a new set of
  // rules
  size_t numVars = gd.variables.size();
  size_t numSymbols = gd.tokens.size() + numVars;
  vector<DFARuleSet> newTransitions(numSymbols);

  for (const DFARule& rule : node->getValue()) {
    if (rule.atEnd()) {
      continue;
    }
    newTransitions[symToArrInd(rule.nextSymbol(), numVars)].insert(rule.nextStep());
  }

  // Apply epsilon transitions and create the transition
  // Do the transitions for each symbol in parallel
  vector<DFA_t::Node*> addedNodes;
  mutex mtx;
  auto job = [node, &mtx, &addedNodes, &gd, &nulls, &firsts, &dfa](
                 DFARuleSet* transitionRules, size_t symbolIndex) {
    epsilonTransition(*transitionRules, gd, nulls, firsts);

    mtx.lock();
    DFA_t::Node* newNode = dfa.addTransition(node, symbolIndex, move(*transitionRules));
    if (newNode) {
      addedNodes.push_back(newNode);
    }
    mtx.unlock();
  };

  vector<future<void>> eTransJobs;
  eTransJobs.reserve(numSymbols);
  for (size_t i = 0; i < numSymbols; ++i) {
    DFARuleSet& transitionRules = newTransitions[i];
    // Has a valid transition
    if (!transitionRules.empty()) {
      eTransJobs.push_back(async(job, &transitionRules, arrIndToSym(i, numVars)));
    }
  }

  for (auto& job : eTransJobs) {
    job.wait();
  }

  return addedNodes;
}

/* Constructs the starting node of the DFA */
DFA_t initDFA(const GrammarData& gd, const BitSetVars& nulls, const vector<BitSetToks>& firsts) {
  int rootType = gd.variables[S].concreteTypes[0];
  BitSetToks initLookahead(gd.tokens.size() + 1);
  initLookahead[EPSILON] = true;
  DFARuleSet firstSet = { DFARule{
      SCONC, gd.concretes[rootType].argSymbols, 0, std::move(initLookahead) } };
  epsilonTransition(firstSet, gd, nulls, firsts);
  DFA_t dfa(move(firstSet));
  return dfa;
}


}  // namespace


/* Build the DFA */
DFA_t buildParserDFA(const GrammarData& gd, const ParseFlags& parseFlags) {
  auto nullFirstsPair = getNullsAndFirsts(gd);
  const vector<BitSetToks>& firsts = nullFirstsPair.second;
  const BitSetVars& nulls = nullFirstsPair.first;

  DFA_t dfa = initDFA(gd, nulls, firsts);
  queue<DFA_t::Node*> q;
  q.push(dfa.getRoot());

  while (!q.empty()) {
    DFA_t::Node* node = q.front();
    q.pop();
    vector<DFA_t::Node*> addedNodes = createTransitions(dfa, node, gd, nulls, firsts);
    for (DFA_t::Node* newNode : addedNodes) {
      q.push(newNode);
    }
  }

  if (!parseFlags.logFile.empty()) {
    ofstream logStream(parseFlags.logFile);
    if (logStream.is_open()) {
      printNullabilities(logStream, nulls, gd);
      printFirsts(logStream, firsts, gd);
      printDfa(logStream, dfa, gd);
    } else {
      cerr << Logger::warningColored << ": could not open " << parseFlags.logFile
           << " for logging: " << strerror(errno) << endl;
    }
  }

  return dfa;
}


/*************************
 * Write the DFA as code *
 *************************/

/* CONFLICT RESOLUTION STRATEGY */

// Reduce-reduce conflicts are resolved by the rule with higher precedence*
// - If the precedences are equal, the first rule declared will be chosen and a warning will be
//   emitted.

// Shift-reduce conflicts are resolved in the following manner
// (from https://www.haskell.org/happy/doc/html/sec-Precedences.html):
// - If the precedence of the rule is higher, then the conflict is resolved
//   as a reduce.
// - If the precedence of the lookahead token is higher, then the conflict
//   is resolved as a shift.
// - If the precedences are equal, then
//    ~ If the token is left-associative, then reduce
//    ~ If the token is right-associative, then shift
//    ~ If the token is non-associative, then fail
// - If neither the rule nor the token have precedence, then the default is
//   to shift and a conflict will have been reported by findConflicts() in build_parser.cpp
//   (note that this criterion differs slightly from Happy)
//
// * A rule's precedence is it's override precedence if it exists, otherwise the precedence of its
//   last token

namespace {

using ConflictMap = unordered_map<pair<int, int>, string, boost::hash<pair<int, int>>>;

void shiftReduceConflict(
    const DFARule& shiftRule,
    const DFARule& reduceRule,
    const GrammarData& gd,
    ConflictMap& conflicts) {
  auto p = make_pair(shiftRule.concrete, reduceRule.concrete);
  if (conflicts.contains(p)) {
    return;
  }

  ostringstream warning;
  warning << Logger::warningColored << ": Shift-reduce conflict for rules\n\t";
  streamRule(warning, shiftRule, gd);
  warning << "\n\t";
  streamRule(warning, reduceRule, gd);
  warning << '\n';
  conflicts.emplace(move(p), warning.str());
}

void reduceReduceConflict(
    const DFARule& reduceRule1,
    const DFARule& reduceRule2,
    const GrammarData& gd,
    ConflictMap& conflicts) {
  // No need to repeat the warning if they are switched
  auto p = reduceRule1.concrete < reduceRule2.concrete
               ? make_pair(reduceRule1.concrete, reduceRule2.concrete)
               : make_pair(reduceRule2.concrete, reduceRule1.concrete);
  if (conflicts.contains(p)) {
    return;
  }

  ostringstream warning;
  warning << Logger::warningColored << ": Reduce-reduce conflict for rules\n\t";
  streamRule(warning, reduceRule1, gd);
  warning << "\n\t";
  streamRule(warning, reduceRule2, gd);
  warning << '\n';
  conflicts.emplace(move(p), warning.str());
}

template <typename Cmp>
void findReduceReduceConflicts(
    const DFARule* nextRule,
    std::multiset<const DFARule*, Cmp>& reducibleRules,
    const GrammarData& gd,
    ConflictMap& conflicts) {
  // We use a multiset so that we can compare with the highest precedences first
  for (const DFARule* redRule : reducibleRules) {
    // Not a reduce-reduce conflict if the lookahead sets are disjoint
    if (!nextRule->lookahead.intersects(redRule->lookahead)) {
      continue;
    }

    // If these rules conflict, pick one based on the criteria above
    int rulePrec = nextRule->getPrecedence(gd);
    int redRulePrec = redRule->getPrecedence(gd);
    if (rulePrec == redRulePrec) {
      reduceReduceConflict(*redRule, *nextRule, gd, conflicts);
      if (nextRule->concrete < redRule->concrete) {
        reducibleRules.erase(redRule);
        reducibleRules.insert(nextRule);
      }
    } else if (rulePrec > redRulePrec) {
      reducibleRules.erase(redRule);
      reducibleRules.insert(nextRule);
    }
    return;
  }

  // No conflicts with any rules in the set
  reducibleRules.insert(nextRule);
}

void findShiftReduceConflicts(
    const DFARule& reducibleRule,
    int rulePrecedence,
    const DFARuleSet& ruleSet,
    const GrammarData& gd,
    ConflictMap& conflicts) {
  const vector<Token>& tokens = gd.tokens;
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
    int nextTokenIndex = tokToArrInd(nextSymbol);
    // No conflict if the next input token is not in the lookahead set (b/c
    // can't reduce)
    if (!reducibleRule.lookahead[nextTokenIndex + 1]) {
      continue;
    }

    // Unspecified precedence -> shift-reduce conflict! (Will be resolved by
    // shifting)
    if (rulePrecedence == NONE && tokens[nextTokenIndex].precedence == NONE) {
      shiftReduceConflict(rule, reducibleRule, gd, conflicts);
    }
  }
}


struct RuleData {
  DFARule reducibleRule;
  int precedence;
};

/*
 * Remove the pieces of the ruleSet we do not need to actually run the DFA.
 * Also find any shift- or reduce-reduce conflicts
 */
vector<RuleData>
condenseRuleSet(const DFARuleSet& ruleSet, const GrammarData& gd, ConflictMap& conflicts) {
  auto ruleCmp = [&gd](const DFARule* r1, const DFARule* r2) {
    return r1->getPrecedence(gd) > r2->getPrecedence(gd);
  };
  std::multiset<const DFARule*, decltype(ruleCmp)> reducibleRules(ruleCmp);

  for (const DFARule& rule : ruleSet) {
    if (!rule.atEnd()) {
      continue;
    }
    findReduceReduceConflicts(&rule, reducibleRules, gd, conflicts);
  }

  vector<RuleData> ruleData;
  if (reducibleRules.empty()) {
    return ruleData;
  }

  // Check for shift-reduce conflicts
  for (const DFARule* redRule : reducibleRules) {
    int rulePrecedence = redRule->getPrecedence(gd);
    findShiftReduceConflicts(*redRule, rulePrecedence, ruleSet, gd, conflicts);
    ruleData.push_back({ *redRule, rulePrecedence });
  }

  return ruleData;
}

void rdToCode(ostream& code, const RuleData& ruleData) {
  code << "RuleData{";

  const DFARule& rule = ruleData.reducibleRule;
  // RuleData::reducibleRule::concrete
  code << "DFARule{" << to_string(rule.concrete) << ',';

  // RuleData::reducibleRule::symbols
  code << '{';
  for_each(
      rule.symbols.cbegin(), rule.symbols.cend(), [&code](int n) { code << to_string(n) << ','; });
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
  code << "}},";
  // RuleData::precedence
  code << to_string(ruleData.precedence) << '}';
}

string rdVecToCode(const vector<RuleData>& v) {
  ostringstream code;
  code << "vector<RuleData>{";
  for_each(v.cbegin(), v.cend(), [&code](const RuleData& rd) {
    rdToCode(code, rd);
    code << ',';
  });
  code << '}';
  return code.str();
}

}  // namespace


void condensedDFAToCode(ostream& out, const GrammarData& gd, const ParseFlags& parseFlags) {
  ConflictMap conflicts;
  buildParserDFA(gd, parseFlags)
      .streamAsCode(
          out,
          "vector<RuleData>",
          "int",
          [&gd, &conflicts](const DFARuleSet& ruleSet) {
            return rdVecToCode(condenseRuleSet(ruleSet, gd, conflicts));
          },
          [](int n) { return to_string(n); });

  for (const auto& [_, warning] : conflicts) {
    cerr << warning << endl;
  }
}
