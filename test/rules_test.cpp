#include "rules.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testAddRhses() {
  RuleList ruleList;
  addRhses(ruleList, Symbol::EXPR);

  const Rule expected0 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule expected1 = {Symbol::EXPR, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(2, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected1));
}

void testEpsilonTransition() {
  const Rule init = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  RuleList ruleList = {init};
  DFA_t dfa(ruleList);
  epsilonTransition(ruleList);

  const Rule expected0 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule expected1 = {Symbol::EXPR, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(3, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), init));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected1));
}

void testInitDFA() {
  DFA_t initialNfa = initDFA();
  const RuleList& ruleList = initialNfa.getRoot()->getValue();

  const Rule expectedRule0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule expectedRule1 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule expectedRule2 = {Symbol::EXPR, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(3, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule1));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule2));
}

void testCreateTransitions() {
  const Rule rule0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule rule1 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule rule2 = {Symbol::EXPR, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 0};
  DFA_t dfa({rule0, rule1, rule2});
  createTransitions(dfa, dfa.getRoot());

  const auto& transitions = dfa.getRoot()->getTransitions();
  const RuleList& ruleListExpr = transitions.at(Symbol::EXPR)->getValue();
  const RuleList& ruleListInt = transitions.at(Symbol::INT)->getValue();
  const Rule expectedExpr0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 1};
  const Rule expectedInt0 = {Symbol::EXPR, {Symbol::INT, Symbol::PLUS, Symbol::EXPR}, 1};
  const Rule expectedInt1 = {Symbol::EXPR, {Symbol::INT}, 1};

  tester.assertEquals(2, transitions.size());
  tester.assertEquals(1, ruleListExpr.size());
  tester.assertEquals(1, count(ruleListExpr.begin(), ruleListExpr.end(), expectedExpr0));
  tester.assertEquals(2, ruleListInt.size());
  tester.assertEquals(1, count(ruleListInt.begin(), ruleListInt.end(), expectedInt0));
  tester.assertEquals(1, count(ruleListInt.begin(), ruleListInt.end(), expectedInt1));
}

void testCreateTransitionsEndRule() {
  const Rule rule = {Symbol::EXPR, {Symbol::INT}, 1};
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
