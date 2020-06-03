#include "src/language/typecheck/context.hpp"

#include <list>
#include <stdexcept>

#include <boost/algorithm/string/join.hpp>
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

  // Iterate backwards through the parts of the qualifier path until we reach
  // the first one
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

  // We've reached the first part of the qualifier path, so insert the context
  // if it doesn't already exist (duplicate imports ok, we log a note)
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
    const std::vector<TypePtr>& paramTypes,
    Ctx& ctx,
    size_t line) const {
  list<string> filepath;
  // TODO: Remove when done
  if (qualifiers.empty()) {
    throw runtime_error("Ctx::CtxTree::lookupFn");
  }

  const unordered_map<string, NodePtr>* currentMap = &roots_;
  const Node* child;
  for (auto revIter = qualifiers.crbegin(); revIter != qualifiers.crend();
       ++revIter) {
    const string& part = *revIter;
    filepath.push_front(part);
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      ctx.undefinedFn(qualifiers, name, paramTypes, line);
      return nullptr;
    }

    child = iter->second.get();
    currentMap = &child->children;
  }

  // We've reached the end of the path used to qualify the function, so
  // start looking for a context
  do {
    // If this context is null (not an exact match), continue searching down the
    // tree until we find either:
    // - a Node with a non-null context (unique suffix)
    // - a Node with more than 1 child (ambiguous qualifier)
    const CtxPtr& maybeCtx = child->ctx;
    if (maybeCtx) {
      auto infoAndIters = maybeCtx->lookupFn(name, paramTypes);
      if (infoAndIters.first) {
        return infoAndIters.first;
      } else {
        ctx.undefinedFn(
            qualifiers,
            name,
            paramTypes,
            line,
            infoAndIters.second,
            boost::join(filepath, "/").append(".prez"));
        return nullptr;
      }
    }

    currentMap = &child->children;
    if (currentMap->empty()) {
      throw runtime_error("Ctx::CtxTree::lookupFn (empty)");
    }

    if (currentMap->size() > 1) {
      ostream& err = ctx.getLogger().logError(line);
      err << "Ambiguous qualifier for function '"
          << boost::join(qualifiers, "::") << "::" << name;
      Ctx::streamParamTypes(paramTypes, err);
      err << "'. Found";
      for (const auto& [part, _] : *currentMap) {
        err << "\n\t" << part << "::" << boost::join(filepath, "::");
      }
      return nullptr;
    }

    const auto iter = currentMap->cbegin();
    const string& part = iter->first;
    filepath.push_front(part);
    child = iter->second.get();
  } while (true);
}
