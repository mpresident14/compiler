#include "src/parser/config_grammar.hpp"
#include "src/parser/generate.hpp"
#include "src/parser/utils.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int, char **argv) {
  generateLexerCode(argv[1], ADDL_CODE, CONFIG_GRAMMAR);
  return 0;
}
