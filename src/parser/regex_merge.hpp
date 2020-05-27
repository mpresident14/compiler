#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "src/parser/utils.hpp"

#include <ostream>

void mergedRgxDFAToCode(std::ostream& out, const GrammarData& gd);

#endif
