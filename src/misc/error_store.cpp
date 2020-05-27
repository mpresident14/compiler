#include "src/misc/error_store.hpp"

using namespace std;

stringstream& Logger::logError(size_t line, string_view msg) {
  ++errorCount_;
  return log(MsgType::ERROR, line, msg);
}

stringstream& Logger::logWarning(size_t line, string_view msg) {
  return log(MsgType::WARNING, line, msg);
}

stringstream& Logger::logNote(size_t line, string_view msg) {
  return log(MsgType::NOTE, line, msg);
}

stringstream& Logger::log(MsgType type, size_t line, string_view msg) {
  errors_.push_back(stringstream());
  stringstream& error = errors_.back();

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

  if (!msg.empty()) {
    error << msg << '\n';
  }
  return error;
}
