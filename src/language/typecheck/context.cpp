#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;


namespace ctx {
namespace {
  std::unordered_map<std::string, VarInfo> varMap;
  // TODO: Remove initialization when we add a print() to the language
  std::unordered_map<std::string, FnInfo> fnMap = {
    { "printInt",
      FnInfo{ vector<Type>{ { TypeName::INT, "" } },
              { TypeName::VOID, "" }, "", 0 } }
  };
  std::string currentFn;
  /* Separate Logger for each source file */
  std::unordered_map<std::string, Logger> loggers;
  std::string currentFile;
  Logger* currentLogger = nullptr;


  void insertTemp(string_view name, const Type& type, int temp, size_t line) {
    auto insertResult =
        varMap.emplace(name, VarInfo{ move(type), temp, line, false });
    if (!insertResult.second) {
      auto& errStream = currentLogger->logError(line);
      errStream << "Redefinition of variable \"" << name
                << "\". Originally declared on line "
                << insertResult.first->second.line;
    }
  }

  void removeTemp(const std::string& var, size_t line) {
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


void setCurrentFile(const std::string& fileName) {
  currentFile = fileName;
  currentLogger = &loggers.at(fileName);
}


Logger& addLogger(const std::string& srcFileName) {
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
  insertTemp(name, type, temp, line);
  return temp;
}


void insertParam(
    string_view name,
    const Type& type,
    MachineReg reg,
    size_t line) {
  insertTemp(name, type, reg, line);
}


const VarInfo& lookupVar(const std::string& name, size_t line) {
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


void removeVars(const vector<std::pair<std::string, size_t>>& vars) {
  for (const auto& [var, line] : vars) {
    removeTemp(var, line);
  }
}

void removeParams(const vector<std::string>& params, size_t line) {
  for (const string& param : params) {
    removeTemp(param, line);
  }
}

void insertFn(
    string_view name,
    std::vector<Type>&& paramTypes,
    const Type& returnType,
    size_t line) {
  auto insertResult =
      fnMap.emplace(name, FnInfo{ move(paramTypes), move(returnType), currentFile, line });
  if (!insertResult.second) {
    auto& errStream = currentLogger->logError(line);
    errStream << "Redefinition of function \"" << name
              << "\". Originally declared on line "
              << insertResult.first->second.line;
  }
}

// TODO: Handle function overloads (maybe by giving them different names
// depending on params)
const FnInfo& lookupFn(const std::string& name, size_t line) {
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
