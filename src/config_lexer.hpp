#ifndef CONFIG_LEXER_HPP
#define CONFIG_LEXER_HPP

#include "utils.hpp"

#include <vector>
#include <string>

void deleteObj(const StackObj& s);
std::vector<StackObj> tokenize(const std::string& input);

#endif
