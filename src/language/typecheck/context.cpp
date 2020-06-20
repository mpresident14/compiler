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

string qualifiedFn(vector<string> qualifiers, string_view fnName) {
  if (qualifiers.empty()) {
    return string(fnName);
  }
  return boost::join(qualifiers, "::").append("::").append(fnName);
}

}  // namespace

void Ctx::streamParamTypes(const vector<TypePtr>& paramTypes, ostream& err) {
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
    shared_ptr<unordered_map<string, string>> fileIds,
    shared_ptr<unordered_map<string, string>> typeIds)
    : filename_(filename),
      logger(filename),
      fileIds_(move(fileIds)),
      typeIds_(move(typeIds)) {}

Logger& Ctx::getLogger() noexcept { return logger; }

Ctx::CtxTree& Ctx::getCtxTree() noexcept { return ctxTree_; };

const string& Ctx::getFilename() const noexcept { return filename_; }

const Type& Ctx::getCurrentRetType() const noexcept { return *currentRetType_; }

void Ctx::setCurrentRetType(TypePtr type) noexcept {
  currentRetType_ = move(type);
}

int Ctx::insertVar(string_view name, TypePtr type, size_t line) {
  int temp = newTemp();
  auto insertResult =
      varMap_.emplace(name, VarInfo{ move(type), temp, line, false });
  if (!insertResult.second) {
    auto& errStream = logger.logError(line);
    errStream << "Redefinition of variable '" << name
              << "'. Originally declared on line "
              << insertResult.first->second.line;
  }
  return temp;
}


const Ctx::VarInfo* Ctx::lookupVar(const string& name, size_t line) {
  const VarInfo* varInfo = lookupTempVar(name);
  if (!varInfo) {
    logger.logError(
        line, string("Undefined variable '").append(name).append("'"));
  }
  return varInfo;
}

const Ctx::VarInfo* Ctx::lookupTempVar(const string& name) {
  auto iter = varMap_.find(name);
  if (iter == varMap_.end()) {
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
  auto iter = varMap_.find(var);
  if (iter != varMap_.end() && !iter->second.used) {
    logger.logWarning(
        line, string("Unused variable '").append(var).append("'"));
  }
  varMap_.erase(var);
}


void Ctx::insertFn(
    const string& name,
    const vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line) {
  auto iterPair = fnMap_.equal_range(name);
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
      errStream << "'. Originally declared at " << fnInfo.declFile << ", line "
                << fnInfo.line;
      return;
    }
  }
  fnMap_.emplace(name, FnInfo{ paramTypes, move(returnType), filename_, line });
}


// TODO: Need to allow implicit conversions between integral types
pair<
    const Ctx::FnInfo*,
    pair<
        unordered_multimap<string, Ctx::FnInfo>::iterator,
        unordered_multimap<string, Ctx::FnInfo>::iterator>>
Ctx::lookupFn(const string& name, const vector<TypePtr>& paramTypes) {
  auto iterPair = fnMap_.equal_range(name);
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
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
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
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "'. No imported file matches qualifiers.";
}

void Ctx::undefinedFn(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const pair<
        unordered_multimap<string, Ctx::FnInfo>::iterator,
        unordered_multimap<string, Ctx::FnInfo>::iterator>& candidates,
    string_view searchedFile) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName);
  streamParamTypes(paramTypes, err);

  if (candidates.first == candidates.second) {
    err << "'\nNo candidate functions in " << searchedFile;
  } else {
    err << "'\nCandidate functions in " << searchedFile << ":";
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
    const string& filename,
    const vector<TypePtr>& paramTypes) {
  if (fnName.front() != '_') {
    string newName = "_";
    newName.append(fnName);
    newName.append(fileIds_->at(filename));

    for (const TypePtr& type : paramTypes) {
      newName.push_back('_');
      newName.append(type->getId(*typeIds_));
    }
    return newName;
  }
  return string(fnName);
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
