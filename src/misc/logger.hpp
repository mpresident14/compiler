#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <string.h>

class Logger {
public:
  class Exception : public std::exception {
  public:
    const char* what() const noexcept override { return nullptr; }
  };

  static constexpr char errorColored[] = "\033[1;31mError\033[0m";
  static constexpr char warningColored[] = "\033[1;34mWarning\033[0m";
  static constexpr char noteColored[] = "\033[1;35mWarning\033[0m";

  Logger() = default;
  Logger(std::string_view srcFile);
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger(Logger&&) = default;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = default;

  std::ostringstream& logError(size_t line = 0, std::string_view msg = "");
  std::ostringstream& logWarning(size_t line = 0, std::string_view msg = "");
  std::ostringstream& logNote(size_t line = 0, std::string_view msg = "");
  bool hasErrors() const noexcept;
  void streamLog(std::ostream& out = std::cerr) const;

  void setSrcFile(const std::string& fileName) noexcept { srcFile_ = fileName; }

  template<typename FStream>
  void checkFile(const std::string& fileName, FStream& file) {
    if (!file.is_open()) {
      logFatal(0, "Could not open file " + fileName + ": " + strerror(errno));
    }
  }

  void logFatal(size_t line, std::string_view msg);
  void checkLog() const;

private:
  enum class MsgType { ERROR, WARNING, NOTE };

  std::ostringstream& log(MsgType type, size_t line, std::string_view msg);

  std::vector<std::ostringstream> logs_;
  size_t errorCount_ = 0;
  std::string srcFile_;
};


#endif  // LOGGER_HPP
