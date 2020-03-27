#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "utils.hpp"
#include "dfa.hpp"
#include "rules.hpp"
#include "build_parser.hpp"
#include "regex.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <algorithm>
#include <functional>
#include <optional>


struct GrammarData {
  std::vector<int> concToSymb;    // Indexed by Concrete
  std::vector<int> overridePrecs; // Indexed by Concrete
  std::vector<int> tokenPrecs;    // Indexed by Token
  std::vector<Assoc> tokenAssoc;  // Indexed by Token
};


#endif
