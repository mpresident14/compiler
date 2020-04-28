#include "generate.hpp"
#include "utils.hpp"
#include "config_grammar.hpp"

#include <iostream>
#include <fstream>
#include <cstddef>
#include <string>

using namespace std;

int main() {


  generateLexerCode("src/config_lexer", ADDL_CODE, CONFIG_GRAMMAR);

  return 0;
}
