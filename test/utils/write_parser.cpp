#include "test/utils/expr_grammar.hpp"
#include "generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;
using namespace test::expr_grammar;

int main() {
  generateCode("test/generated/expr_parser", "#include \"test/utils/expr.hpp\"\n", "", GRAMMAR_DATA);
  generateCode("test/generated/bad_expr_parser", "#include \"test/utils/expr.hpp\"\n", "", BAD_GRAMMAR_DATA);

  return 0;
}
