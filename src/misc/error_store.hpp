#ifndef ERROR_STORE_HPP
#define ERROR_STORE_HPP

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


class ErrorStore {
public:
  static constexpr char errorColored[] = "\033[1;31mError\033[0m";
  static constexpr char warningColored[] = "\033[1;34mWarning\033[0m";
  static constexpr char noteColored[] = "\033[1;35mWarning\033[0m";

  ErrorStore() = default;
  ~ErrorStore() = default;
  ErrorStore(const ErrorStore&) = delete;
  ErrorStore(ErrorStore&&) = default;
  ErrorStore& operator=(const ErrorStore&) = delete;
  ErrorStore& operator=(ErrorStore&&) = default;

  std::stringstream& addError(size_t line = 0, std::string_view msg = "");
  std::stringstream& addWarning(size_t line = 0, std::string_view msg = "");
  std::stringstream& addNote(size_t line = 0, std::string_view msg = "");

  /* Throw if there were any MsgType::ERRORs */
  template<typename ExceptType = std::runtime_error>
  void displayErrors() {
  if (errorCount_ == 0) {
    for (std::stringstream& stream : errors_) {
      std::cerr << stream.str() << std::endl;
    }
  } else {
    std::string allErrs;
    for (std::stringstream& stream : errors_) {
      allErrs.append(stream.str());
    }
    throw ExceptType(allErrs);
  }
}

private:
  enum class MsgType {ERROR, WARNING, NOTE};
  inline std::stringstream&  add(MsgType type, size_t line, std::string_view msg);

  std::vector<std::stringstream> errors_;
  size_t errorCount_ = 0;
};




#endif // ERROR_STORE_HPP
