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

void testGetDeriv_character() {
  auto r1 = parse(dfa, lex("a"));

  TESTER.assertEquals(Epsilon(), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_alt() {
  auto r1 = parse(dfa, lex("ac|ad"));
  auto r2 = parse(dfa, lex("ac|bd"));

  TESTER.assertEquals(*parse(dfa, lex("c|d")), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*parse(dfa, lex("d")), *r2->getDeriv('b'));
}

void testGetDeriv_concat() {
  auto r1 = parse(dfa, lex("ac"));
  auto r2 = parse(dfa, lex("a*c"));

  TESTER.assertEquals(*parse(dfa, lex("c")), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*r2, *r2->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('c'));
}

void testGetDeriv_star() {
  auto r1 = parse(dfa, lex("a*"));

  TESTER.assertEquals(*r1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}

void testGetDeriv_not() {
  auto r1 = parse(dfa, lex("^a"));

  TESTER.assertEquals(Not(new Epsilon()), *r1->getDeriv('a'));
  TESTER.assertEquals(Not(new EmptySet()), *r1->getDeriv('b'));
}

void testGetDeriv_range() {
  auto r1 = parse(dfa, lex("[0-9]"));

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
