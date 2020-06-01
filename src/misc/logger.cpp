#include "src/misc/logger.hpp"

using namespace std;


Logger::Logger(string_view filename) : filename_(filename) {}

ostringstream& Logger::logError(size_t line, string_view msg) {
  ++errorCount_;
  return log(MsgType::ERROR, line, msg);
}

ostringstream& Logger::logWarning(size_t line, string_view msg) {
  return log(MsgType::WARNING, line, msg);
}

ostringstream& Logger::logNote(size_t line, string_view msg) {
  return log(MsgType::NOTE, line, msg);
}

ostringstream& Logger::log(MsgType type, size_t line, string_view msg) {
  logs_.push_back(ostringstream());
  ostringstream& error = logs_.back();

  if (line != 0) {
    switch (type) {
      case MsgType::ERROR:
        error << errorColored;
        break;
      case MsgType::WARNING:
        error << warningColored;
        break;
      case MsgType::NOTE:
        error << noteColored;
        break;
      default:
        throw invalid_argument("Unknown MsgType");
    }
    error << " on line " << line << ": ";
  }

  error << msg;
  return error;
}


bool Logger::hasErrors() const noexcept { return errorCount_ != 0; }

// TODO: Make this operator<<
void Logger::streamLog(std::ostream& out) const {
  if (logs_.empty()) {
    return;
  }

  if (!filename_.empty()) {
    out << filename_ << ":\n";
  }

  for (const std::ostringstream& stream : logs_) {
    out << stream.str() << '\n';
  }
}

void Logger::logFatal(size_t line, std::string_view msg) {
  logError(line, msg);
  throw Exception(*this);
}


Logger::Exception::Exception(const Logger& logger) {
  if (!logger.filename_.empty()) {
    what_.append(logger.filename_).append(":\n");
  }
  for (const std::ostringstream& stream : logger.logs_) {
    what_.append(stream.str()).push_back('\n');
  }
}

const char* Logger::Exception::what() const noexcept { return what_.c_str(); }
