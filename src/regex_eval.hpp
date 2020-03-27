#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "utils.hpp"
#include "dfa.hpp"
#include "rules.hpp"
#include "build_parser.hpp"
#include "regex.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <algorithm>
#include <functional>
#include <optional>


RgxPtr parse(const std::string& pattern);

#endif
