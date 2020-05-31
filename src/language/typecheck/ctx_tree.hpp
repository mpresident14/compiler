#ifndef CTX_TREE_HPP
#define CTX_TREE_HPP

#include "src/language/typecheck/context.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>


class CtxTree {
public:
  struct Node {
    virtual ~Node() = 0;
    bool isLeaf;
  };

  struct InternalNode : Node {
    std::string pathPart;
    std::unordered_set<std::unique_ptr<Node>> children;
  };

  struct LeafNode : Node {
    Ctx* ctx;
  };

  CtxTree();
  ~CtxTree() = default;
  CtxTree(const CtxTree&) = delete;
  CtxTree(CtxTree&&) = default;
  CtxTree& operator=(const CtxTree&) = delete;
  CtxTree& operator=(CtxTree&&) = default;

  /* Return true if successfully added, false if already exists */
  bool addCtx(std::string import, CtxPtr ctx);
  const CtxPtr& getCtx(std::string import) const;

private:
  std::unique_ptr<InternalNode> root_;
};

CtxTree::Node::~Node(){}

#endif  // CTX_TREE_HPP
