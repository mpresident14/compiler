#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <ostream>
#include <unordered_map>
#include <utility>
#include <variant>
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

  enum class FnLookupRes { FOUND, UNDEFINED, AMBIG_OVERLOAD, NARROWING, BAD_QUALS, AMBIG_QUALS };

  struct FnLookupInfo {
    FnLookupRes res;
    std::variant<std::vector<const FnInfo*>, std::vector<const std::string*>> candidates;
    std::string filename;
  };

  struct FieldInfo {
    TypePtr type;
    size_t offset;
  };

  struct ClassInfo {
    std::unordered_multimap<std::string, FnInfo> methods;
    std::unordered_map<std::string, FieldInfo> fields;
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
    FnLookupInfo lookupFn(
        const std::vector<std::string> qualifiers,
        const std::string& name,
        const std::vector<TypePtr>& paramTypes) const;

  private:
    /* The roots specify .prez files that were imported
     * Each level down in the context tree represents a level up in the
     * directory tree */
    std::unordered_map<std::string, NodePtr> roots_;
  };

  static void streamParamTypes(const std::vector<TypePtr>& paramTypes, std::ostream& err);

  Ctx(std::string_view filename,
      const std::shared_ptr<std::unordered_map<std::string, std::string>>& fileIds,
      const std::shared_ptr<std::unordered_map<std::string, std::string>>& typeIds);
  ~Ctx() = default;
  Ctx(const Ctx&) = delete;
  Ctx(Ctx&&) = default;
  Ctx& operator=(const Ctx&) = delete;
  Ctx& operator=(Ctx&&) = default;

  Logger& getLogger() noexcept;
  CtxTree& getCtxTree() noexcept;
  const std::string& getFilename() const noexcept;
  const Type& getCurrentRetType() const noexcept;
  // TODO: Here and everywhere applicable: Should take in const TypePtr& and no move
  void setCurrentRetType(TypePtr type) noexcept;

  int insertVar(std::string_view name, TypePtr type, size_t line);
  const VarInfo* lookupVar(const std::string& name, size_t line);
  const VarInfo* lookupTempVar(const std::string& name);
  void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
  void removeParams(const std::vector<std::string>& params, size_t line);
  void insertClass(
      const std::string& name,
      std::unordered_multimap<std::string, Ctx::FnInfo>&& methods,
      std::unordered_map<std::string, Ctx::FieldInfo>&& fields);
  /* Only searches this context, nullptr if it doesn't exist */
  const ClassInfo* lookupClass(const std::string& name);
  /* Also searches context tree, nullptr if it doesn't exist */
  const ClassInfo* lookupClassRec(const std::vector<std::string>& qualifiers, const std::string& name);
  void insertFn(
      std::unordered_multimap<std::string, FnInfo>& funcMap,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      const TypePtr& returnType,
      size_t line);
  void insertFn(
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      const TypePtr& returnType,
      size_t line);
  /* Only searches this context, empty candidate vector if it doesn't exist */
  FnLookupInfo lookupFn(const std::string& name, const std::vector<TypePtr>& paramTypes);
  /* Also searches context tree, nullptr if it doesn't exist */
  const FnInfo* lookupFnRec(
      const std::vector<std::string>& qualifiers,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  void undefinedFn(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line,
      const std::vector<const FnInfo*>& candidates,
      std::string_view searchedFile);
  void undefFnBadQuals(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  void undefFnAmbigQuals(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line,
      const std::vector<const std::string*> candidates,
      std::string_view searchedPath);
  void ambigOverload(
      const std::vector<std::string>& qualifiers,
      std::string_view fnName,
      const std::vector<TypePtr>& paramTypes,
      size_t line,
      const std::vector<const FnInfo*>& candidates,
      std::string_view searchedFile);
  void warnNarrow(
      const std::vector<TypePtr>& fromTypes,
      const std::vector<TypePtr>& toTypes,
      size_t line);
  /* Mangle all user functions based on the filename (My special functions begin
   * with "__") Return the function name if it doesn't need to be mangled */
  std::string mangleFn(
      std::string_view fnName,
      const std::string& filename,
      const std::vector<TypePtr>& paramTypes);
  void addFileId(std::string_view filename);
  void addTypeId(std::string_view typeName);
  void typeError(const Type& expected, const Type& got, size_t line);
  void displayLogs() const;
  bool hasErrors() const noexcept;


private:
  void removeTemp(const std::string& var, size_t line);

  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_multimap<std::string, FnInfo> fnMap_;
  std::unordered_multimap<std::string, ClassInfo> classMap_;
  TypePtr currentRetType_;
  std::string filename_;
  Logger logger;
  CtxTree ctxTree_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> fileIds_;
  std::shared_ptr<std::unordered_map<std::string, std::string>> typeIds_;
};

#endif  // CONTEXT_HPP
