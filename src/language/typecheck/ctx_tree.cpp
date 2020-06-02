#include "src/language/typecheck/context.hpp"

#include <stdexcept>

#include <boost/algorithm/string/split.hpp>

using namespace std;

namespace {

vector<string> splitPath(string_view importPath) {
  // TODO: Does this/our parser handle absolute paths?
  importPath = importPath.substr(0, importPath.size() - sizeof(".prez") + 1);
  vector<string> pathParts;
  boost::split(pathParts, importPath, [](char c) { return c == '/'; });
  return pathParts;
}

}  // namespace

bool Ctx::CtxTree::addCtx(string_view importPath, CtxPtr ctx) {
  vector<string> pathParts = splitPath(importPath);
  unordered_map<string, NodePtr>* currentMap = &roots_;

  // Iterate backwards through the parts of the path until we reach the first
  // one
  auto iterToFirst = prev(pathParts.crend());
  for (auto revIter = pathParts.crbegin(); revIter != iterToFirst; ++revIter) {
    const string& part = *revIter;
    auto mapIter = currentMap->find(part);
    if (mapIter == currentMap->end()) {
      // Path part needs to be created
      Node* newNode = new Node{ nullptr, {} };
      auto insertIter = currentMap->emplace(part, NodePtr(newNode)).first;
      currentMap = &insertIter->second->children;
    } else {
      // Path part already exists
      NodePtr& child = mapIter->second;
      currentMap = &child->children;
    }
  }

  // We've reached the first part of the path, so insert the context if
  // it doesn't already exist (duplicate imports ok)
  // TODO: enforce importPath not empty, either in grammar or elsewhere
  const string& firstPart = pathParts[0];
  auto mapIter = currentMap->find(firstPart);
  if (mapIter == currentMap->end()) {
    Node* newNode = new Node{ ctx, {} };
    currentMap->emplace(firstPart, NodePtr(newNode));
    return true;
  } else {
    NodePtr& child = mapIter->second;
    bool doesNotExist = child->ctx == nullptr;
    child->ctx = move(ctx);
    return doesNotExist;
  }
}


const Ctx::FnInfo* Ctx::CtxTree::lookupFn(
    const vector<string> qualifiers,
    const string& name,
    const std::vector<TypePtr>& paramTypes) const {
  // TODO: Remove when done
  if (qualifiers.empty()) {
    throw runtime_error("Ctx::CtxTree::lookupFn");
  }

  const unordered_map<string, NodePtr>* currentMap = &roots_;
  const Node* child;
  for (auto revIter = qualifiers.crbegin(); revIter != qualifiers.crend();
       ++revIter) {
    const string& part = *revIter;
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      return nullptr;
    }

    child = iter->second.get();
    currentMap = &child->children;
  }

  // We've reached the end of the path used to qualify the function, so
  // start looking for a context
  do {
    // If this context is null, continue searching down the tree until
    // we find either:
    // - a non-null context (resolves to shortest path)
    // - a Node with not exactly 1 child (can't resolve qualifiers)
    const CtxPtr& maybeCtx = child->ctx;
    if (maybeCtx) {
      return maybeCtx->lookupFn(name, paramTypes);
    }

    currentMap = &child->children;
    if (currentMap->size() != 1) {
      return nullptr;
    }

    child = currentMap->cbegin()->second.get();
  } while (true);
}
