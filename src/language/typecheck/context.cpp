#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/join.hpp>

using namespace std;


Ctx::Ctx(string_view fileName) : fileName_(fileName) {}

Logger& Ctx::getLogger() noexcept { return logger; }

Ctx::CtxTree& Ctx::getCtxTree() noexcept { return ctxTree_; };

const string& Ctx::getCurrentFn() const noexcept { return currentFn; }

void Ctx::setCurrentFn(string_view fnName) { currentFn = fnName; }


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
  auto insertResult = fnMap.emplace(
      name, FnInfo{ paramTypes, move(returnType), fileName_, line });
  if (!insertResult.second) {
    auto& errStream = logger.logError(line);
    errStream << "Redefinition of function \"" << name
              << "\". Originally declared on line "
              << insertResult.first->second.line;
  }
}

// TODO: Handle function overloads (maybe by giving them different names
// depending on params)
const Ctx::FnInfo* Ctx::lookupFn(const string& name) {
  auto iter = fnMap.find(name);
  if (iter == fnMap.end()) {
    return nullptr;
  }
  return &iter->second;
}


const Ctx::FnInfo& Ctx::lookupFnRec(
    const std::vector<std::string> qualifiers,
    const std::string& name,
    size_t line) {
  if (qualifiers.empty()) {
    // If no qualifiers we only try our own context
    const Ctx::FnInfo* fnInfo = lookupFn(name);
    if (!fnInfo) {
      undefinedFn(qualifiers, name, line);
    }
    return *fnInfo;
  }
  return ctxTree_.lookupFn(qualifiers, name, line, *this);
}


void Ctx::undefinedFn(
    const vector<string> qualifiers,
    const string& fnName,
    size_t line) {
  string fullFn = boost::join(qualifiers, "::").append("::").append(fnName);
  logger.logFatal(line, "Undefined function " + fullFn);
}

void typeError(const Type& expected, const Type& got, size_t line, Ctx& ctx) {
  ostringstream& err = ctx.getLogger().logError(line);
  err << "Expected " << expected << ", got " << got;
}

bool Ctx::displayLogs() const {
  logger.streamLog();
  return logger.hasErrors();
}
