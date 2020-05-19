#include "src/parser/generate.hpp"
#include "test/parser/utils/expr_grammar.hpp"
#include "src/parser/config_parse.hpp"

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
      ParseInfo{
          BAD_GRAMMAR_DATA,
          "#include \"test/parser/utils/expr.hpp\"\n",
          ""});

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  generateParserCode(
      "test/parser/generated/expr_parser",
      ParseInfo{
          GRAMMAR_DATA,
          "#include \"test/parser/utils/expr.hpp\"\n",
          ""});
  test_parserWithConflicts();

  return 0;
}
