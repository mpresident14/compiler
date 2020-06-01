#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <string_view>
#include <vector>

class Logger {
public:
  class Exception : public std::exception {
  public:
    Exception(const Logger& logger);
    const char* what() const noexcept override;

  private:
    std::string what_;
  };

  friend class Exception;

  static constexpr char errorColored[] = "\033[1;31mError\033[0m";
  static constexpr char warningColored[] = "\033[1;35mWarning\033[0m";
  static constexpr char noteColored[] = "\033[1;34mWarning\033[0m";

  Logger() = default;
  Logger(std::string_view filename);
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

  template <typename FStream>
  void checkFile(std::string_view fileName, FStream& file) {
    if (!file.is_open()) {
      logFatal(
          0,
          std::string("Could not open file ")
              .append(fileName)
              .append(": ")
              .append(strerror(errno)));
    }
  }

  void logFatal(size_t line, std::string_view msg);

private:
  enum class MsgType { ERROR, WARNING, NOTE };

  std::ostringstream& log(MsgType type, size_t line, std::string_view msg);

  std::string filename_ = "";
  std::vector<std::ostringstream> logs_;
  size_t errorCount_ = 0;
};


#endif  // LOGGER_HPP
