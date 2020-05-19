#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <sstream>
#include <stdexcept>
#include <string>

inline const char* errorColored = "\033[1;31mError\033[0m";
inline const char* warningColored = "\033[1;34mWarning\033[0m";

void inline throwIfError(const std::stringstream& stream) {
  const std::string& str = stream.str();
  if (!str.empty()) {
    throw std::runtime_error(str.c_str());
  }
}

#endif // ERRORS_HPP
