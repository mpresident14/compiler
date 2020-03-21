#include "regex_grammar.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
#include "parse.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;


UnitTest TESTER = UnitTest::createTester();
DFA_t dfa = buildParserDFA();

using UniqPtr = unique_ptr<Regex>;

void testGetDeriv_character() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("a")));

  TESTER.assertEquals(Epsilon(), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_alt() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("ac|ad")));
  UniqPtr r2 = UniqPtr(parse(dfa, lex("ac|bd")));

  UniqPtr e1 = UniqPtr(parse(dfa, lex("c|d")));
  UniqPtr e2 = UniqPtr(parse(dfa, lex("d")));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*e2, *r2->getDeriv('b'));
}

void testGetDeriv_concat() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("ac")));
  UniqPtr r2 = UniqPtr(parse(dfa, lex("a*c")));

  UniqPtr e1 = UniqPtr(parse(dfa, lex("c")));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*r2, *r2->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('c'));
}

void testGetDeriv_star() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("a*")));

  TESTER.assertEquals(*r1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_not() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("^a")));

  TESTER.assertEquals(Not(new Epsilon()), *r1->getDeriv('a'));
  TESTER.assertEquals(Not(new EmptySet()), *r1->getDeriv('b'));
}

void testGetDeriv_range() {
  UniqPtr r1 = UniqPtr(parse(dfa, lex("[0-9]")));

  TESTER.assertEquals(EmptySet(), *r1->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r1->getDeriv('7'));
}

int main(int, char**) {
  testGetDeriv_character();
  testGetDeriv_alt();
  testGetDeriv_concat();
  testGetDeriv_star();
  testGetDeriv_not();
  testGetDeriv_range();

  return 0;
}
