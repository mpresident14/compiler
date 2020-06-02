#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/join.hpp>

using namespace std;


Ctx::Ctx(
    string_view filename,
    std::shared_ptr<std::unordered_map<std::string, std::string>> fnEncodings,
    std::shared_ptr<std::unordered_map<std::string, std::string>> typeEncodings)
    : filename_(filename),
      logger(filename),
      fnEncodings_(fnEncodings),
      typeEncodings_(typeEncodings) {}

Logger& Ctx::getLogger() noexcept { return logger; }

Ctx::CtxTree& Ctx::getCtxTree() noexcept { return ctxTree_; };

const std::string& Ctx::getFilename() const noexcept { return filename_; }

const Type& Ctx::getCurrentRetType() const noexcept { return *currentRetType_; }

void Ctx::setCurrentRetType(TypePtr type) noexcept {
  currentRetType_ = move(type);
}

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
    std::string_view name,
    const std::vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line) {
  /* auto insertResult =  */
  fnMap.emplace(name, FnInfo{ paramTypes, move(returnType), filename_, line });
  // TODO
  // if (!insertResult.second) {
  //   auto& errStream = logger.logError(line);
  //   // TODO: Output parameter types
  //   errStream << "Redefinition of function \"" << name
  //             << "\". Originally declared on line "
  //             << insertResult.first->second.line;
  // }
}


const Ctx::FnInfo* Ctx::lookupFn(
    const string& name,
    const std::vector<TypePtr>& paramTypes) {
  auto iterPair = fnMap.equal_range(name);
  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const vector<TypePtr> fnParamTypes = iter->second.paramTypes;
    if (equal(
            paramTypes.cbegin(),
            paramTypes.cend(),
            fnParamTypes.cbegin(),
            fnParamTypes.cend())) {
      return &iter->second;
    }
  }

  return nullptr;
}

// TODO: Allow "from <file> import <function/class>""
const Ctx::FnInfo* Ctx::lookupFnRec(
    const std::vector<std::string>& qualifiers,
    const std::string& name,
    const std::vector<TypePtr>& paramTypes) {
  if (qualifiers.empty()) {
    // If no qualifiers we only try our own context
    const Ctx::FnInfo* fnInfo = lookupFn(name, paramTypes);
    return fnInfo;
  }

  const Ctx::FnInfo* fnInfo = ctxTree_.lookupFn(qualifiers, name, paramTypes);
  return fnInfo;
}


// TODO: Show parameters, possibly suggest alternatives
void Ctx::undefinedFn(
    const vector<string>& qualifiers,
    const string& fnName,
    const std::vector<TypePtr>& paramTypes,
    size_t line) {
  ostringstream err;
  err << "Undefined function "
      << boost::join(qualifiers, "::").append("::").append(fnName) << '(';
  if (!paramTypes.empty()) {
    for (auto iter = paramTypes.cbegin(); iter != prev(paramTypes.cend());
         ++iter) {
      err << **iter << ',';
    }
    err << *paramTypes.back();
  }
  err << ')';

  logger.logFatal(line, err.str());
}


string Ctx::mangleFn(
    string_view fnName,
    string_view filename,
    const std::vector<TypePtr>& paramTypes) {
  // TODO: Remove runPrez and printInt when applicable
  if (fnName.front() == '_' || fnName == "runprez" || fnName == "printInt") {
    return string(fnName);
  }

  // TODO: Use file encodings
  filename = filename.substr(0, filename.size() - sizeof(".prez") + 1);
  string newName;
  newName.append(filename).push_back('_');
  replace(newName.begin(), newName.end(), '/', '_');


  for (const TypePtr& type : paramTypes) {
    newName.push_back('_');
    newName.append(type->encode(*typeEncodings_));
  }
  return newName;
}


void Ctx::typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = logger.logError(line);
  err << "Expected " << expected << ", got " << got;
}

void Ctx::displayLogs() const { logger.streamLog(); }

bool Ctx::hasErrors() const noexcept { return logger.hasErrors(); }
