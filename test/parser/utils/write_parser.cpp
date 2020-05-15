#include "src/parser/generate.hpp"
#include "test/parser/utils/expr_grammar.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace test::expr_grammar;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
stringstream errBuffer;

void test_parserWithConflicts() {
  generateParserCode(
      "test/parser/generated/bad_expr_parser",
      "#include \"test/parser/utils/expr.hpp\"\n",
      "",
      BAD_GRAMMAR_DATA);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  generateParserCode(
      "test/parser/generated/expr_parser",
      "#include \"test/parser/utils/expr.hpp\"\n",
      "",
      GRAMMAR_DATA);
  test_parserWithConflicts();

  return 0;
}
