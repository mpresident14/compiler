#include "src/misc/logger.hpp"
#include "src/parser/config_parse.hpp"
#include "src/parser/generate.hpp"
#include "src/parser/utils.hpp"
#include "test/parser/utils/expr_grammar.hpp"

#include <iostream>
#include <sstream>

#include <prez/unit_test.hpp>

using namespace std;
using namespace test::expr_grammar;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
ostringstream errBuffer;

void test_parserWithConflicts() {
  generateParserCode(
      ParseInfo{ BAD_GRAMMAR_DATA, "#include \"test/parser/utils/expr.hpp\"\n", "" },
      ParseFlags{ "test/parser/generated/bad_expr_parser", "" },
      cerr);

  TESTER.assertTrue(errBuffer.str().starts_with(Logger::warningColored));
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  generateParserCode(
      ParseInfo{ GRAMMAR_DATA, "#include \"test/parser/utils/expr.hpp\"\n", "" },
      ParseFlags{ "test/parser/generated/expr_parser", "" },
      cerr);
  test_parserWithConflicts();

  return 0;
}
