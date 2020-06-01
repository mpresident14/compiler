#include "src/language/typecheck/context.hpp"

#include <boost/algorithm/string/split.hpp>

using namespace std;

namespace {

vector<string> splitPath(string_view importPath) {
  importPath = importPath.substr(0, importPath.size() - sizeof(".prez") + 1);
  vector<string> pathParts;
  boost::split(pathParts, importPath, [](char c) { return c == '/'; });
  return pathParts;
}

}  // namespace

bool Ctx::CtxTree::addCtx(string_view importPath, CtxPtr ctx) {
  vector<string> pathParts = splitPath(importPath);
  unordered_map<string, NodePtr>* currentMap = &roots_;

  for (auto revIter = pathParts.crbegin(); revIter != pathParts.crend();
       ++revIter) {
    const string& part = *revIter;
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      // Path part needs to be created
      CtxPtr newCtx = nullptr;
      // Last part of path, put the context here
      if (revIter == prev(pathParts.crend())) {
        newCtx = move(ctx);
      }
      Node* newNode = new Node{ newCtx, {} };
      auto insertIter = currentMap->emplace(part, NodePtr(newNode)).first;
      currentMap = &insertIter->second->children;
    } else {
      // Path part already exists
      NodePtr& child = iter->second;
      if (revIter == prev(pathParts.crend())) {
        bool doesNotExist = child->ctx == nullptr;
        // Last part of path, put the context here
        child->ctx = move(ctx);
        return doesNotExist;
      }
      currentMap = &child->children;
    }
  }
  return true;
}


const Ctx::FnInfo& Ctx::CtxTree::lookupFn(
    const vector<string> qualifiers,
    const string& name,
    size_t line,
    Ctx& logCtx) const {
  const unordered_map<string, NodePtr>* currentMap = &roots_;

  for (auto revIter = qualifiers.crbegin(); revIter != qualifiers.crend();
       ++revIter) {
    const string& part = *revIter;
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      // Throws
      logCtx.undefinedFn(qualifiers, name, line);
    }

    const Node* child = iter->second.get();
    if (revIter == prev(qualifiers.crend())) {
      // We've reached the end of the path used to qualify the function, so
      // start looking for a context
      do {
        // If this context is null, continue searching down the tree until
        // we find either:
        // - a non-null context (resolves to shortest path)
        // - a Node with not exactly 1 child (can't resolve qualifiers)
        const CtxPtr& maybeCtx = child->ctx;
        if (maybeCtx) {
          const Ctx::FnInfo* fnInfo = maybeCtx->lookupFn(name);
          if (fnInfo) {
            return *fnInfo;
          }
          // TODO: I think we should allow resolution to keep searching if the
          // function wasn't found in this context (e.g., if I have file.prez
          // and folder/file.prez and I call file::func(), but func() is only
          // in folder/file.prez, this should still resolve)
          // Throws
          logCtx.undefinedFn(qualifiers, name, line);
        }

        currentMap = &child->children;
        if (currentMap->size() != 1) {
          // Throws
          logCtx.undefinedFn(qualifiers, name, line);
        }

        child = currentMap->cbegin()->second.get();
      } while (true);
    } else {
      currentMap = &child->children;
    }
  }
  throw runtime_error("CtxTree::lookupFn");
}
