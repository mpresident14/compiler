#include "regex_match.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_set>
#include <sstream>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;


UnitTest TESTER = UnitTest::createTester();

DFA_t dfa = buildParserDFA();
stringstream errBuffer;
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

void testHashFn() {
  RgxPtr r1 = RgxPtr(parse(dfa, lex("a")));
  unordered_set<RgxPtr> rgxs = { r1->getDeriv('b') };

  TESTER.assertTrue(rgxs.contains(r1->getDeriv('c')));
}

void testMatches() {
  TESTER.assertTrue(matches("a", "a"));
  TESTER.assertFalse(matches("a", "b"));

  TESTER.assertTrue(matches("ab*", "abbbb"));
  TESTER.assertFalse(matches("ab*", "ac"));

  TESTER.assertTrue(matches("[1-9][0-9]*", "1234"));
  TESTER.assertFalse(matches("[1-9][0-9]*", "01234"));

  TESTER.assertTrue(matches("^a*c", "ab"));
  TESTER.assertFalse(matches("^a*c", "c"));

  TESTER.assertTrue(matches("a*|[0-9]", ""));
  TESTER.assertTrue(matches("a*|[0-9]", "5"));
  TESTER.assertFalse(matches("a*|[0-9]", "c"));

  TESTER.assertTrue(matches("(a|b)[0-9]", "a5"));
  TESTER.assertFalse(matches("(a|b)[0-9]", "a"));

  TESTER.assertTrue(matches("a.c", "abc"));
  TESTER.assertTrue(matches("a.c", "arc"));
}


void testBuildMergedRgxDFA_withNullableRegex() {
  vector<TokenPattern> patterns = { { "a*", 1 },
                                    { "[1-9][0-9]*", 2 },
                                    { "for", 3 } };
  buildMergedRgxDFA(patterns);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}


void testBuildMergedRgxDFA_withInvalidRegex() {
  vector<TokenPattern> patterns = { { ".", 1 },
                                    { "1-9][0-9]*", 2 },
                                    { "for", 3 } };

  TESTER.assertThrows([&patterns]() { buildMergedRgxDFA(patterns); });
}


int main(int, char**) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  testGetDeriv_character();
  testGetDeriv_alt();
  testGetDeriv_concat();
  testGetDeriv_star();
  testGetDeriv_not();
  testGetDeriv_range();
  testHashFn();
  testMatches();
  testBuildMergedRgxDFA_withNullableRegex();
  testBuildMergedRgxDFA_withInvalidRegex();

  return 0;
}
