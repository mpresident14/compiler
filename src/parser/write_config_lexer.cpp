#include "src/parser/config_grammar.hpp"
#include "src/parser/generate.hpp"
#include "src/parser/utils.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main() {
  generateLexerCode("src/parser/config_lexer", ADDL_CODE, CONFIG_GRAMMAR);

  return 0;
}
