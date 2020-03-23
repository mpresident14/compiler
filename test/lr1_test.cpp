// ExprPtr(parse.hpp relies on a grammar, so we have to include lr1_grammar.hpp before.
// This setup allows us to use arbitrary grammars for ExprPtr(parse without getting
// multiple definitions of Symbol, Concrete, etc.
#include "test/lr1_grammar.hpp"
#include "parse.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

stringstream errBuffer;

void testRuleEquality() {
  const DFARule rule0 = { Concrete::ETIMES,
                          { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                          0,
                          BitSetToks("001") /* {PLUS} */ };

  const DFARule rule1 = { Concrete::ETIMES,
                          { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                          0,
                          BitSetToks("011") /* {STAR, PLUS} */ };

  RuleSet ruleSet0 = { rule0 };
  RuleSet ruleSet1 = { rule1 };

  // See notes in rules.hpp for how and why this is a thing
  TESTER.assertTrue(ruleSet0.contains(rule1));
  TESTER.assertTrue(ruleSet1.contains(rule0));
  TESTER.assertNotEqual(ruleSet0, ruleSet1);
}

void testAddRhses() {
  queue<DFARule> ruleQueue;
  const DFARule fromRule = { Concrete::ETIMES,
                             { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                             0,
                             BitSetToks("001") /* {PLUS} */ };
  addRhses(ruleQueue, fromRule);

  const DFARule expected0 = { Concrete::EINT,
                              { Symbol::INT },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected1 = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected2 = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };

  TESTER.assertEquals(3, ruleQueue.size());
  TESTER.assertEquals(expected0, ruleQueue.front());
  ruleQueue.pop();
  TESTER.assertEquals(expected1, ruleQueue.front());
  ruleQueue.pop();
  TESTER.assertEquals(expected2, ruleQueue.front());
  ruleQueue.pop();
}

void testEpsilonTransition() {
  const DFARule initRule = { Concrete::SCONC, { Symbol::EXPR }, 0, BitSetToks() /* {} */ };
  RuleSet ruleSet = { initRule };
  epsilonTransition(ruleSet);

  const DFARule expected0 = { Concrete::EINT,
                              { Symbol::INT },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected1 = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected2 = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };

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

  const DFARule initRule = { Concrete::SCONC, { Symbol::EXPR }, 0, BitSetToks() /* {} */ };
  const DFARule expected0 = { Concrete::EINT,
                              { Symbol::INT },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected1 = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expected2 = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };

  TESTER.assertEquals(4, ruleSet.size());
  TESTER.assertEquals(initRule, *ruleSet.find(initRule));
  TESTER.assertEquals(expected0, *ruleSet.find(expected0));
  TESTER.assertEquals(expected1, *ruleSet.find(expected1));
  TESTER.assertEquals(expected2, *ruleSet.find(expected2));
}

void testCreateTransitions() {
  const DFARule rule0 = { Concrete::SCONC, { Symbol::EXPR }, 1, BitSetToks() /* {} */ };
  const DFARule rule1 = { Concrete::EPLUS,
                          { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                          1,
                          BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule rule2 = { Concrete::ETIMES,
                          { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                          1,
                          BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ rule0, rule1, rule2 });
  createTransitions(dfa, dfa.getRoot());
  const auto& transitions = dfa.getRoot()->getTransitions();
  const RuleSet& ruleSetPlus = transitions.at(Symbol::PLUS)->getValue();
  const RuleSet& ruleSetTimes = transitions.at(Symbol::STAR)->getValue();


  const DFARule expectedPlus = { Concrete::EPLUS,
                                 { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                                 2,
                                 BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expectedTimes = { Concrete::ETIMES,
                                  { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                                  2,
                                  BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expectedBoth0 = { Concrete::EINT,
                                  { Symbol::INT },
                                  0,
                                  BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expectedBoth1 = { Concrete::EPLUS,
                                  { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                                  0,
                                  BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule expectedBoth2 = { Concrete::ETIMES,
                                  { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                                  0,
                                  BitSetToks("011") /* {STAR, PLUS} */ };

  TESTER.assertEquals(2, transitions.size());

  TESTER.assertEquals(4, ruleSetPlus.size());
  TESTER.assertEquals(expectedPlus, *ruleSetPlus.find(expectedPlus));
  TESTER.assertEquals(expectedBoth0, *ruleSetPlus.find(expectedBoth0));
  TESTER.assertEquals(expectedBoth1, *ruleSetPlus.find(expectedBoth1));
  TESTER.assertEquals(expectedBoth2, *ruleSetPlus.find(expectedBoth2));

  TESTER.assertEquals(4, ruleSetTimes.size());
  TESTER.assertEquals(expectedTimes, *ruleSetTimes.find(expectedTimes));
  TESTER.assertEquals(expectedBoth0, *ruleSetTimes.find(expectedBoth0));
  TESTER.assertEquals(expectedBoth1, *ruleSetTimes.find(expectedBoth1));
  TESTER.assertEquals(expectedBoth2, *ruleSetTimes.find(expectedBoth2));
}

void testTryReduce_noMatchStack() {
  const DFARule rule0 = { Concrete::EPLUS,
                          { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                          3,
                          BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ rule0 });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::NONE, tryReduce(dfa.getRoot(), Symbol::PLUS, stk, &reduceStart));
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_notAtEnd() {
  const DFARule rule0 = { Concrete::EPLUS,
                          { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                          2,
                          BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ rule0 });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::NONE, tryReduce(dfa.getRoot(), Symbol::PLUS, stk, &reduceStart));
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}

void testTryReduce_notInLookahead() {
  const DFARule rule0 = { Concrete::ETIMES,
                          { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                          3,
                          BitSetToks("010") /* {STAR} */ };

  DFA_t dfa({ rule0 });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::NONE, tryReduce(dfa.getRoot(), Symbol::PLUS, stk, &reduceStart));
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_lowerRulePrecedence_hasShiftable() {
  const DFARule reducible = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              3,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule shiftable = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                              1,
                              BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ reducible, shiftable });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::NONE, tryReduce(dfa.getRoot(), Symbol::STAR, stk, &reduceStart));
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_lowerRulePrecedence_noShiftable() {
  const DFARule rule0 = { Concrete::EPLUS,
                          { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                          3,
                          BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ rule0 });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::EPLUS, tryReduce(dfa.getRoot(), Symbol::STAR, stk, &reduceStart));
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_higherRulePrecedence() {
  const DFARule shiftable = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              1,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule reducible = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::STAR, Symbol::EXPR },
                              3,
                              BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ shiftable, reducible });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::ETIMES, tryReduce(dfa.getRoot(), Symbol::PLUS, stk, &reduceStart));
  TESTER.assertEquals(0, reduceStart);
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_unspecifiedPrecedence() {
  const DFARule shiftable = { Concrete::EINT,
                              { Symbol::INT },
                              0,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule reducible = { Concrete::EINT,
                              { Symbol::INT },
                              1,
                              BitSetToks("111") /* {INT, STAR, PLUS} */ };

  DFA_t dfa({ shiftable, reducible });
  const std::vector<StackObj>& stk = {
    StackObj{ new int(1), Symbol::INT, Concrete::NONE },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::NONE, tryReduce(dfa.getRoot(), Symbol::INT, stk, &reduceStart));
  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}


void testTryReduce_equalPrecedence_leftAssoc() {
  const DFARule shiftable = { Concrete::EPLUS,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              1,
                              BitSetToks("011") /* {STAR, PLUS} */ };
  const DFARule reducible = { Concrete::ETIMES,
                              { Symbol::EXPR, Symbol::PLUS, Symbol::EXPR },
                              3,
                              BitSetToks("011") /* {STAR, PLUS} */ };

  DFA_t dfa({ shiftable, reducible });
  const std::vector<StackObj>& stk = {
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
    StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
    StackObj{ new Expr*(new EInt(1)), Symbol::EXPR, Concrete::EINT },
  };
  size_t reduceStart;

  TESTER.assertEquals(Concrete::ETIMES, tryReduce(dfa.getRoot(), Symbol::PLUS, stk, &reduceStart));
  TESTER.assertEquals(0, reduceStart);
  for_each(stk.cbegin(), stk.cend(), mem_fun_ref(&StackObj::deleteObj));
}

void testShiftReduce() {
  using ExprPtr = unique_ptr<Expr>;

  DFA_t dfa = buildParserDFA();
  ExprPtr expr0 = ExprPtr(parse(dfa, { StackObj{ new int(1), Symbol::INT, Concrete::NONE } }));
  // 1 + 2
  ExprPtr expr1 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE } }));
  // 1 + 2 + 50
  ExprPtr expr2 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));
  // 1 + 2 * 50
  ExprPtr expr3 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));
  // 1 * 2 + 50
  ExprPtr expr4 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));
  // 1 * 2 * 50
  ExprPtr expr5 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::STAR, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));

  // 1 +
  ExprPtr noParse0 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE } }));
  // + 2
  ExprPtr noParse1 = ExprPtr(parse(
      dfa,
      { StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE } }));
  // 1 2 + 50
  ExprPtr noParse2 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ new int(2), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));
  // 1 + + 50
  ExprPtr noParse3 = ExprPtr(parse(
      dfa,
      { StackObj{ new int(1), Symbol::INT, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ nullptr, Symbol::PLUS, Concrete::NONE },
        StackObj{ new int(50), Symbol::INT, Concrete::NONE } }));

  TESTER.assertEquals(1, expr0->eval());
  TESTER.assertEquals(3, expr1->eval());
  TESTER.assertEquals(53, expr2->eval());
  TESTER.assertEquals(101, expr3->eval());
  TESTER.assertEquals(52, expr4->eval());
  TESTER.assertEquals(100, expr5->eval());

  TESTER.assertEquals(nullptr, noParse0);
  TESTER.assertEquals(nullptr, noParse1);
  TESTER.assertEquals(nullptr, noParse2);
  TESTER.assertEquals(nullptr, noParse3);
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  testRuleEquality();
  testAddRhses();
  testEpsilonTransition();
  testInitDFA();
  testCreateTransitions();
  testTryReduce_notAtEnd();
  testTryReduce_notInLookahead();
  testTryReduce_noMatchStack();
  testTryReduce_lowerRulePrecedence_hasShiftable();
  testTryReduce_lowerRulePrecedence_noShiftable();
  testTryReduce_higherRulePrecedence();
  testTryReduce_unspecifiedPrecedence();
  testTryReduce_equalPrecedence_leftAssoc();
  testShiftReduce();

  return 0;
}
