#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

class CtxTree;

class Ctx {
public:
  using CtxPtr = std::shared_ptr<Ctx>;

  struct VarInfo {
    TypePtr type;
    int temp;
    size_t line;
    bool used;
  };


  struct FnInfo {
    // TODO: Shouldn't need this field anymore
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
    std::string declFile;
    size_t line;
  };


  class CtxTree {
  public:
    struct Node {
      /* non-nullptr if file exists at this level in directory tree */
      CtxPtr ctx;
      std::unordered_map<std::string, std::unique_ptr<Node>> children;
    };
    using NodePtr = std::unique_ptr<Node>;

    CtxTree() = default;
    ~CtxTree() = default;
    CtxTree(const CtxTree&) = delete;
    CtxTree(CtxTree&&) = default;
    CtxTree& operator=(const CtxTree&) = delete;
    CtxTree& operator=(CtxTree&&) = default;

    /* Return true if successfully added, false if already exists */
    bool addCtx(std::string_view importPath, CtxPtr ctx);
    const Ctx::FnInfo* lookupFn(
        const std::vector<std::string> qualifiers,
        const std::string& name) const;

  private:
    /* The roots specify .prez files that were imported
     * Each level down represents a level up in the directory */
    std::unordered_map<std::string, NodePtr> roots_;
  };

  Ctx(std::string_view filename,
      std::shared_ptr<std::unordered_map<std::string, std::string>> fnEncodings,
      std::shared_ptr<std::unordered_map<std::string, std::string>> typeEncodings);
  ~Ctx() = default;
  Ctx(const Ctx&) = delete;
  Ctx(Ctx&&) = default;
  Ctx& operator=(const Ctx&) = delete;
  Ctx& operator=(Ctx&&) = default;

  Logger& getLogger() noexcept;
  CtxTree& getCtxTree() noexcept;
  const std::string& getFilename() const noexcept;
  const Type& getCurrentRetType() const noexcept;
  void setCurrentRetType(TypePtr type) noexcept;

  int insertVar(std::string_view name, TypePtr type, size_t line);
  const VarInfo& lookupVar(const std::string& name, size_t line);
  void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
  void removeParams(const std::vector<std::string>& params, size_t line);
  void insertFn(
      std::string_view name,
      std::string_view mangledName,
      const std::vector<TypePtr>& paramTypes,
      TypePtr returnType,
      size_t line);
  /* Only searches this context, nullptr if it doesn't exist */
  const FnInfo* lookupFn(const std::string& mangledName);
  /* Also searches context tree */
  const FnInfo* lookupFnRec(
      const std::vector<std::string>& qualifiers,
      const std::string& mangledName);
  void undefinedFn(
      const std::vector<std::string>& qualifiers,
      const std::string& fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  /* Mangle all user functions based on the filename (non-user functions begin
   * with '_') Return empty if function doesn't need to be mangled */
  // TODO: remove the optional when we get rid of printInt and runprez
  std::optional<std::string> mangleWithParams(
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes);
  std::string mangleWithFile(std::string_view fnName, std::string_view filename);
  void typeError(const Type& expected, const Type& got, size_t line);
  void displayLogs() const;
  bool hasErrors() const noexcept;

private:
  void removeTemp(const std::string& var, size_t line);

  std::unordered_map<std::string, VarInfo> varMap;
  // TODO: Remove initialization when we add a print() to the language
  std::unordered_map<std::string, FnInfo> fnMap = {
    { "printInt", FnInfo{ std::vector<TypePtr>{ intType }, voidType, "", 0 } }
  };
  TypePtr currentRetType_;
  std::string filename_;
  Logger logger;
  CtxTree ctxTree_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> fnEncodings_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> typeEncodings_;
};

#endif  // CONTEXT_HPP
