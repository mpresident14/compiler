#include "gen_parser.hpp"
#include "expr.hpp"

#include <memory>

#include <prez/unit_test.hpp>
#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

using ExprPtr = unique_ptr<Expr>;

/* See test/write_lexer.cpp */
void testParse() {
  ExprPtr e0(parse("3+12 + 4"));
  ExprPtr e1(parse("3+12 *4"));
  ExprPtr e2(parse("3 *12 + 4"));
  ExprPtr e3(parse("3* 12*  4"));

  TESTER.assertEquals(19, e0->eval());
  TESTER.assertEquals(51, e1->eval());
  TESTER.assertEquals(40, e2->eval());
  TESTER.assertEquals(144, e3->eval());
}


void testParse_invalidTokens() {
  ostringstream expectedErr0;
  expectedErr0 << "Lexer error at: a * 24\n"
      << "Previous tokens were: " << vector<string>{"INT", "PLUS"};

  string err0 = TESTER.assertThrows([](){ parse("1 + a * 24"); });
  TESTER.assertEquals(expectedErr0.str(), err0);
}


void testParse_noParse() {
  ostringstream expectedErr0;
  expectedErr0 << "No parse:\n\tStack: " << vector<string>{ "Expr", "PLUS", "Expr", "STAR", "PLUS"}
           << "\n\tRemaining tokens: " << vector<string>{"INT"};

  string err0 = TESTER.assertThrows([](){ parse("123 + 24* + 5"); });
  TESTER.assertEquals(expectedErr0.str(), err0);

  // Note that INT is not in lookahead set after Expr STAR INT, so INT doesn't reduce to Expr
  ostringstream expectedErr1;
  expectedErr1 << "No parse:\n\tStack: " << vector<string>{ "Expr", "STAR", "INT", "INT"}
           << "\n\tRemaining tokens: " << vector<string>{"STAR", "PLUS", "INT"};

  string err1 = TESTER.assertThrows([](){ parse("3 * 2 34* + 5"); });
  TESTER.assertEquals(expectedErr1.str(), err1);
}


int main() {
  testParse();
  testParse_invalidTokens();
  testParse_noParse();

  return 0;
}
