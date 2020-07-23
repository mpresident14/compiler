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
    bool isVirtual;
    size_t id;
    std::string declFile;
    size_t line;
  };

  struct FieldInfo {
    TypePtr type;
    size_t offset;
  };

  struct ClassInfo {
    std::unordered_multimap<std::string, FnInfo> methods;
    std::unordered_map<std::string, FieldInfo> fields;
    std::unordered_map<size_t, size_t> vTableOffsets;
    std::string declFile;
    int id;
  };

  enum class LookupStatus { FOUND, UNDEFINED, AMBIG_OVERLOAD, NARROWING, BAD_QUALS, AMBIG_QUALS };


  template <typename T>
  struct LookupRes {
    LookupStatus status;
    /* Either the result or candidates for ambiguous qualifiers */
    std::variant<T, std::vector<const std::string*>> result;
    /* Searched file or prefix for ambiguous qualifiers */
    std::string searchedPath;
  };

  using FnLookupRes = LookupRes<std::vector<const FnInfo*>>;
  using ClsLookupRes = LookupRes<const ClassInfo*>;
  using CtxLookupRes = LookupRes<Ctx*>;

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
    FnLookupRes lookupFn(
        const std::vector<std::string> qualifiers,
        const std::string& name,
        const std::vector<TypePtr>& paramTypes) const;
    ClsLookupRes lookupClass(const std::vector<std::string> qualifiers, const std::string& name)
        const;

  private:
    CtxLookupRes lookupCtx(const std::vector<std::string> qualifiers) const;
    /* The roots specify .prez files that were imported
     * Each level down in the context tree represents a level up in the
     * directory tree */
    std::unordered_map<std::string, NodePtr> roots_;
  };

  static void streamParamTypes(const std::vector<TypePtr>& paramTypes, std::ostream& err);
  /* Mangle all user functions based on the function ID (My special functions begin
   * with "__") Return the function name if it doesn't need to be mangled */
  static std::string mangleFn(std::string_view fnName, size_t id);

  Ctx(std::string_view filename,
      const std::shared_ptr<std::unordered_map<int, ClassInfo*>>& classIds);
  ~Ctx() = default;
  Ctx(const Ctx&) = delete;
  Ctx(Ctx&&) = default;
  Ctx& operator=(const Ctx&) = delete;
  Ctx& operator=(Ctx&&) = default;

  Logger& getLogger() noexcept;
  CtxTree& getCtxTree() noexcept;
  const Type& getCurrentRetType() const noexcept;
  void setCurrentRetType(const TypePtr& type) noexcept;

  void includeDecls(Ctx& ctx);
  int insertVar(std::string_view name, const TypePtr& type, size_t line);
  const VarInfo* lookupVar(const std::string& name, size_t line);
  void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
  void removeParams(const std::vector<std::string>& params, size_t line);
  Ctx::ClassInfo& insertClass(const std::string& name, int id, size_t line);
  /* Only searches this context */
  ClsLookupRes lookupClass(const std::string& name);
  /* Searches global classIds_ */
  const ClassInfo* lookupClass(int id);
  /* Also searches context tree, nullptr if it doesn't exist */
  const ClassInfo*
  lookupClassRec(const std::vector<std::string>& qualifiers, const std::string& name, size_t line);
  std::pair<const Ctx::ClassInfo*, const Ctx::FnInfo*> lookupMethod(
      int id,
      const std::string& className,
      const std::string& methodName,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  void insertFn(
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      const TypePtr& returnType,
      size_t id,
      size_t line);
  void insertMethod(
      std::unordered_multimap<std::string, FnInfo>& funcMap,
      std::string_view className,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      const TypePtr& returnType,
      bool isVirtual,
      size_t id,
      size_t line);
  /* Only searches this context */
  FnLookupRes lookupFn(const std::string& name, const std::vector<TypePtr>& paramTypes);
  /* Also searches context tree, nullptr if it doesn't exist */
  const FnInfo* lookupFnRec(
      const std::vector<std::string>& qualifiers,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes,
      size_t line);
  /* Checks if this is a valid type. If the type is a Class, then it sets its id if it is not yet
   * set */
  void checkType(Type& type, size_t line);
  void typeError(const Type& expected, const Type& got, size_t line);
  void displayLogs() const;
  bool hasErrors() const noexcept;


private:
  void removeTemp(const std::string& var, size_t line);
  FnLookupRes lookupFn(
      const std::unordered_multimap<std::string, FnInfo>& funcMap,
      const std::string& name,
      const std::vector<TypePtr>& paramTypes);
  const FnInfo* handleFnLookupRes(
      const FnLookupRes& lookupRes,
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
  const ClassInfo* handleClassLookupRes(
      const ClsLookupRes& lookupRes,
      const std::vector<std::string>& qualifiers,
      const std::string& name,
      size_t line);
  void undefinedClass(
      const std::vector<std::string>& qualifiers,
      std::string_view className,
      size_t line,
      std::string_view searchedFile);
  void undefClassBadQuals(
      const std::vector<std::string>& qualifiers,
      std::string_view className,
      size_t line);
  void undefClassAmbigQuals(
      const std::vector<std::string>& qualifiers,
      std::string_view className,
      size_t line,
      const std::vector<const std::string*> candidates,
      std::string_view searchedPath);

  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_multimap<std::string, FnInfo> fnMap_;
  std::unordered_map<std::string, ClassInfo> classMap_;
  TypePtr currentRetType_;
  std::string filename_;
  Logger logger;
  CtxTree ctxTree_;
  /* Class IDs Explanation
   * 1. First, we go through all the class declarations and add them to the context of their own
   *    file, which gives each class a unique ID. It also adds the ID to the global classIds_ map.
   * 2. When a class name appears in a src file, it is parsed as a Class Type, but it has no id.
   *    Every language structure that contains a Type calls Ctx::checkType(), which, if it is a
   *    Class Type, validates that it refers to an actual class and determines its ID via the
   *    CtxTree.
   * 3. Finally, when we do a method invocation or member access, we get a Class Type from the
   *    object, from which we can grab the class ID and perform the necessary lookup via the global
   *    classIds_ map. Using the ID for this lookup rather than the class name prevents a situation
   *    such as the following:
   *
   *    <file_obj.prez>
   *    class FileObj;
   *
   *    <file.prez>
   *    import "file_obj.prez"
   *    file_obj::FileObj getFile();
   *
   *    <main.prez>
   *    import "file.prez"
   *    file::getFile().method();
   *
   *    If we just used the name of the returned Type (FileObj), main.prez would not have the
   *    correct import. Even if it did, the qualifiers might be incorrect because they would be
   *    based on those of file.prez.
   */
  std::shared_ptr<std::unordered_map<int, ClassInfo*>> classIds_;
};

#endif  // CONTEXT_HPP
