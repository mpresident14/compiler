#ifndef CONFIG_GRAMMAR_HPP
#define CONFIG_GRAMMAR_HPP

#include "utils.hpp"

GrammarData CONFIG_GRAMMAR = {
  {
      { "HEADER", "", NONE, Assoc::NONE, "", "", "#header" },
      { "SOURCE", "", NONE, Assoc::NONE, "", "", "#source" },
      { "TOKENS", "", NONE, Assoc::NONE, "", "", "#tokens" },
      { "PREC", "", NONE, Assoc::NONE, "", "", "#prec" },
      { "GRAMMAR", "", NONE, Assoc::NONE, "", "", "#grammar" },
      { "DEFINED", "", NONE, Assoc::NONE, "", "", ":=" },
      { "BAR", "", NONE, Assoc::NONE, "", "", "\\|" },
      { "ARROW", "", NONE, Assoc::NONE, "", "", "->" },
      { "IDENT", "string", NONE, Assoc::NONE, "str", "", "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*\\**&*" },
      { "STRLIT", "string", NONE, Assoc::NONE, "extractString(str, '\"')", "", R"("([^"]|\\")*")" },
      { "CODE", "string", NONE, Assoc::NONE, "extractString(str, '%')", "", R"(%([^%]|\\%)*%)" },
  },
  {},
  {}
};

const char ADDL_CODE[] =
    R"(
      using namespace std;
      string extractString(string_view str, char escaped) {
        string_view lit = str.substr(1, str.size() - 2);
        size_t len = lit.size();
        string ret;
        ret.reserve(len);
        for (size_t i = 0; i < len - 1; ++i) {
          char c = lit[i];
          if (c == '\\' && lit[i+1] == escaped) {
            continue;
          }
          ret.append(1, c);
        }
        ret.append(1, lit[len-1]);
        return ret;
      }
    )";

#endif
