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
  const Rule expected1 = {Symbol::EXPR, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(2, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected1));
}

void testEpsilonTransition() {
  const Rule init = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  NFA_t nfa({init});
  epsilonTransition(nfa.getRoot());

  const Rule expected0 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule expected1 = {Symbol::EXPR, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}, 0};
  const RuleList& ruleList = nfa.getRoot()->value_;

  tester.assertEquals(3, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), init));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expected1));
}

void testInitNFA() {
  NFA_t initialNfa = initNFA();
  const RuleList& ruleList = initialNfa.getRoot()->value_;

  const Rule expectedRule0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule expectedRule1 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule expectedRule2 = {Symbol::EXPR, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}, 0};

  tester.assertEquals(3, ruleList.size());
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule0));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule1));
  tester.assertEquals(1, count(ruleList.begin(), ruleList.end(), expectedRule2));
}

void testaddTransitions() {
  const Rule rule0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 0};
  const Rule rule1 = {Symbol::EXPR, {Symbol::INT}, 0};
  const Rule rule2 = {Symbol::EXPR, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}, 0};
  NFA_t nfa({rule0, rule1, rule2});
  addTransitions(nfa.getRoot());

  auto& transitions = nfa.getRoot()->transitions_;
  const RuleList& ruleListExpr = transitions[Symbol::EXPR]->value_;
  const RuleList& ruleListInt = transitions[Symbol::INT]->value_;
  const Rule expectedExpr0 = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 1};
  const Rule expectedExpr1 = {Symbol::EXPR, {Symbol::EXPR, Symbol::PLUS, Symbol::EXPR}, 1};
  const Rule expectedInt0 = {Symbol::EXPR, {Symbol::INT}, 1};

  tester.assertEquals(2, transitions.size());
  tester.assertEquals(2, ruleListExpr.size());
  tester.assertEquals(1, count(ruleListExpr.begin(), ruleListExpr.end(), expectedExpr0));
  tester.assertEquals(1, count(ruleListExpr.begin(), ruleListExpr.end(), expectedExpr1));
  tester.assertEquals(1, ruleListInt.size());
  tester.assertEquals(1, count(ruleListInt.begin(), ruleListInt.end(), expectedInt0));
}

void testaddTransitionsEndRule() {
  const Rule rule = {Symbol::EXPR, {Symbol::INT}, 1};
  NFA_t nfa({rule});
  addTransitions(nfa.getRoot());

  auto& transitions = nfa.getRoot()->transitions_;
  tester.assertEquals(0, transitions.size());
}

void testForError() {
  const Rule rule = {Symbol::STMT, {Symbol::EXPR, Symbol::DOLLAR}, 1};
  NFA_t nfa({rule});
  addTransitions(nfa.getRoot());

  auto& transitions = nfa.getRoot()->transitions_;
  for (auto& tpair : transitions) {
    cout << "BEFORE: " << *tpair.second << endl;
    epsilonTransition(tpair.second);
    cout << "AFTER: " << *tpair.second << endl;
  }
}


int main(int, char**) {
  // testAddRhses();
  // testEpsilonTransition();
  // testInitNFA();
  // testaddTransitions();
  // testaddTransitionsEndRule();
  testForError();

  return 0;
}
