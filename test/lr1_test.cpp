// parse.hpp relies on a grammar, so we have to include lr1_grammar.hpp before.
// This setup allows us to use arbitrary grammars for parse without getting
// multiple definitions of Symbol, Concrete, etc.
#include "test/lr1_grammar.hpp"
#include "parse.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

void testRuleEquality() {
  const DFARule rule0 = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("001") /* {PLUS} */};

  const DFARule rule1 = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};

  RuleSet ruleSet0 = {rule0};
  RuleSet ruleSet1 = {rule1};

  // See notes in rules.hpp for how and why this is a thing
  TESTER.assertTrue(ruleSet0.contains(rule1));
  TESTER.assertTrue(ruleSet1.contains(rule0));
  TESTER.assertNotEqual(ruleSet0, ruleSet1);
}

void testAddRhses() {
  queue<DFARule> ruleQueue;
  const DFARule fromRule = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("001") /* {PLUS} */};
  addRhses(ruleQueue, fromRule);

  const DFARule expected0 = {
      Concrete::EINT, {Symbol::INT}, 0, BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected1 = {Concrete::EPLUS,
      {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected2 = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};

  TESTER.assertEquals(3, ruleQueue.size());
  TESTER.assertEquals(expected0, ruleQueue.front());
  ruleQueue.pop();
  TESTER.assertEquals(expected1, ruleQueue.front());
  ruleQueue.pop();
  TESTER.assertEquals(expected2, ruleQueue.front());
  ruleQueue.pop();
}

void testEpsilonTransition() {
  const DFARule initRule = {Concrete::SCONC,
      {Symbol::EXPR},
      0,
      BitSetToks() /* {} */};
  RuleSet ruleSet = {initRule};
  epsilonTransition(ruleSet);

  const DFARule expected0 = {
      Concrete::EINT, {Symbol::INT}, 0, BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected1 = {Concrete::EPLUS,
      {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected2 = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};

  TESTER.assertEquals(4, ruleSet.size());
  // Can't just use contains because we have defined equality within
  // a RuleSet equality to ignore the lookahead
  TESTER.assertEquals(initRule, *ruleSet.find(initRule));
  TESTER.assertEquals(expected0, *ruleSet.find(expected0));
  TESTER.assertEquals(expected1, *ruleSet.find(expected1));
  TESTER.assertEquals(expected2, *ruleSet.find(expected2));
}


void testInitDFA() {
  DFA_t initialNfa = initDFA();
  const RuleSet& ruleSet = initialNfa.getRoot()->getValue();

  const DFARule initRule = {Concrete::SCONC,
      {Symbol::EXPR},
      0,
      BitSetToks() /* {} */};
  const DFARule expected0 = {
      Concrete::EINT, {Symbol::INT}, 0, BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected1 = {Concrete::EPLUS,
      {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};
  const DFARule expected2 = {Concrete::ETIMES,
      {Symbol::EXPR, Symbol::STAR, Symbol::EXPR},
      0,
      BitSetToks("011") /* {STAR, PLUS} */};

  TESTER.assertEquals(4, ruleSet.size());
  TESTER.assertEquals(initRule, *ruleSet.find(initRule));
  TESTER.assertEquals(expected0, *ruleSet.find(expected0));
  TESTER.assertEquals(expected1, *ruleSet.find(expected1));
  TESTER.assertEquals(expected2, *ruleSet.find(expected2));
}

// void testCreateTransitions() {
//   const DFARule rule0 = {Concrete::TINT, {Symbol::INT}, 0, BitSetToks()};
//   const DFARule rule1 = {
//       Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 2, BitSetToks()};

//   DFA_t dfa({rule0, rule1});
//   createTransitions(dfa, dfa.getRoot());
//   const auto& transitions = dfa.getRoot()->getTransitions();
//   const RuleSet& ruleSet0 = transitions.at(Symbol::INT)->getValue();
//   const RuleSet& ruleSet1 = transitions.at(Symbol::TERM)->getValue();

//   const DFARule expected0 = {Concrete::TINT, {Symbol::INT}, 1, BitSetToks()};
//   const DFARule expected1 = {
//       Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 3, BitSetToks()};

//   TESTER.assertEquals(2, transitions.size());
//   TESTER.assertEquals(1, ruleSet0.size());
//   TESTER.assertTrue(ruleSet0.contains(expected0));
//   TESTER.assertEquals(1, ruleSet1.size());
//   TESTER.assertTrue(ruleSet1.contains(expected1));
// }

// void testCreateTransitionsEndRule() {
//   const DFARule rule = {
//       Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 3, BitSetToks()};
//   DFA_t dfa({rule});
//   createTransitions(dfa, dfa.getRoot());
//   auto& transitions = dfa.getRoot()->getTransitions();

//   TESTER.assertEquals(0, transitions.size());
// }

// void testShiftReduce() {
//   DFA_t dfa = buildDFA();
//   auto expr0 = parse(dfa, {StackObj{new Int("1"), Symbol::INT, Concrete::NONE}});
//   auto expr1 = parse(dfa,
//       {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("2"), Symbol::INT, Concrete::NONE}});
//   auto expr2 = parse(dfa,
//       {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("2"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});

//   auto noParse0 = parse(dfa,
//       {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE}});
//   auto noParse1 = parse(dfa,
//       {StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("2"), Symbol::INT, Concrete::NONE}});
//   auto noParse2 = parse(dfa,
//       {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
//           StackObj{new Int("2"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});
//   auto noParse3 = parse(dfa,
//       {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
//           StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});

//   TESTER.assertEquals(1, eval(expr0.get()));
//   TESTER.assertEquals(3, eval(expr1.get()));
//   TESTER.assertEquals(53, eval(expr2.get()));

//   TESTER.assertEquals(nullptr, noParse0);
//   TESTER.assertEquals(nullptr, noParse1);
//   TESTER.assertEquals(nullptr, noParse2);
//   TESTER.assertEquals(nullptr, noParse3);
// }

int main() {
  testRuleEquality();
  testAddRhses();
  testEpsilonTransition();
  testInitDFA();
  // testCreateTransitions();
  // testCreateTransitionsEndRule();
  // testShiftReduce();

  return 0;
}
