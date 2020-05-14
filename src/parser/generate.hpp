#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "src/parser/utils.hpp"

#include <ostream>
#include <string>

void generateParserCode(const std::string &parserFilePath,
                        const std::string &addlHdrIncludes,
                        const std::string &addlCode,
                        const GrammarData &grammarData);

void generateLexerCode(const std::string &lexerFilePath,
                       const std::string &addlHdrIncludes,
                       const GrammarData &grammarData);

#endif
