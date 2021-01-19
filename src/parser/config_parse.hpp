#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "src/misc/logger.hpp"
#include "src/parser/utils.hpp"
#include "src/parser/parse_info.hpp"

#include <ostream>
#include <string>
#include <vector>

ParseInfo parseConfig(const std::string &filename, std::ostream &warnings);

#endif
