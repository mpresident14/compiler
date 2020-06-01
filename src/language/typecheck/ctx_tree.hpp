#ifndef CTX_TREE_HPP
#define CTX_TREE_HPP


#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class Ctx;
using CtxPtr = std::shared_ptr<Ctx>;

class CtxTree {
public:
  struct Node {
    /* non-nullptr if file exists at this level in directory tree */
    CtxPtr ctx;
    std::unordered_map<std::string, std::unique_ptr<Node>> children;
  };
  using NodePtr = std::unique_ptr<Node>;

  CtxTree();
  ~CtxTree() = default;
  CtxTree(const CtxTree&) = delete;
  CtxTree(CtxTree&&) = default;
  CtxTree& operator=(const CtxTree&) = delete;
  CtxTree& operator=(CtxTree&&) = default;

  /* Return true if successfully added, false if already exists */
  bool addCtx(std::string_view importPath, CtxPtr ctx);
  const Ctx::FnInfo& lookupFn(
      const std::vector<std::string> qualifiers,
      const std::string& name,
      size_t line,
      Ctx& ctx) const;

private:
  const Ctx::FnInfo& resolveFn(const unordered_map<string, NodePtr>* currentMap) const;

  /* The roots specify .prez files that were imported
   * Each level down represents a level up in the directory */
  std::unordered_map<std::string, NodePtr> roots_;
};

#endif  // CTX_TREE_HPP
