#include "expr_grammar.hpp"
#include "generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

int main() {
  generateCode("test/expr_parser", "#include \"test/expr.hpp\"\n", "", GRAMMAR_DATA);
  generateCode("test/bad_expr_parser", "#include \"test/expr.hpp\"\n", "", BAD_GRAMMAR_DATA);

  return 0;
}
