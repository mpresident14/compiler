#include "src/misc/logger.hpp"
#include "src/parser/config_parse.hpp"
#include "src/parser/generate.hpp"
#include "src/parser/testing/expr_grammar.hpp"
#include "src/parser/utils.hpp"

#include <iostream>
#include <sstream>

#include <prez/unit_test.hpp>

using namespace std;
using namespace test::expr_grammar;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
ostringstream errBuffer;

char *parserFilePath;
char *goodParserName;
char *badParserName;
const char *exprHppInclude = "#include \"src/parser/testing/expr.hpp\"\n";

void test_parserWithConflicts() {
  generateParserCode(
      ParseInfo{BAD_GRAMMAR_DATA, exprHppInclude, ""},
      ParseFlags{string(parserFilePath).append(badParserName), ""}, cerr);

  TESTER.assertTrue(errBuffer.str().starts_with(Logger::warningColored));
}

int main(int, char **argv) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  parserFilePath = argv[1];
  goodParserName = argv[2];
  badParserName = argv[2];

  generateParserCode(
      ParseInfo{GRAMMAR_DATA, exprHppInclude, ""},
      ParseFlags{string(parserFilePath).append(goodParserName), ""}, cerr);
  test_parserWithConflicts();

  return 0;
}
