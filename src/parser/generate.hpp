#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parser/config_parse.hpp"
#include "src/parser/utils.hpp"

#include <ostream>
#include <sstream>
#include <string>

void generateParserCode(
    const ParseInfo& parseInfo, const ParseFlags& parseFlags, std::ostream& warnings);

void generateLexerCode(
    const std::string& lexerFilePath, const std::string& addlHdrIncludes, const GrammarData& gd);

#endif
