#include "src/parser/generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;



int main() {
  GrammarData grammarData = {
      {
          { "DNOTABC", "char*", NONE, Assoc::NONE, "new char(#str[1])", "delete #obj;", "d[^abc]" },
          { "FOR", "", NONE, Assoc::NONE, "", "", "for" },
          { "H[ab]]LLO", "", NONE, Assoc::NONE, "", "", "h[ab\\]]llo" },
          { "H.LLO", "", NONE, Assoc::NONE, "", "", "h\\.llo" },
          { "H_LLO", "", NONE, Assoc::NONE, "", "", "h.llo" },
          { "STRLIT", "std::string", NONE, Assoc::NONE, "#str.substr(1, #str.size() - 2)", "", R"("([^"]|\\")*")" },
          { "NUMBER", "int", NONE, Assoc::NONE, "std::stoi(std::string(#str))", "", "[1-9][0-9]*" },
          { "IDENT", "std::string", NONE, Assoc::NONE, "#str", "", "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*" },
          { "whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+" },
      },
      {},
      {}
  };

  generateLexerCode("test/parser/generated/random_lexer", "", grammarData);

  return 0;
}
