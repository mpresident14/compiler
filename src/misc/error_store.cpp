#include "src/misc/error_store.hpp"

using namespace std;

stringstream& ErrorStore::addError(size_t line, string_view msg) {
  ++errorCount_;
  return add(MsgType::ERROR, line, msg);
}

stringstream& ErrorStore::addWarning(size_t line, string_view msg) {
  return add(MsgType::WARNING, line, msg);
}

stringstream& ErrorStore::addNote(size_t line, string_view msg) {
  return add(MsgType::NOTE, line, msg);
}

stringstream& ErrorStore::add(MsgType type, size_t line, string_view msg) {
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
      default: throw invalid_argument("Unknown MsgType");
    }
    error << " on line " << line << ": ";
  }

  if (!msg.empty()) {
    error << msg << '\n';
  }
  return error;
}

