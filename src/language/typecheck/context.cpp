#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"
#include "src/language/utils.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;

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
    const shared_ptr<unordered_map<int, ClassInfo*>>& classIds)
    : filename_(filename), logger(filename), classIds_(classIds) {}

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
  auto iter = classMap_.find(name);
  if (iter == classMap_.end()) {
    throw runtime_error("Ctx::addClassId should be called before Ctx::insertClass");
  }

  iter->second.methods = move(methods);
  iter->second.fields = move(fields);
}

Ctx::ClsLookupRes Ctx::lookupClass(const string& name) {
  auto iter = classMap_.find(name);
  if (iter == classMap_.end()) {
    return { LookupStatus::UNDEFINED, nullptr, filename_ };
  }
  return { LookupStatus::FOUND, &iter->second, filename_ };
}

const Ctx::ClassInfo* Ctx::lookupClass(int id) {
  auto iter = classIds_->find(id);
  if (iter == classIds_->end()) {
    return nullptr;
  }
  return iter->second;
}

const Ctx::ClassInfo*
Ctx::lookupClassRec(const vector<string>& qualifiers, const string& name, size_t line) {
  LookupRes lookupRes =
      qualifiers.empty() ? lookupClass(name) : ctxTree_.lookupClass(qualifiers, name);

  return handleClassLookupRes(lookupRes, qualifiers, name, line);
}


const Ctx::FnInfo* Ctx::lookupMethod(
    int id,
    const string& className,
    const string& methodName,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  // The only way this will not be FOUND is if the object that was created was not a valid type.
  // If this is the case, the error was already logged, so we don't want to log an error for every
  // method invocation on this object. Thus, we don't use lookupClassRec.
  // TODO: Wrong if object is an rvalue, e.g., MyObject(1).doStuff();
  const ClassInfo* classInfo = lookupClass(id);
  if (!classInfo) {
    // If we hit this case, we already logged an undefined class error somewhere else, so don't
    // duplicate it
    return nullptr;
  }

  LookupRes methodLookup = lookupFn(classInfo->methods, methodName, paramTypes);
  methodLookup.searchedPath = classInfo->declFile;
  return handleFnLookupRes(
      methodLookup, {}, string(className).append("::").append(methodName), paramTypes, line);
}


