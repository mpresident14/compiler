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
      { "STRLIT", "string", NONE, Assoc::NONE, "str.substr(1, str.size() - 2)", "", R"("([^"]|\\")*")" },
      { "CODE", "string", NONE, Assoc::NONE, "str.substr(1, str.size() - 2)", "", R"(%([^%]|\\%)*%)" },
  },
  {},
  {}
};

#endif
