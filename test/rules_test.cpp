#include "rules.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <algorithm>

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

int main(int, char**)
{
  testAddRhses();

  return 0;
}
