#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"

#include <stdexcept>

using namespace std;

int Context::insertVar(const std::string& name, const Type& type) {
  int temp = newTemp();
  if (!varMap_.emplace(name, VarInfo{ move(type), temp }).second) {
    throw invalid_argument("Redefinition of variable  \"" + name + "\"");
  }
  return temp;
}

void Context::insertParam(
    const std::string& name,
    const Type& type,
    MachineReg reg) {
  if (!varMap_.emplace(name, VarInfo{ move(type), reg }).second) {
    throw invalid_argument("Redefinition of variable  \"" + name + "\"");
  }
}

const Context::VarInfo& Context::lookupVar(const std::string& name) const {
  auto iter = varMap_.find(name);
  if (iter == varMap_.end()) {
    throw invalid_argument("Undefined variable \"" + name + "\"");
  }
  return iter->second;
}

void Context::removeVar(const string& name) { varMap_.erase(name); }


void Context::insertFn(
    const std::string& name,
    std::vector<Type>&& paramTypes,
    const Type& returnType) {
  if (!fnMap_.emplace(name, FnInfo{ move(paramTypes), move(returnType) })
           .second) {
    throw invalid_argument("Redefinition of function  \"" + name + "\"");
  }
}

const Context::FnInfo& Context::lookupFn(const std::string& name) const {
  auto iter = fnMap_.find(name);
  if (iter == fnMap_.end()) {
    throw invalid_argument("Undefined function \"" + name + "\"");
  }
  return iter->second;
}


void Context::displayLog() const {
  bool hasError = false;
  for (const Logger& logger : loggers_) {
    try {
      logger.displayLog();
    } catch (std::exception& e) {
      cerr << e.what() << endl;
      hasError = true;
    }
  }

  if (hasError) {
    throw runtime_error("Context::displayLog");
  }
}

// TODO: Remove this when we add a print() to the language
Context::Context() {
  fnMap_.emplace(
      "printInt",
      FnInfo{ vector<Type>{ { TypeName::INT, "" } }, { TypeName::VOID, "" } });
}
