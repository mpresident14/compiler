#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parser/utils.hpp"
#include "src/parser/config_parse.hpp"

#include <ostream>
#include <string>

void generateParserCode(
    const std::string& parserFilePath,
    const ParseInfo& parseInfo);

void generateLexerCode(
    const std::string& lexerFilePath,
    const std::string& addlHdrIncludes,
    const GrammarData& grammarData);

#endif
