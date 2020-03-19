#include "parse.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

int eval(Term* tInt) { return ((TInt*)tInt)->i_; }
int eval(Expr* expr) {
  switch (expr->getType()) {
    case Concrete::ETERM:
      return eval(((ETerm*)expr)->t_);
    case Concrete::EPLUS:
      return eval(((EPlus*)expr)->t_) + eval(((EPlus*)expr)->e_);
    default:
      return -1;
  }
}

void testAddRhses() {
  RuleSet ruleSet;
  addRhses(ruleSet, Symbol::EXPR);

  const DFARule expected0 = {Concrete::ETERM, {Symbol::TERM}, 0};
  const DFARule expected1 = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 0};

  TESTER.assertEquals(2, ruleSet.size());
  TESTER.assertTrue(ruleSet.contains(expected0));
  TESTER.assertTrue(ruleSet.contains(expected1));
}

void testEpsilonTransition() {
  const DFARule init1 = {Concrete::ETERM, {Symbol::TERM}, 0};
  const DFARule init2 = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 0};
  RuleSet ruleSet = {init1, init2};
  epsilonTransition(ruleSet);

  const DFARule expected = {Concrete::TINT, {Symbol::INT}, 0};

  TESTER.assertEquals(3, ruleSet.size());
  TESTER.assertTrue(ruleSet.contains(init1));
  TESTER.assertTrue(ruleSet.contains(init2));
  TESTER.assertTrue(ruleSet.contains(expected));
}

void testInitDFA() {
  DFA_t initialNfa = initDFA();
  const RuleSet& ruleSet = initialNfa.getRoot()->getValue();

  const DFARule expected0 = {Concrete::ETERM, {Symbol::TERM}, 0};
  const DFARule expected1 = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 0};
  const DFARule expected2 = {Concrete::TINT, {Symbol::INT}, 0};

  TESTER.assertEquals(3, ruleSet.size());
  TESTER.assertTrue(ruleSet.contains(expected0));
  TESTER.assertTrue(ruleSet.contains(expected1));
  TESTER.assertTrue(ruleSet.contains(expected2));
}

void testCreateTransitions() {
  const DFARule rule0 = {Concrete::TINT, {Symbol::INT}, 0};
  const DFARule rule1 = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 2};

  DFA_t dfa({rule0, rule1});
  createTransitions(dfa, dfa.getRoot());
  const auto& transitions = dfa.getRoot()->getTransitions();
  const RuleSet& ruleSet0 = transitions.at(Symbol::INT)->getValue();
  const RuleSet& ruleSet1 = transitions.at(Symbol::TERM)->getValue();

  const DFARule expected0 = {Concrete::TINT, {Symbol::INT}, 1};
  const DFARule expected1 = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 3};

  TESTER.assertEquals(2, transitions.size());
  TESTER.assertEquals(1, ruleSet0.size());
  TESTER.assertTrue(ruleSet0.contains(expected0));
  TESTER.assertEquals(1, ruleSet1.size());
  TESTER.assertTrue(ruleSet1.contains(expected1));
}

void testCreateTransitionsEndRule() {
  const DFARule rule = {Concrete::EPLUS, {Symbol::EXPR, Symbol::PLUS, Symbol::TERM}, 3};
  DFA_t dfa({rule});
  createTransitions(dfa, dfa.getRoot());
  auto& transitions = dfa.getRoot()->getTransitions();

  TESTER.assertEquals(0, transitions.size());
}

void testShiftReduce() {
  DFA_t dfa = buildDFA();
  auto expr0 = parse(dfa, {StackObj{new Int("1"), Symbol::INT, Concrete::NONE}});
  auto expr1 = parse(dfa,
      {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("2"), Symbol::INT, Concrete::NONE}});
  auto expr2 = parse(dfa,
      {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("2"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});

  auto noParse0 = parse(dfa,
      {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE}});
  auto noParse1 = parse(dfa,
      {StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("2"), Symbol::INT, Concrete::NONE}});
  auto noParse2 = parse(dfa,
      {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
          StackObj{new Int("2"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});
  auto noParse3 = parse(dfa,
      {StackObj{new Int("1"), Symbol::INT, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Plus(), Symbol::PLUS, Concrete::NONE},
          StackObj{new Int("50"), Symbol::INT, Concrete::NONE}});

  TESTER.assertEquals(1, eval(expr0.get()));
  TESTER.assertEquals(3, eval(expr1.get()));
  TESTER.assertEquals(53, eval(expr2.get()));

  TESTER.assertEquals(nullptr, noParse0);
  TESTER.assertEquals(nullptr, noParse1);
  TESTER.assertEquals(nullptr, noParse2);
  TESTER.assertEquals(nullptr, noParse3);
}

int main() {
  testAddRhses();
  testEpsilonTransition();
  testInitDFA();
  testCreateTransitions();
  testCreateTransitionsEndRule();
  testShiftReduce();

  return 0;
}
