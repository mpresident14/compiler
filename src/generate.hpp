#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "utils.hpp"

#include <string>
#include <ostream>

void generateCode(
    const std::string& parserFilePath,
    const std::string& addlHdrIncludes,
    const std::string& addlCode,
    const GrammarData& grammarData);

void lexerStuff(std::ostream& out, const GrammarData& grammarData);

#endif
