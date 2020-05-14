#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "src/parser/utils.hpp"

#include <ostream>

void rgxDFAToCode(std::ostream &out, const GrammarData &grammarData);

#endif
