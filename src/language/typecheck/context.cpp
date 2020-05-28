#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>

using namespace std;


namespace ctx {
namespace {
  std::unordered_map<std::string, VarInfo> varMap;
  // TODO: Remove initialization when we add a print() to the language
  std::unordered_map<std::string, FnInfo> fnMap = {
    { "printInt",
      FnInfo{ vector<Type>{ { TypeName::INT, "" } },
              { TypeName::VOID, "" } } }
  };
  std::string currentFn;
  /* Separate Logger for each source file */
  std::unordered_map<std::string, Logger> loggers;
  std::string currentFile;
  Logger* currentLogger = nullptr;
}  // namespace


void setCurrentFn(string_view fnName) {currentFn = fnName;}


const string& getCurrentFn() noexcept { return currentFn; }


void setCurrentFile(string_view fileName) {
  currentFile = fileName;
  currentLogger = &loggers.at(string(fileName));
}


Logger& addLogger(string_view srcFileName) {
  loggers.emplace(srcFileName, Logger(srcFileName));
  setCurrentFile(srcFileName);
  return *currentLogger;
}


Logger& getLogger() {
  if (!currentLogger) {
    throw runtime_error("Logger::getLogger");
  }
  return *currentLogger;
}


int insertVar(string_view name, const Type& type, size_t line) {
  int temp = newTemp();
  if (!varMap.emplace(name, VarInfo{ move(type), temp }).second) {
    currentLogger->logError(
        line, string("Redefinition of variable \"").append(name).append("\""));
  }
  return temp;
}


void insertParam(
    string_view name,
    const Type& type,
    MachineReg reg,
    size_t line) {
  if (!varMap.emplace(name, VarInfo{ move(type), reg }).second) {
    currentLogger->logError(
        line, string("Redefinition of variable \"").append(name).append("\""));
  }
}


const VarInfo& lookupVar(string_view name, size_t line) {
  auto iter = varMap.find(string(name));
  if (iter == varMap.end()) {
    // We can't really continue from this error
    currentLogger->logFatal(
        line, string("Undefined variable \"").append(name).append("\""));
    throw invalid_argument("");
  }
  return iter->second;
}


void removeVar(string_view name) { varMap.erase(string(name)); }


void insertFn(
    string_view name,
    std::vector<Type>&& paramTypes,
    const Type& returnType,
    size_t line) {
  if (!fnMap.emplace(name, FnInfo{ move(paramTypes), move(returnType) })
           .second) {
    currentLogger->logError(
        line, string("Redefinition of function \"").append(name).append("\""));
  }
}

// TODO: Handle function overloads
const FnInfo& lookupFn(string_view name, size_t line) {
  auto iter = fnMap.find(string(name));
  if (iter == fnMap.end()) {
    // We can't really continue from this error
    currentLogger->logFatal(
        line, string("Undefined function \"").append(name).append("\""));
  }
  return iter->second;
}


bool displayLogs() {
  bool hasError = false;
  for (const auto& [_, logger] : loggers) {
    logger.streamLog();
    hasError |= logger.hasErrors();
  }
  return hasError;
}

}  // namespace ctx
