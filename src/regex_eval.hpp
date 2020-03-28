#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "utils.hpp"
#include "regex.hpp"
#include "regex_parse.hpp"
#include "dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <queue>
#include <numeric>
#include <functional>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <utility>

#include <prez/print_stuff.hpp>


void rgxDFAToCode(
    std::ostream& out,
    const std::vector<std::pair<std::string, int>> patterns);

#endif
