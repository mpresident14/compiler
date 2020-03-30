#include "regex_parse.hpp"
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
  RgxPtr r0 = parse("a");
  RgxPtr r1 = parse("^a|b");
  RgxPtr r2 = parse("^(a|b)");
  RgxPtr r3 = parse("abcd*");
  RgxPtr r4 = parse("a|b|cd");
  RgxPtr r5 = parse("[1-9]");
  RgxPtr r6 = parse("[1-9]*");

  TESTER.assertEquals(RgxType::CHARACTER, r0->getType());
  TESTER.assertEquals(RgxType::ALT, r1->getType());
  TESTER.assertEquals(RgxType::NOT, r2->getType());
  TESTER.assertEquals(RgxType::CONCAT, r3->getType());
  TESTER.assertEquals(RgxType::ALT, r4->getType());
  TESTER.assertEquals(RgxType::RANGE, r5->getType());
  TESTER.assertEquals(RgxType::STAR, r6->getType());

  // No conflicts
  TESTER.assertEquals("", errBuffer.str());
}


void testParseError() {
  string err0 = TESTER.assertThrows([]() { parse("abc(b*"); });
  ostringstream expectedStk0;
  ostringstream expectedRemaining0;
  expectedStk0 << vector<int>{ 1 /* REGEX */, -6 /* LPAREN */, 1 /* REGEX */ };
  expectedRemaining0 << vector<int>();

  TESTER.assertTrue(err0.find(expectedStk0.str()) != string::npos);
  TESTER.assertTrue(err0.find(expectedRemaining0.str()) != string::npos);

  string err1 = TESTER.assertThrows([]() { parse("abc^*d"); });
  ostringstream expectedStk1;
  ostringstream expectedRemaining1;
  expectedStk1 << vector<int>{ 1 /* REGEX */, -3 /* CARET */, -2 /* STAR */ };
  expectedRemaining1 << vector<int>{ -9 /* CHAR */ };

  TESTER.assertTrue(err1.find(expectedStk1.str()) != string::npos);
  TESTER.assertTrue(err1.find(expectedRemaining1.str()) != string::npos);

  // No conflicts
  TESTER.assertEquals("", errBuffer.str());
}


void testGetDeriv_character() {
  RgxPtr r1 = RgxPtr(parse("a"));

  TESTER.assertEquals(Epsilon(), *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}


void testGetDeriv_alt() {
  RgxPtr r1 = RgxPtr(parse("ac|ad"));
  RgxPtr r2 = RgxPtr(parse("ac|bd"));

  RgxPtr e1 = RgxPtr(parse("c|d"));
  RgxPtr e2 = RgxPtr(parse("d"));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*e2, *r2->getDeriv('b'));
}


void testGetDeriv_concat() {
  RgxPtr r1 = RgxPtr(parse("ac"));
  RgxPtr r2 = RgxPtr(parse("a*c"));

  RgxPtr e1 = RgxPtr(parse("c"));

  TESTER.assertEquals(*e1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
  TESTER.assertEquals(*r2, *r2->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('c'));
}


void testGetDeriv_star() {
  RgxPtr r1 = RgxPtr(parse("a*"));

  TESTER.assertEquals(*r1, *r1->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r1->getDeriv('b'));
}


void testGetDeriv_not() {
  RgxPtr r1 = RgxPtr(parse("^a"));

  TESTER.assertEquals(Not(new Epsilon()), *r1->getDeriv('a'));
  TESTER.assertEquals(Not(new EmptySet()), *r1->getDeriv('b'));
}


void testGetDeriv_range() {
  RgxPtr r1 = RgxPtr(parse("[0-9]"));

  TESTER.assertEquals(EmptySet(), *r1->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r1->getDeriv('7'));
}


void testHashFn() {
  RgxPtr r1 = RgxPtr(parse("a"));
  unordered_set<RgxPtr> rgxs = { r1->getDeriv('b') };

  TESTER.assertTrue(rgxs.contains(r1->getDeriv('c')));
}



void testRgxDFAToCode_withNullableRegex() {
  vector<pair<string, int>> patterns = { { "a*", 1 },
                                    { "[1-9][0-9]*", 2 },
                                    { "for", 3 } };

  stringstream out;
  out.setstate(ios_base::badbit);
  rgxDFAToCode(out, patterns);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}


void testRgxDFAToCode_withInvalidRegex() {
  vector<pair<string, int>> patterns = { { ".", 1 },
                                    { "1-9][0-9]*", 2 },
                                    { "for", 3 } };

  stringstream out;
  out.setstate(ios_base::badbit);
  TESTER.assertThrows(([&](){ rgxDFAToCode(out, patterns); }));
}


int main(int, char**) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  testParse();
  testParseError();
  testGetDeriv_character();
  testGetDeriv_alt();
  testGetDeriv_concat();
  testGetDeriv_star();
  testGetDeriv_not();
  testGetDeriv_range();
  testHashFn();
  testRgxDFAToCode_withNullableRegex();
  testRgxDFAToCode_withInvalidRegex();

  return 0;
}