const Ctx::ClassInfo* Ctx::handleClassLookupRes(
    const ClsLookupRes& lookupRes,
    const vector<string>& qualifiers,
    const string& name,
    size_t line) {
  switch (lookupRes.status) {
    case LookupStatus::FOUND:
      return get<const ClassInfo*>(lookupRes.result);
    case LookupStatus::UNDEFINED:
      undefinedClass(qualifiers, name, line, lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::BAD_QUALS:
      undefClassBadQuals(qualifiers, name, line);
      return nullptr;
    case LookupStatus::AMBIG_QUALS:
      undefClassAmbigQuals(
          qualifiers,
          name,
          line,
          get<vector<const string*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    default:
      throw invalid_argument("Ctx::lookupClassRec");
  }
}

void Ctx::insertFn(
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    size_t id,
    size_t line) {
  insertFn(fnMap_, name, paramTypes, returnType, id, line);
}

void Ctx::insertFn(
    unordered_multimap<string, Ctx::FnInfo>& funcMap,
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    size_t id,
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
  funcMap.emplace(name, FnInfo{ paramTypes, move(returnType), id, filename_, line });
}

Ctx::FnLookupRes Ctx::lookupFn(const string& name, const vector<TypePtr>& paramTypes) {
  return lookupFn(fnMap_, name, paramTypes);
}

Ctx::FnLookupRes Ctx::lookupFn(
    const unordered_multimap<string, FnInfo>& funcMap,
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
      return { LookupStatus::FOUND, vector<const FnInfo*>{ &iter->second }, filename_ };
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
    return { LookupStatus::AMBIG_OVERLOAD, move(*matchVec), filename_ };
  } else if (matchVec->size() == 1) {
    return { matchVec == &narrowMatches ? LookupStatus::NARROWING : LookupStatus::FOUND,
             { move(*matchVec) },
             filename_ };
  } else {
    // No matches
    vector<const FnInfo*> candidates;
    transform(iterPair.first, iterPair.second, back_inserter(candidates), [](const auto& p) {
      return &p.second;
    });
    return { LookupStatus::UNDEFINED, move(candidates), filename_ };
  }
}


// TODO: Allow "from <file> import <function/class>" ???
const Ctx::FnInfo* Ctx::lookupFnRec(
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  LookupRes lookupRes = qualifiers.empty() ? lookupFn(name, paramTypes)
                                           : ctxTree_.lookupFn(qualifiers, name, paramTypes);
  return handleFnLookupRes(lookupRes, qualifiers, name, paramTypes, line);
}

const Ctx::FnInfo* Ctx::handleFnLookupRes(
    const FnLookupRes& lookupRes,
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) {
  switch (lookupRes.status) {
    case LookupStatus::NARROWING: {
      const FnInfo* fnInfo = get<vector<const FnInfo*>>(lookupRes.result).front();
      // This function duplicates some work we already did, but it would take
      // more effort to keep track of the narrowing conversions as we search for
      // a feasible overload
      warnNarrow(paramTypes, fnInfo->paramTypes, line);
      return fnInfo;
    }
    case LookupStatus::FOUND:
      return get<vector<const FnInfo*>>(lookupRes.result).front();
    case LookupStatus::AMBIG_OVERLOAD:
      ambigOverload(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const FnInfo*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::UNDEFINED:
      undefinedFn(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const FnInfo*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::BAD_QUALS:
      undefFnBadQuals(qualifiers, name, paramTypes, line);
      return nullptr;
    case LookupStatus::AMBIG_QUALS:
      undefFnAmbigQuals(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const string*>>(lookupRes.result),
          lookupRes.searchedPath);
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
  err << "Undefined function '" << qualifiedName(qualifiers, fnName);
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
  err << "Undefined function '" << qualifiedName(qualifiers, fnName);
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
  err << "Ambiguous qualifier for function '" << qualifiedName(qualifiers, fnName);
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
  err << "Call of overloaded function '" << qualifiedName(qualifiers, fnName);
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


void Ctx::undefinedClass(
    const vector<string>& qualifiers,
    string_view className,
    size_t line,
    string_view searchedFile) {
  ostream& err = logger.logError(line);
  err << "Class '" << qualifiedName(qualifiers, className) << "' is not defined in "
      << searchedFile;
}

void Ctx::undefClassBadQuals(const vector<string>& qualifiers, string_view className, size_t line) {
  ostream& err = logger.logError(line);
  err << "Undefined class '" << qualifiedName(qualifiers, className)
      << "'. No imported file matches qualifiers.";
}

void Ctx::undefClassAmbigQuals(
    const vector<string>& qualifiers,
    string_view className,
    size_t line,
    const vector<const string*> candidates,
    string_view searchedPath) {
  ostream& err = logger.logError(line);
  err << "Ambiguous qualifier for function '" << qualifiedName(qualifiers, className) << "'. Found";
  for (const string* cand : candidates) {
    err << "\n\t" << *cand << "::" << searchedPath;
  }
}

void Ctx::checkType(Type& type, size_t line) {
  // Built-in types are always valid
  if (type.typeName != TypeName::CLASS) {
    return;
  }

  // Already checked this one
  Class& classTy = static_cast<Class&>(type);
  if (classTy.id != Class::ID_EMPTY) {
    return;
  }

  const ClassInfo* classInfo = lookupClassRec(classTy.qualifiers, classTy.className, line);
  if (!classInfo) {
    // Prevent doing the check again
    classTy.id = Class::ID_UNKNOWN;
  } else {
    classTy.id = classInfo->id;
  }
}


string Ctx::mangleFn(string_view fnName, size_t id) {
  if (fnName.starts_with("__") || fnName == "main") {
    return string(fnName);
  }

  return string(fnName).append(1, '_').append(to_string(id));
}

void Ctx::addClassId(string_view className, int id, size_t line) {
  auto p = classMap_.emplace(className, ClassInfo{ {}, {}, filename_, id });
  if (p.second) {
    classIds_->emplace(id, &p.first->second);
  } else {
    ostringstream& err = logger.logError(line);
    err << "Redefinition of class " << className;
  }
}

void Ctx::typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = logger.logError(line);
  err << "Expected " << expected << ", got " << got;
}

void Ctx::displayLogs() const { logger.streamLog(); }

bool Ctx::hasErrors() const noexcept { return logger.hasErrors(); }
