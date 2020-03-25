#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

// TODO: When generated, these lines will be replaced by the
// user's grammar file and the generated file names
// WARNING: Depends on write_dfa.cpp
#include "test/tokenize_grammar.hpp"

#include <prez/print_stuff.hpp>

#include <optional>
#include <string_view>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <functional>
#include <algorithm>
#include <cctype>

std::vector<StackObj> tokenize(const std::string& input);



#endif
