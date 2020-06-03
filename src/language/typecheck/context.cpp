#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/join.hpp>
#include <prez/print_stuff.hpp>

using namespace std;

namespace {

std::string qualifiedFn(
    std::vector<std::string> qualifiers,
    std::string_view fnName) {
  if (qualifiers.empty()) {
    return string(fnName);
  }
  return boost::join(qualifiers, "::").append("::").append(fnName);
}

}

void Ctx::streamParamTypes(
    const std::vector<TypePtr>& paramTypes,
    ostream& err) {
  err << '(';
  if (!paramTypes.empty()) {
    for (auto iter = paramTypes.cbegin(); iter != prev(paramTypes.cend());
         ++iter) {
      err << **iter << ", ";
    }
    err << *paramTypes.back();
  }
  err << ')';
}


Ctx::Ctx(
    string_view filename,
    std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds,
    std::shared_ptr<std::unordered_map<std::string, std::string>> typeIds)
    : filename_(filename),
      logger(filename),
      fileIds_(move(fileIds)),
      typeIds_(move(typeIds)) {}

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
    errStream << "Redefinition of variable '" << name
              << "'. Originally declared on line "
              << insertResult.first->second.line;
  }
  return temp;
}


const Ctx::VarInfo* Ctx::lookupVar(const string& name, size_t line) {
  auto iter = varMap.find(name);
  if (iter == varMap.end()) {
    // We can't really continue from this error
    logger.logError(
        line, string("Undefined variable '").append(name).append("'"));
    return nullptr;
  }
  VarInfo& varInfo = iter->second;
  varInfo.used = true;
  return &varInfo;
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
        line, string("Unused variable '").append(var).append("'"));
  }
  varMap.erase(var);
}


void Ctx::insertFn(
    const std::string& name,
    const std::vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line) {
  auto iterPair = fnMap.equal_range(name);
  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const FnInfo& fnInfo = iter->second;
    const vector<TypePtr>& fnParamTypes = fnInfo.paramTypes;
    if (equal(
            paramTypes.cbegin(),
            paramTypes.cend(),
            fnParamTypes.cbegin(),
            fnParamTypes.cend())) {
      ostream& errStream = logger.logError(line);
      errStream << "Redefinition of function '" << name;
      Ctx::streamParamTypes(paramTypes, errStream);
      errStream << "'. Originally declared at " << fnInfo.declFile
                << ", line " << fnInfo.line;
      return;
    }
  }
  fnMap.emplace(name, FnInfo{ paramTypes, move(returnType), filename_, line });
}


pair<
    const Ctx::FnInfo*,
    std::pair<
        std::unordered_multimap<std::string, Ctx::FnInfo>::iterator,
        std::unordered_multimap<std::string, Ctx::FnInfo>::iterator>>
Ctx::lookupFn(const string& name, const std::vector<TypePtr>& paramTypes) {
  auto iterPair = fnMap.equal_range(name);
  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const vector<TypePtr>& fnParamTypes = iter->second.paramTypes;
    if (equal(
            paramTypes.cbegin(),
            paramTypes.cend(),
            fnParamTypes.cbegin(),
            fnParamTypes.cend())) {
      return { &iter->second, move(iterPair) };
    }
  }

  return { nullptr, move(iterPair) };
}

// TODO: Allow "from <file> import <function/class>" ???
const Ctx::FnInfo* Ctx::lookupFnRec(
    const std::vector<std::string>& qualifiers,
    const std::string& name,
    const std::vector<TypePtr>& paramTypes,
    size_t line) {
  if (qualifiers.empty()) {
    // If no qualifiers, we only try our own context
    auto infoAndIters = lookupFn(name, paramTypes);
    if (infoAndIters.first) {
      return infoAndIters.first;
    }
    undefinedFn(
        qualifiers, name, paramTypes, line, infoAndIters.second, filename_);
    return nullptr;
  }
  return ctxTree_.lookupFn(qualifiers, name, paramTypes, *this, line);
}


void Ctx::undefinedFn(
    const std::vector<std::string>& qualifiers,
    std::string_view fnName,
    const std::vector<TypePtr>& paramTypes,
    size_t line) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName)<< '\'';
  streamParamTypes(paramTypes, err);
  err << ". No imported file matches qualifiers.";
}

void Ctx::undefinedFn(
    const vector<string>& qualifiers,
    std::string_view fnName,
    const std::vector<TypePtr>& paramTypes,
    size_t line,
    const std::pair<
        std::unordered_multimap<std::string, Ctx::FnInfo>::iterator,
        std::unordered_multimap<std::string, Ctx::FnInfo>::iterator>&
        candidates,
    std::string_view searchedFile) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName) << '\'';
  streamParamTypes(paramTypes, err);

  if (candidates.first == candidates.second) {
    err << "\nNo candidate functions in " << searchedFile;
  } else {
    err << "\nCandidate functions in " << searchedFile << ":";
    for (auto iter = candidates.first; iter != candidates.second; ++iter) {
      const FnInfo& fnInfo = iter->second;
      err << "\n\tLine " << fnInfo.line << ": " << *fnInfo.returnType << ' '
          << fnName;
      streamParamTypes(fnInfo.paramTypes, err);
    }
  }
}


string Ctx::mangleFn(
    string_view fnName,
    const std::string& filename,
    const std::vector<TypePtr>& paramTypes) {
  // TODO: Remove runPrez and printInt when applicable
  if (fnName.front() == '_' || fnName == "runprez" || fnName == "printInt") {
    return string(fnName);
  }

  string newName(fnName);
  newName.append(fileIds_->at(filename));

  for (const TypePtr& type : paramTypes) {
    newName.push_back('_');
    newName.append(type->getId(*typeIds_));
  }
  return newName;
}

void Ctx::addFileId(size_t id, string_view filename) {
  fileIds_->emplace(filename, to_string(id));
}

void Ctx::typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = logger.logError(line);
  err << "Expected " << expected << ", got " << got;
}

void Ctx::displayLogs() const { logger.streamLog(); }

bool Ctx::hasErrors() const noexcept { return logger.hasErrors(); }
