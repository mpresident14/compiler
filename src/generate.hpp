#ifndef GENERATE_HPP
#define GENERATE_HPP

#include "utils.hpp"
#include "build_parser.hpp"
#include "regex_eval.hpp"

#include <fstream>
#include <ostream>
#include <cstddef>
#include <string>
#include <sstream>

void generateCode(const std::string& parserFilePath, const std::string& classFile, const std::string& addlCode, const GrammarData& grammarData);

#endif
