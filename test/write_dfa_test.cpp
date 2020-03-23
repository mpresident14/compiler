#include "regex_match.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

// WARNING: Depends on writeDFA() in regex_test.cpp
#include "gen_regex_dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

void testGeneratedRgxDFA() {
  vector<StackObj> actual = tokenize("123fora", getRoot());

  TESTER.assertEquals(3, actual.size());

  TESTER.assertEquals(Symbol::DASH, actual[0].symbol);
  TESTER.assertEquals(Symbol::BAR, actual[1].symbol);
  TESTER.assertEquals(Symbol::CHAR, actual[2].symbol);

  TESTER.assertEquals(123, *(int*)actual[0].obj);
  TESTER.assertEquals(nullptr, actual[1].obj);
  TESTER.assertEquals('a', *(char*)actual[2].obj);
}

int main(int, char**) {
  testGeneratedRgxDFA();

  return 0;
}
