#include "test/utils/expr_grammar.hpp"
#include "generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace test::expr_grammar;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();
stringstream errBuffer;

void test_parserWithConflicts() {
  generateCode("test/generated/bad_expr_parser", "#include \"test/utils/expr.hpp\"\n", "", BAD_GRAMMAR_DATA);

  TESTER.assertTrue(errBuffer.str().starts_with("WARNING"));
}

int main() {
  // To test stderr output
  cerr.rdbuf(errBuffer.rdbuf());

  generateCode("test/generated/expr_parser", "#include \"test/utils/expr.hpp\"\n", "", GRAMMAR_DATA);
  test_parserWithConflicts();

  return 0;
}
