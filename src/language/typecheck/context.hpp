#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <unordered_map>
#include <vector>


class Context {
public:
  struct VarInfo {
    Type type;
    int temp;
  };

  struct FnInfo {
    std::vector<Type> paramTypes;
    Type returnType;
  };

  static Context& getContext() {
    static Context ctx;
    return ctx;
  }

  ~Context() = default;
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  const std::unordered_map<std::string, VarInfo>& getVarMap() const noexcept {
    return varMap_;
  }

  const std::unordered_map<std::string, FnInfo>& getFnMap() const noexcept {
    return fnMap_;
  }

  void setCurrentFn(const std::string& fnName) { currentFn_ = fnName; }
  void setReturnTy(const Type& type) { returnType_ = type; }
  const std::string& getCurrentFn() const noexcept { return currentFn_; }
  const Type& getReturnTy() const noexcept { return returnType_; }
  void addLogger(const std::string& srcFileName) { loggers_.emplace_back(srcFileName); }
  Logger& currentLogger() noexcept { return loggers_.back(); }

  int insertVar(const std::string& name, const Type& type);
  void insertParam(const std::string& name, const Type& type, MachineReg reg);
  const VarInfo& lookupVar(const std::string& name) const;
  void removeVar(const std::string& name);

  void insertFn(
      const std::string& name,
      std::vector<Type>&& paramTypes,
      const Type& returnType);
  const FnInfo& lookupFn(const std::string& name) const;

  /* Throws if any errors were encountered */
  void displayLog() const;

private:
  Context();

  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_map<std::string, FnInfo> fnMap_;
  std::string currentFn_;
  Type returnType_;
  /* Separate Logger for each source file */
  std::vector<Logger> loggers_;
};

#endif  // CONTEXT_HPP
