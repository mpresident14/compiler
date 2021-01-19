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

void test_parserWithConflicts() {
  generateParserCode(ParseInfo{BAD_GRAMMAR_DATA, exprHppInclude, ""},
                     ParseFlags{outDir, includeDir, conflictedParserName, ""},
                     cerr);

  TESTER.assertTrue(errBuffer.str().starts_with(Logger::warningColored));
}

char *outDir;
char *includeDir;
char *okParserName;
char *conflictedParserName;
const char *exprHppInclude = "#include \"src/parser/testing/expr.hpp\"\n";

int main(int, char **argv) {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  outDir = argv[1];
  includeDir = argv[2];
  okParserName = argv[3];
  conflictedParserName = argv[4];

  generateParserCode(ParseInfo{GRAMMAR_DATA, exprHppInclude, ""},
                     ParseFlags{outDir, includeDir, okParserName, ""}, cerr);
  test_parserWithConflicts();

  return 0;
}
