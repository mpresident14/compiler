
// #include "src/parser/regex_parse.hpp"
#include "src/parser/regex_parser.hpp"
#include "src/parser/regex_eval.hpp"
#include "src/parser/regex.hpp"
#include "src/parser/dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_set>
#include <sstream>

#include <prez/unit_test.hpp>
#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;
using src::parser::regex_parser::parse;

UnitTest TESTER = UnitTest::createTester();

stringstream errBuffer;

void testParse() {
  RgxPtr r0 = RgxPtr(parse("a"));
  RgxPtr r1 = RgxPtr(parse("ab*|b"));
  RgxPtr r2 = RgxPtr(parse("[^a-z]"));
  RgxPtr r3 = RgxPtr(parse("abcd*"));
  RgxPtr r4 = RgxPtr(parse("a|b|cd"));
  RgxPtr r5 = RgxPtr(parse("[1-9]"));
  RgxPtr r6 = RgxPtr(parse("[1-9]*"));
  RgxPtr r7 = RgxPtr(parse("\\[^([^\\]]|\\\\.)*\\]"));
  RgxPtr r8 = RgxPtr(parse("(a|b)+"));
  RgxPtr r9 = RgxPtr(parse("(ab)?"));

  TESTER.assertEquals(RgxType::CHARACTER, r0->getType());
  TESTER.assertEquals(RgxType::ALT, r1->getType());
  TESTER.assertEquals(RgxType::NOT, r2->getType());
  TESTER.assertEquals(RgxType::CONCAT, r3->getType());
  TESTER.assertEquals(RgxType::ALT, r4->getType());
  TESTER.assertEquals(RgxType::RANGE, r5->getType());
  TESTER.assertEquals(RgxType::STAR, r6->getType());
  TESTER.assertEquals(RgxType::CONCAT, r7->getType());
  TESTER.assertEquals(RgxType::CONCAT, r8->getType());
  TESTER.assertEquals(RgxType::ALT, r9->getType());

  // No conflicts
  TESTER.assertEquals("", errBuffer.str());
}


void testParseError() {
  ostringstream expectedErr0;
  expectedErr0 << "Parse error on line 1:\n\tStack: "
               << vector<string>{ "Concats", "LPAREN", "Regex" }
               << "\n\tRemaining tokens: " << vector<string>{};

  string err0 = TESTER.assertThrows([]() { parse("abc(b*"); });
  TESTER.assertEquals(expectedErr0.str(), err0);

  ostringstream expectedErr1;
  expectedErr1 << "Parse error on line 1:\n\tStack: "
               << vector<string>{ "Regex", "BAR", "STAR" }
               << "\n\tRemaining tokens: " << vector<string>{ "CHAR" };

  string err1 = TESTER.assertThrows([]() { parse("abc|*d"); });
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


void testGetDeriv_brackets() {
  RgxPtr r1 = RgxPtr(parse("[0-9]"));
  RgxPtr r2 = RgxPtr(parse("[^0-9]"));
  RgxPtr r3 = RgxPtr(parse("[\\]*$-]"));
  RgxPtr r4 = RgxPtr(parse("[^\\]*$-]"));

  TESTER.assertEquals(EmptySet(), *r1->getDeriv('a'));
  TESTER.assertEquals(Epsilon(), *r1->getDeriv('7'));
  TESTER.assertEquals(Epsilon(), *r2->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r2->getDeriv('7'));
  TESTER.assertEquals(Epsilon(), *r3->getDeriv(']'));
  TESTER.assertEquals(EmptySet(), *r3->getDeriv('a'));
  TESTER.assertEquals(EmptySet(), *r4->getDeriv(']'));
  TESTER.assertEquals(Epsilon(), *r4->getDeriv('a'));
}


void testHashFn() {
  RgxPtr r1 = RgxPtr(parse("a"));
  cout << "HERE" << endl;
  unordered_set<RgxPtr, Regex::PtrHash> rgxs = { r1->getDeriv('b') };
  cout << "HERE2" << endl;
  TESTER.assertTrue(rgxs.contains(r1->getDeriv('c')));
  cout << "HERE3" << endl;
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
  cout << "HERE4" << endl;
  rgxDFAToCode(out, grammarData);
  cout << "HERE" << endl;

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
  errBuffer.str("");
}


void testRgxDFAToCode_withInvalidRegex() {
  GrammarData grammarData = {
    {
        { "", "", NONE, Assoc::NONE, "", "", "." },
        { "", "", NONE, Assoc::NONE, "", "", "9)[0-9]*" },
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
  testGetDeriv_brackets();
  testHashFn();
  testRgxDFAToCode_withNullableRegex();
  testRgxDFAToCode_withInvalidRegex();

  return 0;
}
