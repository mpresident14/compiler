#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <sstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

constexpr char errorColored[] = "\033[1;31mError\033[0m";
constexpr char warningColored[] = "\033[1;34mWarning\033[0m";


inline void streamError(std::ostream& errors, size_t line) {
  errors << errorColored << " on line " << line << ": ";
}

inline void streamError(std::ostream& errors, size_t line, std::string_view msg) {
  errors << errorColored << " on line " << line << ": " << msg << '\n';
}

inline void streamWarning(std::ostream& errors, size_t line) {
  errors << warningColored << " on line " << line << ": ";
}

inline void streamWarning(std::ostream& errors, size_t line, std::string_view msg) {
  errors << warningColored << " on line " << line << ": " << msg << '\n';
}

template<typename ExceptType = std::runtime_error>
void throwIfError(const std::stringstream& errors) {
  const std::string& str = errors.str();
  if (!str.empty()) {
    throw ExceptType(str.c_str());
  }
}

#endif // ERRORS_HPP
