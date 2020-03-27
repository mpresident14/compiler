#include "regex_eval.hpp"
#include "regex.hpp"
// #include "regex_lexer.hpp"
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

stringstream errBuffer;


void testParse() {
  RgxPtr r1 = parse("a");
  RgxPtr r2 = parse("^a|b");
  // RgxPtr r3 = parse("^a");
  // RgxPtr r3 = parse("^(a|b)");
  RgxPtr r4 = parse("abcd");
  RgxPtr r5 = parse("a|b|cd");
  RgxPtr r6 = parse("[1-9]");
  RgxPtr r7 = parse("[1-9]*");

  // RgxPtr t0 = parse("(a|b)");
  // RgxPtr t1 = parse("cd(a|b)");
  // RgxPtr t2 = parse("c*(a|b)");
  // RgxPtr t3 = parse("^k(a|b)");
  // RgxPtr t4 = parse("[1-3](a|b)");
  // RgxPtr t5 = parse("c|(a|b)");
  RgxPtr t0 = parse("(ab)");
  RgxPtr t1 = parse("c(ab)");
  RgxPtr t2 = parse("c(^b)");
  RgxPtr t3 = parse("^k(a*)");
  RgxPtr t4 = parse("k(a)");

  cout << r1 << endl;


  cout << (void*)t0.get() << endl;
  cout << (void*)t1.get() << endl;
  cout << (void*)t2.get() << endl;
  cout << (void*)t3.get() << endl;
  cout << (void*)t4.get() << endl;
  // cout << (void*)t4.get() << endl;
  // cout << (void*)t5.get() << endl;

  TESTER.assertEquals(RgxType::CHARACTER, r1->getType());
  TESTER.assertEquals(RgxType::ALT, r2->getType());
  // TESTER.assertEquals(RgxType::NOT, r3->getType());
  TESTER.assertEquals(RgxType::CONCAT, r4->getType());
  TESTER.assertEquals(RgxType::ALT, r5->getType());
  TESTER.assertEquals(RgxType::RANGE, r6->getType());
  TESTER.assertEquals(RgxType::STAR, r7->getType());
}

// void testGetDeriv_character() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("a")));

//   TESTER.assertEquals(Epsilon(), *r1->getDeriv('a'));
//   TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
// }

// void testGetDeriv_alt() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("ac|ad")));
//   UniqPtr r2 = UniqPtr(parse(dfa, lex("ac|bd")));

//   UniqPtr e1 = UniqPtr(parse(dfa, lex("c|d")));
//   UniqPtr e2 = UniqPtr(parse(dfa, lex("d")));

//   TESTER.assertEquals(*e1, *r1->getDeriv('a'));
//   TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
//   TESTER.assertEquals(*e2, *r2->getDeriv('b'));
// }

// void testGetDeriv_concat() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("ac")));
//   UniqPtr r2 = UniqPtr(parse(dfa, lex("a*c")));

//   UniqPtr e1 = UniqPtr(parse(dfa, lex("c")));

//   TESTER.assertEquals(*e1, *r1->getDeriv('a'));
//   TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
//   TESTER.assertEquals(*r2, *r2->getDeriv('a'));
//   TESTER.assertEquals(Epsilon(), *r2->getDeriv('c'));
// }

// void testGetDeriv_star() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("a*")));

//   TESTER.assertEquals(*r1, *r1->getDeriv('a'));
//   TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
// }

// void testGetDeriv_not() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("^a")));

//   TESTER.assertEquals(Not(new Epsilon()), *r1->getDeriv('a'));
//   TESTER.assertEquals(Not(new EmptySet()), *r1->getDeriv('b'));
// }

// void testGetDeriv_range() {
//   UniqPtr r1 = UniqPtr(parse(dfa, lex("[0-9]")));

//   TESTER.assertEquals(EmptySet(), *r1->getDeriv('a'));
//   TESTER.assertEquals(Epsilon(), *r1->getDeriv('7'));
// }

// void testHashFn() {
//   RgxPtr r1 = RgxPtr(parse(dfa, lex("a")));
//   unordered_set<RgxPtr> rgxs = { r1->getDeriv('b') };

//   TESTER.assertTrue(rgxs.contains(r1->getDeriv('c')));
// }

// void testMatches() {
//   TESTER.assertTrue(matches("a", "a"));
//   TESTER.assertFalse(matches("a", "b"));

//   TESTER.assertTrue(matches("ab*", "abbbb"));
//   TESTER.assertFalse(matches("ab*", "ac"));

//   TESTER.assertTrue(matches("[1-9][0-9]*", "1234"));
//   TESTER.assertFalse(matches("[1-9][0-9]*", "01234"));

//   TESTER.assertTrue(matches("^a*c", "ab"));
//   TESTER.assertFalse(matches("^a*c", "c"));

//   TESTER.assertTrue(matches("a*|[0-9]", ""));
//   TESTER.assertTrue(matches("a*|[0-9]", "5"));
//   TESTER.assertFalse(matches("a*|[0-9]", "c"));

//   TESTER.assertTrue(matches("(a|b)[0-9]", "a5"));
//   TESTER.assertFalse(matches("(a|b)[0-9]", "a"));

//   TESTER.assertTrue(matches("a.c", "abc"));
//   TESTER.assertTrue(matches("a.c", "arc"));
// }


// void testBuildMergedRgxDFA_withNullableRegex() {
//   vector<TokenPattern> patterns = { { "a*", 1 },
//                                     { "[1-9][0-9]*", 2 },
//                                     { "for", 3 } };
//   buildMergedRgxDFA(patterns);

//   TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
//   errBuffer.str("");
// }


// void testBuildMergedRgxDFA_withInvalidRegex() {
//   vector<TokenPattern> patterns = { { ".", 1 },
//                                     { "1-9][0-9]*", 2 },
//                                     { "for", 3 } };

//   TESTER.assertThrows([&patterns]() { buildMergedRgxDFA(patterns); });
// }


int main(int, char**) {
  // To test stderr output
  // cerr.rdbuf(errBuffer.rdbuf());

  testParse();

  // testGetDeriv_character();
  // testGetDeriv_alt();
  // testGetDeriv_concat();
  // testGetDeriv_star();
  // testGetDeriv_not();
  // testGetDeriv_range();
  // testHashFn();
  // testMatches();
  // testBuildMergedRgxDFA_withNullableRegex();
  // testBuildMergedRgxDFA_withInvalidRegex();

  return 0;
}
