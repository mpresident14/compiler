#include "generate.hpp"

#include <iostream>
#include <fstream>
#include <cstddef>
#include <string>

using namespace std;

/* See test/tokenize.cpp */
void generateConfigLexer() {
  GrammarData grammarData = {
    {
        { "HEADER", "", NONE, Assoc::NONE, "", "", "%header" },
        { "SOURCE", "", NONE, Assoc::NONE, "", "", "%source" },
        { "TOKENS", "", NONE, Assoc::NONE, "", "", "%tokens" },
        { "PREC", "", NONE, Assoc::NONE, "", "", "%prec" },
        { "GRAMMAR", "", NONE, Assoc::NONE, "", "", "%grammar" },
        { "DEFINED", "", NONE, Assoc::NONE, "", "", ":=" },
        { "BAR", "", NONE, Assoc::NONE, "", "", "\\|" },
        { "ARROW", "", NONE, Assoc::NONE, "", "", "\\->" },
        { "IDENT", "string", NONE, Assoc::NONE, "str", "", "([a-z]|[A-z])([a-z]|[A-z]|[0-9])*" },
        { "STRLIT", "string", NONE, Assoc::NONE, "str.substr(1, str.size() - 1)", "", "\"(\\\\|\\\"|\\n|\\t|^\\\")*\"" },
        // TODO: The regex "NOT" definition is not correct
        { "CODE", "string", NONE, Assoc::NONE, "str.substr(2, str.size() - 2)", "", "%{^(%{|%})*%}" },
    },
    {},
    {}
  };

  std::ofstream outFile;
  outFile.open("src/config_lexer.cpp");
  outFile << R"(#include "config_lexer.hpp")" << '\n';
  lexerStuff(outFile, grammarData);
  outFile.close();
}

int main() {
  generateConfigLexer();

  return 0;
}
