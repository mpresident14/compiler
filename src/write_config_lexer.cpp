#include "generate.hpp"
#include "utils.hpp"
#include "config_grammar.hpp"

#include <iostream>
#include <fstream>
#include <cstddef>
#include <string>

using namespace std;

/* See test/tokenize.cpp */
void generateConfigLexer() {
  std::ofstream outFile;
  outFile.open("src/config_lexer.cpp");
  outFile << R"(#include "config_lexer.hpp")" << '\n';
  configLexerStuff(outFile, CONFIG_GRAMMAR);
  outFile.close();
}

int main() {
  generateConfigLexer();

  return 0;
}
