#include "dfa.hpp"
#include "utils.hpp"
#include "rules.hpp"
#include "build_parser.hpp"
#include "test/lr1_grammar.hpp"

#include <prez/unit_test.hpp>
#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;


UnitTest TESTER = UnitTest::createTester();

void testNumStates(const DFA<DFARuleSet, int>& dfa) {
  TESTER.assertEquals(7, dfa.size());
}

void testInitState(const DFA<DFARuleSet, int>& dfa) {
  /* lookahead = {STAR, PLUS, INT} */

  const DFARuleSet initRules = dfa.getRoot()->getValue();
  const DFARule expected0 = { SCONC, { EXPR }, 0, { false, false, false } };
  const DFARule expected1 = { EINT, { INT }, 0, { true, true, false } };
  const DFARule expected2 = {
    EPLUS, { EXPR, PLUS, EXPR }, 0, { true, true, false }
  };
  const DFARule expected3 = {
    ETIMES, { EXPR, STAR, EXPR }, 0, { true, true, false }
  };

  TESTER.assertEquals(4, initRules.size());
  TESTER.assertEquals(expected0, *initRules.find(expected0));
  TESTER.assertEquals(expected1, *initRules.find(expected1));
  TESTER.assertEquals(expected2, *initRules.find(expected2));
  TESTER.assertEquals(expected3, *initRules.find(expected3));
}

void testTransition(const DFA<DFARuleSet, int>& dfa) {
  const auto& transitions = dfa.getRoot()->getTransitions();
  const DFARuleSet& ruleSetInt = transitions.at(INT)->getValue();
  const DFARuleSet& ruleSetExpr = transitions.at(EXPR)->getValue();
  const DFARule expectedInt = { EINT, { INT }, 1, { true, true, false } };
  const DFARule expectedExpr0 = {
    EPLUS, { EXPR, PLUS, EXPR }, 1, { true, true, false }
  };
  const DFARule expectedExpr1 = {
    ETIMES, { EXPR, STAR, EXPR }, 1, { true, true, false }
  };
  const DFARule expectedExpr2 = { SCONC, { EXPR }, 1, { false, false, false } };

  TESTER.assertEquals(2, transitions.size());

  TESTER.assertEquals(1, ruleSetInt.size());
  TESTER.assertEquals(expectedInt, *ruleSetInt.find(expectedInt));

  TESTER.assertEquals(3, ruleSetExpr.size());
  TESTER.assertEquals(expectedExpr0, *ruleSetExpr.find(expectedExpr0));
  TESTER.assertEquals(expectedExpr1, *ruleSetExpr.find(expectedExpr1));
  TESTER.assertEquals(expectedExpr2, *ruleSetExpr.find(expectedExpr2));
}


int main() {
  auto dfa = buildParserDFA(GRAMMAR, 5);

  testNumStates(dfa);
  testInitState(dfa);
  testTransition(dfa);

  return 0;
}
