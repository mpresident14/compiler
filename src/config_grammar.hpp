#ifndef CONFIG_GRAMMAR_HPP
#define CONFIG_GRAMMAR_HPP

#include "utils.hpp"

enum TokenId {
  HEADER = -1,
  SOURCE = -2,
  TOKENS = -3,
  PREC = -4,
  GRAMMAR = -5,
  DEFINED = -6,
  BAR = -7,
  ARROW = -8,
  LEFTASSOC = -9,
  RIGHTASSOC = -10,
  NONASSOC = -11,
  IDENT = -12,
  STRLIT = -13,
  CODE = -14,
};

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
      { "LEFTASSOC", "", NONE, Assoc::NONE, "", "", "#left" },
      { "RIGHTASSOC", "", NONE, Assoc::NONE, "", "", "#right" },
      { "NONASSOC", "", NONE, Assoc::NONE, "", "", "#nonassoc" },
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
