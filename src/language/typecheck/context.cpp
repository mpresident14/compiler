#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;

Ctx::Ctx(string_view fileName) : fileName_(fileName) {}

void Ctx::setCurrentFn(string_view fnName) { currentFn = fnName; }

const string& Ctx::getCurrentFn() const noexcept { return currentFn; }

Logger& Ctx::getLogger() const noexcept { return logger; }


int Ctx::insertVar(string_view name, TypePtr type, size_t line) {
  int temp = newTemp();
  auto insertResult =
        varMap.emplace(name, VarInfo{ move(type), temp, line, false });
    if (!insertResult.second) {
      auto& errStream = logger.logError(line);
      errStream << "Redefinition of variable \"" << name
                << "\". Originally declared on line "
                << insertResult.first->second.line;
    }
  return temp;
}


const Ctx::VarInfo& Ctx::lookupVar(const string& name, size_t line) {
  auto iter = varMap.find(name);
  if (iter == varMap.end()) {
    // We can't really continue from this error
    logger.logFatal(
        line, string("Undefined variable \"").append(name).append("\""));
    throw invalid_argument("");
  }
  VarInfo& varInfo = iter->second;
  varInfo.used = true;
  return varInfo;
}


void Ctx::removeVars(const vector<pair<string, size_t>>& vars) {
  for (const auto& [var, line] : vars) {
    removeTemp(var, line);
  }
}

void Ctx::removeParams(const vector<string>& params, size_t line) {
  for (const string& param : params) {
    removeTemp(param, line);
  }
}

void Ctx::removeTemp(const string& var, size_t line) {
  // If we had a variable redefinition error, we may have already removed this
  // variable
  auto iter = varMap.find(var);
  if (iter != varMap.end() && !iter->second.used) {
    logger.logWarning(
        line, string("Unused variable \"").append(var).append("\""));
  }
  varMap.erase(var);
}


void Ctx::insertFn(
    string_view name,
    const std::vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line) {
  auto insertResult =
      fnMap.emplace(name, FnInfo{ paramTypes, move(returnType), currentFile, line });
  if (!insertResult.second) {
    auto& errStream = logger.logError(line);
    errStream << "Redefinition of function \"" << name
              << "\". Originally declared on line "
              << insertResult.first->second.line;
  }
}

// TODO: Handle function overloads (maybe by giving them different names
// depending on params)
const FnInfo& Ctx::lookupFn(const string& name, size_t line) {
  auto iter = fnMap.find(name);
  if (iter == fnMap.end()) {
    // We can't really continue from this error
    logger.logFatal(
        line, string("Undefined function \"").append(name).append("\""));
  }
  return iter->second;
}


bool Ctx::displayLogs() {
  bool hasError = false;
  for (const auto& [_, logger] : loggers) {
    logger.streamLog();
    hasError |= logger.hasErrors();
  }
  return hasError;
}

