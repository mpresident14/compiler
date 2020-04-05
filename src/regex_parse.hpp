#ifndef REGEX_PARSE_HPP
#define REGEX_PARSE_HPP

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
#include <stdexcept>
#include <sstream>

RgxPtr parse(const std::string& input);

#endif
