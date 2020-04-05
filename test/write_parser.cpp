#include "expr_grammar.hpp"
#include "generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

int main() {
  generateCode("test/gen_parser", "test/expr.hpp", "", GRAMMAR_DATA);

  return 0;
}
