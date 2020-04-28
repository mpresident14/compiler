#include "generate.hpp"

#include <iostream>
#include <cstddef>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;



int main() {
  GrammarData grammarData = {
      {
          { "NOTABC", "char*", NONE, Assoc::NONE, "new char(str[0])", "delete #0;", "[^abc]" },
          { "FOR", "", NONE, Assoc::NONE, "", "", "for" },
          { "H[ab]]LLO", "", NONE, Assoc::NONE, "", "", "h[ab\\]].llo" },
          { "H.LLO_LIT", "", NONE, Assoc::NONE, "", "", "h\\.llo" },
          { "H.LLO", "", NONE, Assoc::NONE, "", "", "h.llo" },
          { "STRLIT", "string", NONE, Assoc::NONE, "str.substr(1, str.size() - 2)", "", R"("([^"]|\\")*")" },
          { "NUMBER", "int", NONE, Assoc::NONE, "stoi(string(str))", "", "[1-9][0-9]*" },
          { "IDENT", "string", NONE, Assoc::NONE, "str", "", "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*" },
      },
      {},
      {}
  };

  generateLexerCode("test/generated/random_lexer", "", grammarData);

  return 0;
}
