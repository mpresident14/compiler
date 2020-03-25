#ifndef REGEX_MATCH_HPP
#define REGEX_MATCH_HPP

#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <queue>
#include <numeric>
#include <functional>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <utility>

#include <prez/print_stuff.hpp>

using TokenPattern = std::pair<std::string, Symbol>;
using RgxDFA = DFA<RgxPtr, char>;

struct MergeData {
  std::vector<std::pair<const RgxDFA::Node*, Symbol>> states;
  Symbol token;
};

namespace std {
  template <>
  struct hash<MergeData> {
    size_t operator()(const MergeData& mergeData) const noexcept {
      const vector<pair<const RgxDFA::Node*, Symbol>>& states = mergeData.states;
      // mergeData will not be empty
      return (size_t)accumulate(
          states.cbegin() + 1,
          states.cend(),
          (uintptr_t)states[0].first,
          [](uintptr_t n, pair<const RgxDFA::Node*, Symbol> node) {
            return n ^ (uintptr_t)node.first;
          });
    }
  };
}  // namespace std

inline bool operator==(const MergeData& md1, const MergeData& md2) noexcept {
  return md1.states == md2.states;
}

using MergedRgxDFA = DFA<MergeData, char>;


bool matches(const std::string& pattern, const std::string& input);
MergedRgxDFA buildMergedRgxDFA(const std::vector<TokenPattern>& tokenPatterns);

std::vector<StackObj> tokenize(const std::string& input, const MergedRgxDFA::Node* dfaRoot);

void writeRegexDFA(
    const std::vector<TokenPattern> patterns,
    const std::string& filename = "regex_dfa.hpp",
    const std::string& namespaceName = "regex_dfa");
#endif
