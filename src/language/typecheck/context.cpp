#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>

using namespace std;


void Context::addLogger(const std::string& srcFileName) {
  loggers_.emplace(srcFileName, Logger(srcFileName));
  currentFile_ = srcFileName;
}

Logger& Context::currentLogger() { return loggers_.at(currentFile_); }

int Context::insertVar(const std::string& name, const Type& type, size_t line) {
  int temp = newTemp();
  if (!varMap_.emplace(name, VarInfo{ move(type), temp }).second) {
    currentLogger().logError(line, "Redefinition of variable \"" + name + "\"");
  }
  return temp;
}

void Context::insertParam(
    const std::string& name,
    const Type& type,
    MachineReg reg, size_t line) {
  if (!varMap_.emplace(name, VarInfo{ move(type), reg }).second) {
    currentLogger().logError(line, "Redefinition of variable \"" + name + "\"");
  }
}

const Context::VarInfo& Context::lookupVar(const std::string& name, size_t line) {
  auto iter = varMap_.find(name);
  if (iter == varMap_.end()) {
    // We can't really continue from this error
    currentLogger().logFatal(line, "Undefined variable \"" + name + "\"");
    throw invalid_argument("");
  }
  return iter->second;
}

void Context::removeVar(const string& name) { varMap_.erase(name); }


void Context::insertFn(
    const std::string& name,
    std::vector<Type>&& paramTypes,
    const Type& returnType, size_t line) {
  if (!fnMap_.emplace(name, FnInfo{ move(paramTypes), move(returnType) })
           .second) {
    currentLogger().logError(line, "Redefinition of function \"" + name + "\"");
  }
}

const Context::FnInfo& Context::lookupFn(const std::string& name, size_t line) {
  auto iter = fnMap_.find(name);
  if (iter == fnMap_.end()) {
    // We can't really continue from this error
    currentLogger().logFatal(line, "Undefined function \"" + name + "\"");
  }
  return iter->second;
}


bool Context::displayLogs() const {
  bool hasError = false;
  for (const auto& [_, logger] : loggers_) {
    logger.streamLog();
    hasError |= logger.hasErrors();
  }
  return hasError;
}

// TODO: Remove this when we add a print() to the language
Context::Context() {
  fnMap_.emplace(
      "printInt",
      FnInfo{ vector<Type>{ { TypeName::INT, "" } }, { TypeName::VOID, "" } });
}
