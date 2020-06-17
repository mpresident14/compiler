#include "src/misc/logger.hpp"


using namespace std;


Logger::Logger(string_view filename) : filename_(filename) {}

ostringstream& Logger::logError(size_t line, string_view msg) {
  return log(MsgType::ERROR, line, msg);
}

ostringstream& Logger::logWarning(size_t line, string_view msg) {
  return log(MsgType::WARNING, line, msg);
}

ostringstream& Logger::logNote(size_t line, string_view msg) {
  return log(MsgType::NOTE, line, msg);
}

ostringstream& Logger::log(MsgType msgType, size_t line, string_view msg) {
  logs_.push_back({line, ostringstream()});
  ostringstream& error = logs_.back().msg;

  switch (msgType) {
    case MsgType::ERROR:
      ++errorCount_;
      error << errorColored;
      break;
    case MsgType::WARNING:
      ++warningCount_;
      error << warningColored;
      break;
    case MsgType::NOTE:
      ++noteCount_;
      error << noteColored;
      break;
    default:
      throw invalid_argument("Unknown MsgType");
  }

  if (line != 0) {
    error << " on line " << line;
  }
  error << ": " << msg;
  return error;
}


bool Logger::hasErrors() const noexcept { return errorCount_ != 0; }


namespace {
const char* maybePlural(size_t n, const char* singular, const char* plural) {
  return n == 1 ? singular : plural;
}
}  // namespace


void Logger::streamLog(ostream& out) const {
  if (logs_.empty()) {
    return;
  }

  if (!filename_.empty()) {
    out << filename_ << ":\n";
  }

  for (const auto& [_, str] : sortLogs()) {
    out << str << '\n';
  }
  out << errorCount_ << maybePlural(errorCount_, " error, ", " errors, ")
      << warningCount_
      << maybePlural(warningCount_, " warning, ", " warnings, ") << noteCount_
      << maybePlural(noteCount_, " note\n\n", " notes\n\n");
}

void Logger::logFatal(size_t line, string_view msg) {
  logError(line, msg);
  throw Exception(*this);
}

std::set<std::pair<size_t, std::string>> Logger::sortLogs() const {
  std::set<std::pair<size_t, std::string>> logSet;
  for (const auto& [line, stream] : logs_) {
    logSet.insert({line, stream.str()});
  }
  return logSet;
}

Logger::Exception::Exception(const Logger& logger) {
  if (!logger.filename_.empty()) {
    what_.append(logger.filename_).append(":\n");
  }
  for (const auto& [_, str] : logger.sortLogs()) {
    what_.append(str).push_back('\n');
  }
}

const char* Logger::Exception::what() const noexcept { return what_.c_str(); }
