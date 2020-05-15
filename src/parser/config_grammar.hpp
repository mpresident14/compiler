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
  TYPE = -15,
  STRLIT = -16,
  CODE = -17,
};

GrammarData CONFIG_GRAMMAR = {
  {
      { "#header", "", NONE, Assoc::NONE, "", "", "#header" },
      { "#source", "", NONE, Assoc::NONE, "", "", "#source" },
      { "#tokens", "", NONE, Assoc::NONE, "", "", "#tokens" },
      { "#skip", "", NONE, Assoc::NONE, "", "", "#skip" },
      { "#prec", "", NONE, Assoc::NONE, "", "", "#prec" },
      { "#grammar", "", NONE, Assoc::NONE, "", "", "#grammar" },
      { "associativity marker", "", NONE, Assoc::NONE, "", "", "#left" },
      { "associativity marker", "", NONE, Assoc::NONE, "", "", "#right" },
      { "associativity marker", "", NONE, Assoc::NONE, "", "", "#nonassoc" },
      { ":=", "", NONE, Assoc::NONE, "", "", ":=" },
      { "|", "", NONE, Assoc::NONE, "", "", "\\|" },
      { "->", "", NONE, Assoc::NONE, "", "", "->" },
      { "COMMENT", "", SKIP_TOKEN, Assoc::NONE, "", "", "//[^\n]*\n" },
      { "identifier",
        "std::string",
        NONE,
        Assoc::NONE,
        "#str",
        "",
        "([a-z]|[A-Z])([a-z]|[A-Z]|[0-9])*" },
      { "type",
        "std::string",
        NONE,
        Assoc::NONE,
        "rmEnclosure(#str)",
        "",
        "{[^}]*}" },
      { "string literal",
        "std::string",
        NONE,
        Assoc::NONE,
        "extractString(#str, '\"')",
        "",
        R"("([^"]|\\")*")" },
      { "code block",
        "std::string",
        NONE,
        Assoc::NONE,
        "extractString(#str, '%')",
        "",
        R"(%([^%]|\\%)*%)" },
      { "whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+" },
  },
  {},
  {}
};

const char ADDL_CODE[] =
    R"(
      using namespace std;
      string rmEnclosure(string_view str) {
        return string(str.substr(1, str.size() - 2));
      }

      string extractString(string_view str, char escaped) {
        string ret;
        string_view lit = str.substr(1, str.size() - 2);
        if (lit.empty()) {
          return ret;
        }
        size_t len = lit.size();
        ret.reserve(len);
        for (size_t i = 0; i < len - 1; ++i) {
          char c = lit[i];
          if (c == '\\' && lit[i+1] == escaped) {
            continue;
          }
          ret.append(1, c);
        }
        ret.append(1, lit.back());
        return ret;
      }
    )";

#endif
