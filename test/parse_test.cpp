#include "gen_parser.hpp"
#include "expr.hpp"

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

/* See test/write_lexer.cpp */
void testParse() {
  Expr* e1 = parse("3+2");

  TESTER.assertEquals(5, e1->eval());

  delete e1;
}


// void testTokenize_withInvalidInput() {
//   TESTER.assertThrows([]() { tokenize("123fo$a"); });
// }

int main() {
  testParse();

  return 0;
}
