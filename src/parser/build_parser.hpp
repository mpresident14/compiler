#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "src/parser/dfa.hpp"
#include "src/parser/rules.hpp"
#include "src/parser/utils.hpp"

#include <optional>
#include <ostream>


DFA<DFARuleSet, int, DFARuleSetHash> buildParserDFA(
    const GrammarData& grammarData);
void condensedDFAToCode(std::ostream& out, const GrammarData& grammarData);

#endif
