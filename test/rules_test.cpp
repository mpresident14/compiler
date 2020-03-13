#include "parse.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testAddRhses() {
  RuleSet ruleSet;
  addRhses(ruleSet, Symbol::EXPR);

  const Rule expected0 = {Concrete::EINT, {Symbol::INT}, 0};
  const Rule expected1 = {Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(2, ruleSet.size());
  tester.assertTrue(ruleSet.contains(expected0));
  tester.assertTrue(ruleSet.contains(expected1));
}

void testEpsilonTransition() {
  const Rule init = {Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  RuleSet ruleSet = {init};
  DFA_t dfa(ruleSet);
  epsilonTransition(ruleSet);

  const Rule expected0 = {Concrete::EINT, {Symbol::INT}, 0};
  const Rule expected1 = {Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(3, ruleSet.size());
  tester.assertTrue(ruleSet.contains(init));
  tester.assertTrue(ruleSet.contains(expected0));
  tester.assertTrue(ruleSet.contains(expected1));
}

void testInitDFA() {
  DFA_t initialNfa = initDFA();
  const RuleSet& ruleSet = initialNfa.getRoot()->getValue();

  const Rule expectedRule0 = {Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule expectedRule1 = {Concrete::EINT, {Symbol::INT}, 0};
  const Rule expectedRule2 = {Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(3, ruleSet.size());
  tester.assertTrue(ruleSet.contains(expectedRule0));
  tester.assertTrue(ruleSet.contains(expectedRule1));
  tester.assertTrue(ruleSet.contains(expectedRule2));
}

void testCreateTransitions() {
  const Rule rule0 = {Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule rule1 = {Concrete::EINT, {Symbol::INT}, 0};
  const Rule rule2 = {Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};
  DFA_t dfa({rule0, rule1, rule2});
  createTransitions(dfa, dfa.getRoot());

  const auto& transitions = dfa.getRoot()->getTransitions();
  const RuleSet& ruleSetExpr = transitions.at(Symbol::EXPR)->getValue();
  const RuleSet& ruleSetInt = transitions.at(Symbol::INT)->getValue();
  const Rule expectedExpr0 = {Concrete::SEXPR, {Symbol::EXPR, Symbol::DOLLAR}, 1};
  const Rule expectedInt0 = {Concrete::EPLUS, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 1};
  const Rule expectedInt1 = {Concrete::EINT, {Symbol::INT}, 1};

  tester.assertEquals(2, transitions.size());
  tester.assertEquals(1, ruleSetExpr.size());
  tester.assertTrue(ruleSetExpr.contains(expectedExpr0));
  tester.assertEquals(2, ruleSetInt.size());
  tester.assertTrue(ruleSetInt.contains(expectedInt0));
  tester.assertTrue(ruleSetInt.contains(expectedInt1));
}

void testCreateTransitionsEndRule() {
  const Rule rule = {Concrete::SEXPR, {Symbol::INT}, 1};
  DFA_t dfa({rule});
  createTransitions(dfa, dfa.getRoot());

  auto& transitions = dfa.getRoot()->getTransitions();
  tester.assertEquals(0, transitions.size());
}

int main(int, char**) {
  testAddRhses();
  testEpsilonTransition();
  testInitDFA();
  testCreateTransitions();
  testCreateTransitionsEndRule();

  return 0;
}
