#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <ostream>
#include <unordered_map>
#include <utility>
#include <vector>

class CtxTree;

class Ctx {
public:
  using CtxPtr = std::unique_ptr<Ctx>;

  struct VarInfo {
    TypePtr type;
    int temp;
    size_t line;
    bool used;
  };


  struct FnInfo {
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
    std::string declFile;
    size_t line;
  };

  /*
   * Name resolution happens as follows:
   * - If there is no qualifier, then only the current source file is searched
   * - If there is a qualifier:
   *    ~ If the qualifier is an exact match of an imported file path, that
   *      file is searched
   *    ~ Otherwise, if the qualifier represents a unique suffix identifying
   *      an imported file path, that file is searched
   *    ~ Otherwise, the name is undefined
   * - In all cases, the overloads are resolved as normal within the file
   * specified This approach allows one to easily identify which file a name is
   * from just by looking at the imports.
   */

  class CtxTree {
  public:
    struct Node {
      /* non-nullptr if file exists at this level in directory tree.
       * The lifetime of the Ctx is controlled by the shared_ptr in a Program */
      Ctx* ctx;
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
    bool addCtx(std::string_view importPath, Ctx* ctx);
    const Ctx::FnInfo* lookupFn(
        const std::vector<std::string> qualifiers,
        const std::string& name,
        const std::vector<TypePtr>& paramTypes,
        Ctx& ctx,
        size_t line) const;

  private:
    /* The roots specify .prez files that were imported
     * Each level down in the context tree represents a level up in the
     * directory tree */
    std::unordered_map<std::string, NodePtr> roots_;
  };

  static void streamParamTypes(
      const std::vector<TypePtr>& paramTypes,
      std::ostream& err);

  Ctx(std::string_view filename,
      std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds,
      std::shared_ptr<std::unordered_map<std::string, std::string>> typeIds);
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
  const VarInfo* lookupVar(const std::string& name, size_t line);
  const VarInfo* lookupTempVar(const std::string& name);
  void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
  void removeParams(const std::vector<std::string>& params, size_t line);
  void insertFn(
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      TypePtr returnType,
      size_t line);
  /* Also searches context tree, nullptr if it doesn't exist */
  const FnInfo* lookupFnRec(
      const std::vector<std::string>& qualifiers,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  /* Only searches this context, nullptr if it doesn't exist
   * Also returns the candidate functions with this name via
   * an iterator range */
  std::pair<
      const FnInfo*,
      std::pair<
          std::unordered_multimap<std::string, FnInfo>::iterator,
          std::unordered_multimap<std::string, FnInfo>::iterator>>
  lookupFn(const std::string& name, const std::vector<TypePtr>& paramTypes);
  void undefinedFn(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  void undefinedFn(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line,
      const std::pair<
          std::unordered_multimap<std::string, FnInfo>::iterator,
          std::unordered_multimap<std::string, FnInfo>::iterator>& candidates,
      std::string_view searchedFile);
  /* Mangle all user functions based on the filename (non-user functions begin
   * with '_') Return the function name if it doesn't need to be mangled */
  std::string mangleFn(
      std::string_view fnName,
      const std::string& filename,
      const std::vector<TypePtr>& paramTypes);
  void addFileId(size_t id, std::string_view filename);
  void typeError(const Type& expected, const Type& got, size_t line);
  void displayLogs() const;
  bool hasErrors() const noexcept;


private:
  void removeTemp(const std::string& var, size_t line);

  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_multimap<std::string, FnInfo> fnMap_;
  TypePtr currentRetType_;
  std::string filename_;
  Logger logger;
  CtxTree ctxTree_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> typeIds_;
};

#endif  // CONTEXT_HPP
