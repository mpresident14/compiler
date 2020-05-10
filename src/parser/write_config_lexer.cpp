#include "src/parser/generate.hpp"
#include "src/parser/utils.hpp"
#include "src/parser/config_grammar.hpp"

#include <iostream>
#include <fstream>
#include <cstddef>
#include <string>

using namespace std;

int main() {
  generateLexerCode("src/parser/config_lexer", ADDL_CODE, CONFIG_GRAMMAR);

  return 0;
}
