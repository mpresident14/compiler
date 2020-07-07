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
    for (auto iter = paramTypes.cbegin(); iter != prev(paramTypes.cend()); ++iter) {
      err << **iter << ", ";
    }
    err << *paramTypes.back();
  }
  err << ')';
}


Ctx::Ctx(
    string_view filename,
    const shared_ptr<unordered_map<string, string>>& fileIds,
    const shared_ptr<unordered_map<string, string>>& typeIds)
    : filename_(filename), logger(filename), fileIds_(fileIds), typeIds_(typeIds) {}

Logger& Ctx::getLogger() noexcept { return logger; }

Ctx::CtxTree& Ctx::getCtxTree() noexcept { return ctxTree_; };

const string& Ctx::getFilename() const noexcept { return filename_; }

const Type& Ctx::getCurrentRetType() const noexcept { return *currentRetType_; }

void Ctx::setCurrentRetType(const TypePtr& type) noexcept { currentRetType_ = type; }

int Ctx::insertVar(string_view name, const TypePtr& type, size_t line) {
  int temp = newTemp();
  auto insertResult = varMap_.emplace(name, VarInfo{ type, temp, line, false });
  if (!insertResult.second) {
    auto& errStream = logger.logError(line);
    errStream << "Redefinition of variable '" << name << "'. Originally declared on line "
              << insertResult.first->second.line;
  }
  return temp;
}


const Ctx::VarInfo* Ctx::lookupVar(const string& name, size_t line) {
  const VarInfo* varInfo = lookupTempVar(name);
  if (!varInfo) {
    logger.logError(line, string("Undefined variable '").append(name).append("'"));
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
    logger.logWarning(line, string("Unused variable '").append(var).append("'"));
  }
  varMap_.erase(var);
}


void Ctx::insertClass(
    const string& name,
    unordered_multimap<string, Ctx::FnInfo>&& methods,
    unordered_map<string, Ctx::FieldInfo>&& fields) {
  // We already check for redefinitions in Class::addToContext
  classMap_.emplace(name, ClassInfo{ move(methods), move(fields) });
}

const Ctx::ClassInfo* Ctx::lookupClass(const string& name) {
  auto iter = classMap_.find(name);
  if (iter == classMap_.end()) {
    return nullptr;
  }
  return &iter->second;
}

const Ctx::ClassInfo* Ctx::lookupClassRec(const vector<string>&, const string& name) {
  // TODO: Implement this
  return lookupClass(name);
}

const Ctx::FnInfo* Ctx::lookupMethod(
    const std::vector<std::string>& qualifiers,
    const std::string& className,
    const std::string& methodName,
    const std::vector<TypePtr>& paramTypes,
    size_t line) {
  const ClassInfo* classInfo = lookupClassRec(qualifiers, className);
  if (!classInfo) {
    throw runtime_error("Ctx::lookupMethod");
  }

  return lookupFnRec(classInfo->methods, {}, methodName, paramTypes, line);
}

void Ctx::insertFn(
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    size_t line) {
  insertFn(fnMap_, name, paramTypes, returnType, line);
}

void Ctx::insertFn(
    unordered_multimap<string, Ctx::FnInfo>& funcMap,
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    size_t line) {
  auto iterPair = funcMap.equal_range(name);
  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const FnInfo& fnInfo = iter->second;
    const vector<TypePtr>& fnParamTypes = fnInfo.paramTypes;
    if (equal(paramTypes.cbegin(), paramTypes.cend(), fnParamTypes.cbegin(), fnParamTypes.cend())) {
      ostream& errStream = logger.logError(line);
      errStream << "Redefinition of function '" << name;
      Ctx::streamParamTypes(paramTypes, errStream);
      errStream << "'. Originally declared at " << fnInfo.declFile << ", line " << fnInfo.line;
      return;
    }
  }
  funcMap.emplace(name, FnInfo{ paramTypes, move(returnType), filename_, line });
}

Ctx::FnLookupInfo Ctx::lookupFn(const string& name, const vector<TypePtr>& paramTypes) {
  return lookupFn(fnMap_, name, paramTypes);
}

Ctx::FnLookupInfo Ctx::lookupFn(
    const std::unordered_multimap<std::string, FnInfo>& funcMap,
    const string& name,
    const vector<TypePtr>& paramTypes) {
  vector<const FnInfo*> wideMatches;
  vector<const FnInfo*> narrowMatches;
  auto iterPair = funcMap.equal_range(name);

  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const vector<TypePtr>& fnParamTypes = iter->second.paramTypes;
    bool isNarrowing = false;
    if (equal(paramTypes.cbegin(), paramTypes.cend(), fnParamTypes.cbegin(), fnParamTypes.cend())) {
      // Exact match
      return { FnLookupRes::FOUND, { vector<const FnInfo*>{ &iter->second } }, filename_ };
    } else if (equal(
                   paramTypes.cbegin(),
                   paramTypes.cend(),
                   fnParamTypes.cbegin(),
                   fnParamTypes.cend(),
                   [&isNarrowing](const TypePtr& from, const TypePtr& to) {
                     bool b;
                     bool res = isConvertible(*from, *to, &b);
                     isNarrowing |= b;
                     return res;
                   })) {
      // Implicit conversion match
      if (isNarrowing && wideMatches.empty()) {
        // No need to keep track of narrowing matches if we already found a
        // widening match
        narrowMatches.push_back(&iter->second);
      } else if (!isNarrowing) {
        wideMatches.push_back(&iter->second);
      }
    }
  }

  // Only try narrowing matches if there are no widening matches
  const vector<const FnInfo*>* matchVec = wideMatches.empty() ? &narrowMatches : &wideMatches;
  if (matchVec->size() > 1) {
    return { FnLookupRes::AMBIG_OVERLOAD, { move(*matchVec) }, filename_ };
  } else if (matchVec->size() == 1) {
    return { matchVec == &narrowMatches ? FnLookupRes::NARROWING : FnLookupRes::FOUND,
             { move(*matchVec) },
             filename_ };
  } else {
    // No matches
    vector<const FnInfo*> candidates;
    transform(iterPair.first, iterPair.second, back_inserter(candidates), [](const auto& p) {
      return &p.second;
    });
    return { FnLookupRes::UNDEFINED, move(candidates), filename_ };
  }
}

