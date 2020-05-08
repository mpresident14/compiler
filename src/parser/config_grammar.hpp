#ifndef CONFIG_GRAMMAR_HPP
#define CONFIG_GRAMMAR_HPP

#include "src/parser/utils.hpp"

enum TokenId {
  HEADER = -1,
  SOURCE = -2,
  TOKENS = -3,
  SKIP = -4,
  PREC = -5,
  GRAMMAR = -6,
  LEFTASSOC = -7,
  RIGHTASSOC = -8,
  NONASSOC = -9,
  DEFINED = -10,
  BAR = -11,
  ARROW = -12,
  COMMENT = -13,
  IDENT = -14,
  STRLIT = -15,
  CODE = -16,
};

GrammarData CONFIG_GRAMMAR = {
  {
      { "HEADER", "", NONE, Assoc::NONE, "", "", "#header" },
      { "SOURCE", "", NONE, Assoc::NONE, "", "", "#source" },
      { "TOKENS", "", NONE, Assoc::NONE, "", "", "#tokens" },
      { "SKIP", "", NONE, Assoc::NONE, "", "", "#skip" },
      { "PREC", "", NONE, Assoc::NONE, "", "", "#prec" },
      { "GRAMMAR", "", NONE, Assoc::NONE, "", "", "#grammar" },
      { "LEFTASSOC", "", NONE, Assoc::NONE, "", "", "#left" },
      { "RIGHTASSOC", "", NONE, Assoc::NONE, "", "", "#right" },
      { "NONASSOC", "", NONE, Assoc::NONE, "", "", "#nonassoc" },
      { "DEFINED", "", NONE, Assoc::NONE, "", "", ":=" },
      { "BAR", "", NONE, Assoc::NONE, "", "", "\\|" },
      { "ARROW", "", NONE, Assoc::NONE, "", "", "->" },
      { "COMMENT", "", SKIP_TOKEN, Assoc::NONE, "", "", "//[^\n]*\n"},
      { "IDENT", "std::string", NONE, Assoc::NONE, "#str", "", "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*\\**&*" },
      { "STRLIT", "std::string", NONE, Assoc::NONE, "extractString(#str, '\"')", "", R"("([^"]|\\")*")" },
      { "CODE", "std::string", NONE, Assoc::NONE, "extractString(#str, '%')", "", R"(%([^%]|\\%)*%)" },
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
