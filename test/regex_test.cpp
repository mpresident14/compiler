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
#include <prez/print_stuff.hpp>

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
  ostringstream expectedErr0;
  expectedErr0 << "No parse:\n\tStack: "
               << vector<string>{ "Regex", "LPAREN", "Regex" }
               << "\n\tRemaining tokens: " << vector<string>{};

  string err0 = TESTER.assertThrows([]() { parse("abc(b*"); });
  TESTER.assertEquals(expectedErr0.str(), err0);

  ostringstream expectedErr1;
  expectedErr1 << "No parse:\n\tStack: "
               << vector<string>{ "Regex", "CARET", "STAR" }
               << "\n\tRemaining tokens: " << vector<string>{ "CHAR" };

  string err1 = TESTER.assertThrows([]() { parse("abc^*d"); });
  TESTER.assertEquals(expectedErr1.str(), err1);

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
  GrammarData grammarData = {
    {
        { "", "", NONE, Assoc::NONE, "", "", "a*" },
        { "", "", NONE, Assoc::NONE, "", "", "[1-9][0-9]*" },
        { "", "", NONE, Assoc::NONE, "", "", "for" },
    },
    {},
    {}
  };

  vector<pair<string, int>> patterns = { { "a*", 1 },
                                         { "[1-9][0-9]*", 2 },
                                         { "for", 3 } };

  stringstream out;
  out.setstate(ios_base::badbit);
  rgxDFAToCode(out, grammarData);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}


void testRgxDFAToCode_withInvalidRegex() {
  GrammarData grammarData = {
    {
        { "", "", NONE, Assoc::NONE, "", "", "." },
        { "", "", NONE, Assoc::NONE, "", "", "1-9][0-9]*" },
        { "", "", NONE, Assoc::NONE, "", "", "for" },
    },
    {},
    {}
  };

  stringstream out;
  out.setstate(ios_base::badbit);
  TESTER.assertThrows(([&]() { rgxDFAToCode(out, grammarData); }));
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
