#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/ctx_tree.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <utility>
#include <vector>

class CtxTree;

class Ctx {
public:
  struct VarInfo {
    TypePtr type;
    int temp;
    size_t line;
    bool used;
  };

  struct FnInfo {
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
    // TODO: Shouldn't need this field anymore
    std::string declFile;
    size_t line;
  };

  Ctx(string_view fileName);
  ~Ctx() = default;
  Ctx(const Ctx&) = delete;
  Ctx(Ctx&&) = default;
  Ctx& operator=(const Ctx&) = delete;
  Ctx& operator=(Ctx&&) = default;

  Logger& getLogger() noexcept;
  CtxTree& getCtxTree() noexcept;
  const std::string& getCurrentFn() const noexcept;
  void setCurrentFn(std::string_view fnName);

  int insertVar(std::string_view name, TypePtr type, size_t line);
  const VarInfo& lookupVar(const std::string& name, size_t line);
  void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
  void removeParams(const std::vector<std::string>& params, size_t line);
  void insertFn(
      std::string_view name,
      const std::vector<TypePtr>& paramTypes,
      TypePtr returnType,
      size_t line);
  /* Only searches this context, nullptr if it doesn't exist */
  const FnInfo* lookupFn(const std::string& name);
  /* Also searches context tree */
  const FnInfo& lookupFnRec(
      const std::vector<std::string> qualifiers,
      const std::string& name,
      size_t line);
  void undefinedFn(
      const std::vector<std::string> qualifiers,
      const std::string& fnName,
      size_t line);
  /* Returns true if there was an error */
  bool displayLogs() const;

private:
  void removeTemp(const std::string& var, size_t line);

  std::unordered_map<std::string, VarInfo> varMap;
  // TODO: Remove initialization when we add a print() to the language
  std::unordered_map<std::string, FnInfo> fnMap = {
    { "printInt", FnInfo{ vector<TypePtr>{ intType }, voidType, "", 0 } }
  };
  std::string currentFn;
  std::string fileName_;
  Logger logger;
  CtxTree ctxTree_;
};

#endif  // CONTEXT_HPP
