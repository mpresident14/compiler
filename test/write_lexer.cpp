#include "regex_eval.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

/* See test/tokenize.cpp */
void generateRgxDFA() {
  GrammarData grammarData = {
    {
      { "", "", NONE, Assoc::NONE, "", "", "a" /* CHAR */ },
      { "", "", NONE, Assoc::NONE, "", "", "for" /* FOR */ },
      { "", "", NONE, Assoc::NONE, "", "", "while" /* WHILE */ },
      { "", "", NONE, Assoc::NONE, "", "", "[1-9][0-9]*" /* INT */ },
      {"", "", NONE, Assoc::NONE, "", "", "([a-z]|[A-Z])([a-z]|[A-Z])*", /* IDENTIFIER */ },
    },
    {},
    {}
  };

  std::ofstream outFile;
  outFile.open("test/gen_lexer.hpp");
  outFile << "#include <unordered_map>\n"
          << "#include <memory>\n";
  rgxDFAToCode(outFile, grammarData);
  outFile.close();
}

int main() {
  generateRgxDFA();

  return 0;
}
