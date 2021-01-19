#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parser/parse_info.hpp"
#include "src/parser/utils.hpp"

#include <ostream>
#include <sstream>
#include <string_view>

void generateParserCode(const ParseInfo &parseInfo,
                        const ParseFlags &parseFlags, std::ostream &warnings);

void generateLexerCode(std::string_view outDir,
                       std::string_view lexerIncludeDir,
                       std::string_view lexerName,
                       std::string_view lexerIncludePath,
                       const GrammarData &gd);

#endif
