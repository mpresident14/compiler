#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


class Logger {
public:
  static constexpr char errorColored[] = "\033[1;31mError\033[0m";
  static constexpr char warningColored[] = "\033[1;34mWarning\033[0m";
  static constexpr char noteColored[] = "\033[1;35mWarning\033[0m";

  Logger() = default;
  Logger(const std::string& srcFile);
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger(Logger&&) = default;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = default;

  std::stringstream& logError(size_t line = 0, std::string_view msg = "");
  std::stringstream& logWarning(size_t line = 0, std::string_view msg = "");
  std::stringstream& logNote(size_t line = 0, std::string_view msg = "");

  /* Throw if there were any MsgType::ERRORs */
  template <typename ExceptType = std::runtime_error>
  void displayLog() const {
    if (logs_.empty()) {
      return;
    }

    if (errorCount_ == 0) {
      std::cerr << srcFile_ << ":\n";
      for (const std::stringstream& stream : logs_) {
        std::cerr << stream.str() << std::endl;
      }
    } else {
      std::string allErrs(srcFile_);
      allErrs.append(":\n");
      for (const std::stringstream& stream : logs_) {
        allErrs.append(stream.str());
      }
      throw ExceptType(allErrs);
    }
  }

  void setSrcFile(const std::string& fileName) noexcept { srcFile_ = fileName; }

private:
  enum class MsgType { ERROR, WARNING, NOTE };

  inline std::stringstream&
  log(MsgType type, size_t line, std::string_view msg);

  std::vector<std::stringstream> logs_;
  size_t errorCount_ = 0;
  std::string srcFile_;
};


#endif  // LOGGER_HPP
