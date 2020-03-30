#include "regex_eval.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

/* See test/tokenize.cpp */
void generateRgxDFA() {
  vector<std::pair<std::string, int>> patterns = {
    { "a", -1 /* CHAR */ },
    { "for", -2 /* FOR */ },
    { "while", -3 /* WHILE */ },
    { "[1-9][0-9]*", -4 /* INT */ },
    { "([a-z]|[A-Z])([a-z]|[A-Z])*", -5 /* IDENTIFIER */ }
  };

  std::ofstream outFile;
  outFile.open("test/gen_lexer.hpp");
  outFile << "#include <unordered_map>\n"
          << "#include <memory>\n";
  rgxDFAToCode(outFile, patterns);
  outFile.close();
}

int main() {
  generateRgxDFA();

  return 0;
}
