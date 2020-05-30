#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;


namespace ctx {
namespace {
  unordered_map<string, VarInfo> varMap;
  // TODO: Remove initialization when we add a print() to the language
  unordered_map<string, FnInfo> fnMap = {
    { "printInt",
      FnInfo{ vector<TypePtr>{ intType },
              voidType, "", 0 } }
  };
  string currentFn;
  /* Separate Logger for each source file */
  unordered_map<string, Logger> loggers;
  string currentFile;
  Logger* currentLogger = nullptr;


  void removeTemp(const string& var, size_t line) {
    // If we had a variable redefinition error, we may have already removed this
    // variable
    auto iter = varMap.find(var);
    if (iter != varMap.end() && !iter->second.used) {
      currentLogger->logWarning(
          line, string("Unused variable \"").append(var).append("\""));
    }
    varMap.erase(var);
  }
}  // namespace


void setCurrentFn(string_view fnName) { currentFn = fnName; }


const string& getCurrentFn() noexcept { return currentFn; }


void setCurrentFile(const string& fileName) {
  currentFile = fileName;
  currentLogger = &loggers.at(fileName);
}


Logger& addLogger(const string& srcFileName) {
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


int insertVar(string_view name, TypePtr type, size_t line) {
  int temp = newTemp();
  auto insertResult =
        varMap.emplace(name, VarInfo{ move(type), temp, line, false });
    if (!insertResult.second) {
      auto& errStream = currentLogger->logError(line);
      errStream << "Redefinition of variable \"" << name
                << "\". Originally declared on line "
                << insertResult.first->second.line;
    }
  return temp;
}


const VarInfo& lookupVar(const string& name, size_t line) {
  auto iter = varMap.find(name);
  if (iter == varMap.end()) {
    // We can't really continue from this error
    currentLogger->logFatal(
        line, string("Undefined variable \"").append(name).append("\""));
    throw invalid_argument("");
  }
  VarInfo& varInfo = iter->second;
  varInfo.used = true;
  return varInfo;
}


void removeVars(const vector<pair<string, size_t>>& vars) {
  for (const auto& [var, line] : vars) {
    removeTemp(var, line);
  }
}

void removeParams(const vector<string>& params, size_t line) {
  for (const string& param : params) {
    removeTemp(param, line);
  }
}

void insertFn(
    string_view name,
    const std::vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line) {
  auto insertResult =
      fnMap.emplace(name, FnInfo{ paramTypes, move(returnType), currentFile, line });
  if (!insertResult.second) {
    auto& errStream = currentLogger->logError(line);
    errStream << "Redefinition of function \"" << name
              << "\". Originally declared on line "
              << insertResult.first->second.line;
  }
}

// TODO: Handle function overloads (maybe by giving them different names
// depending on params)
const FnInfo& lookupFn(const string& name, size_t line) {
  auto iter = fnMap.find(name);
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
