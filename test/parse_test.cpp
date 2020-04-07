#include "test/expr_parser.hpp"
#include "test/bad_expr_parser.hpp"
#include "expr.hpp"

#include <memory>

#include <prez/unit_test.hpp>
#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
stringstream errBuffer;

using ExprPtr = unique_ptr<Expr>;

/* See test/write_lexer.cpp */
void testParse() {
  ExprPtr e0(test::expr_parser::parse("3+12 + 4"));
  ExprPtr e1(test::expr_parser::parse("3+12 *4"));
  ExprPtr e2(test::expr_parser::parse("3 *12 + 4"));
  ExprPtr e3(test::expr_parser::parse("3* 12*  4"));

  TESTER.assertEquals(19, e0->eval());
  TESTER.assertEquals(51, e1->eval());
  TESTER.assertEquals(40, e2->eval());
  TESTER.assertEquals(144, e3->eval());
}


void testParse_invalidTokens() {
  ostringstream expectedErr0;
  expectedErr0 << "Lexer error at: a * 24\n"
               << "Previous tokens were: " << vector<string>{ "INT", "PLUS" };

  string err0 =
      TESTER.assertThrows([]() { test::expr_parser::parse("1 + a * 24"); });
  TESTER.assertEquals(expectedErr0.str(), err0);
}


void testParse_noParse() {
  ostringstream expectedErr0;
  expectedErr0 << "No parse:\n\tStack: "
               << vector<string>{ "Expr", "PLUS", "Expr", "STAR", "PLUS" }
               << "\n\tRemaining tokens: " << vector<string>{ "INT" };

  string err0 =
      TESTER.assertThrows([]() { test::expr_parser::parse("123 + 24* + 5"); });
  TESTER.assertEquals(expectedErr0.str(), err0);

  // Note that INT is not in lookahead set after Expr STAR INT, so INT doesn't
  // reduce to Expr
  ostringstream expectedErr1;
  expectedErr1 << "No parse:\n\tStack: "
               << vector<string>{ "Expr", "STAR", "INT", "INT" }
               << "\n\tRemaining tokens: "
               << vector<string>{ "STAR", "PLUS", "INT" };

  string err1 =
      TESTER.assertThrows([]() { test::expr_parser::parse("3 * 2 34* + 5"); });
  TESTER.assertEquals(expectedErr1.str(), err1);
}


void testParse_withConflict() {
  ExprPtr e(test::bad_expr_parser::parse("3+12 * 4"));

  ostringstream expectedWarning;
  expectedWarning << "WARNING: Shift reduce conflict for rule\n\t2 -> Expr "
                     "PLUS Expr .\n\tNext token: STAR\n";

  TESTER.assertEquals(expectedWarning.str(), errBuffer.str());
}


int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  testParse();
  testParse_invalidTokens();
  testParse_noParse();
  testParse_withConflict();

  return 0;
}