const Ctx::FnInfo* Ctx::lookupFnRec(
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  return lookupFnRec(fnMap_, qualifiers, name, paramTypes, line);
}

// TODO: Allow "from <file> import <function/class>" ???
const Ctx::FnInfo* Ctx::lookupFnRec(
    const std::unordered_multimap<std::string, FnInfo>& funcMap,
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  FnLookupInfo lookupInfo = qualifiers.empty() ? lookupFn(funcMap, name, paramTypes)
                                               : ctxTree_.lookupFn(qualifiers, name, paramTypes);

  switch (lookupInfo.res) {
    case FnLookupRes::NARROWING: {
      const FnInfo* fnInfo = get<0>(lookupInfo.candidates).front();
      // This function duplicates some work we already did, but it would take
      // more effort to keep track of the narrowing conversions as we search for
      // a feasible overload
      warnNarrow(paramTypes, fnInfo->paramTypes, line);
      return fnInfo;
    }
    case FnLookupRes::FOUND:
      return get<0>(lookupInfo.candidates).front();
    case FnLookupRes::AMBIG_OVERLOAD:
      ambigOverload(
          qualifiers, name, paramTypes, line, get<0>(lookupInfo.candidates), lookupInfo.filename);
      return nullptr;
    case FnLookupRes::UNDEFINED:
      undefinedFn(
          qualifiers, name, paramTypes, line, get<0>(lookupInfo.candidates), lookupInfo.filename);
      return nullptr;
    case FnLookupRes::BAD_QUALS:
      undefFnBadQuals(qualifiers, name, paramTypes, line);
      return nullptr;
    case FnLookupRes::AMBIG_QUALS:
      undefFnAmbigQuals(
          qualifiers, name, paramTypes, line, get<1>(lookupInfo.candidates), lookupInfo.filename);
      return nullptr;
    default:
      throw invalid_argument("Ctx::lookupFnRec");
  }
}


void Ctx::undefinedFn(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const FnInfo*>& candidates,
    string_view searchedFile) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName);
  streamParamTypes(paramTypes, err);

  if (candidates.empty()) {
    err << "'\nNo candidate functions in " << searchedFile;
  } else {
    err << "'\nCandidate functions in " << searchedFile << ":";
    for (const FnInfo* fnInfo : candidates) {
      err << "\n\tLine " << fnInfo->line << ": " << *fnInfo->returnType << ' ' << fnName;
      streamParamTypes(fnInfo->paramTypes, err);
    }
  }
}

void Ctx::undefFnBadQuals(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << qualifiedFn(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "'. No imported file matches qualifiers.";
}

void Ctx::undefFnAmbigQuals(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const string*> candidates,
    string_view searchedPath) {
  ostream& err = logger.logError(line);
  err << "Ambiguous qualifier for function '" << boost::join(qualifiers, "::") << "::" << fnName;
  streamParamTypes(paramTypes, err);
  err << "'. Found";
  for (const string* cand : candidates) {
    err << "\n\t" << *cand << "::" << searchedPath;
  }
}

void Ctx::ambigOverload(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const Ctx::FnInfo*>& candidates,
    string_view searchedFile) {
  ostream& err = logger.logError(line);
  err << "Call of overloaded function '" << qualifiedFn(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "' is ambiguous. Candidate functions in " << searchedFile << ":";
  for (const FnInfo* fnInfo : candidates) {
    err << "\n\tLine " << fnInfo->line << ": " << *fnInfo->returnType << ' ' << fnName;
    streamParamTypes(fnInfo->paramTypes, err);
  }
}

void Ctx::warnNarrow(
    const vector<TypePtr>& fromTypes,
    const vector<TypePtr>& toTypes,
    size_t line) {
  size_t len = fromTypes.size();
  for (size_t i = 0; i < len; ++i) {
    const Type& from = *fromTypes[i];
    const Type& to = *toTypes[i];
    if (from.numBytes > to.numBytes) {
      ostringstream& warn = logger.logWarning(line);
      warn << "Narrowing conversion from " << from << " to " << to;
    }
  }
}


string
Ctx::mangleFn(string_view fnName, const string& filename, const vector<TypePtr>& paramTypes) {
  if (!fnName.starts_with("__")) {
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

namespace {

string newFileId() {
  static size_t i = 0;
  return to_string(i++);
}

string newTypeId() {
  static size_t i = 0;
  return to_string(i++);
}

}  // namespace

void Ctx::addFileId(string_view filename) { fileIds_->emplace(filename, newFileId()); }
void Ctx::addTypeId(string_view typeName) { typeIds_->emplace(typeName, newTypeId()); }

void Ctx::typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = logger.logError(line);
  err << "Expected " << expected << ", got " << got;
}

void Ctx::displayLogs() const { logger.streamLog(); }

bool Ctx::hasErrors() const noexcept { return logger.hasErrors(); }
