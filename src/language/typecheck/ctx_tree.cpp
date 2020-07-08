#include "src/language/typecheck/context.hpp"

#include <list>
#include <stdexcept>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include "src/language/utils.hpp"

using namespace std;

// TODO: Handle absolute paths: How do we distinguish "import /home/file1" and "import home/file1"?

bool Ctx::CtxTree::addCtx(string_view importPath, Ctx* ctx) {
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
  const string& firstPart = pathParts[0];
  auto mapIter = currentMap->find(firstPart);
  if (mapIter == currentMap->end()) {
    Node* newNode = new Node{ ctx, {} };
    currentMap->emplace(firstPart, NodePtr(newNode));
    return true;
  } else {
    NodePtr& child = mapIter->second;
    bool doesNotExist = child->ctx == nullptr;
    child->ctx = ctx;
    return doesNotExist;
  }
}


Ctx::FnLookupInfo Ctx::CtxTree::lookupFn(
    const vector<string> qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes) const {
  list<string> filepath;
  if (qualifiers.empty()) {
    throw runtime_error("Ctx::CtxTree::lookupFn");
  }

  const unordered_map<string, NodePtr>* currentMap = &roots_;
  const Node* child;
  for (auto revIter = qualifiers.crbegin(); revIter != qualifiers.crend(); ++revIter) {
    const string& part = *revIter;
    filepath.push_front(part);
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      // Qualifiers point to file that was not imported
      return { FnLookupRes::BAD_QUALS, {}, "" };
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
    Ctx* maybeCtx = child->ctx;
    if (maybeCtx) {
      FnLookupInfo lookupInfo = maybeCtx->lookupFn(name, paramTypes);
      if (lookupInfo.res != FnLookupRes::FOUND) {
        lookupInfo.filename = boost::join(filepath, "/").append(".prez");
      }
      return lookupInfo;
    }

    currentMap = &child->children;
    if (currentMap->empty()) {
      throw runtime_error("Ctx::CtxTree::lookupFn (empty)");
    }

    if (currentMap->size() > 1) {
      // Qualifiers are ambiguous based on imports
      vector<const string*> candidates;
      for (const auto& [part, _] : *currentMap) {
        candidates.push_back(&part);
      }
      return { FnLookupRes::AMBIG_QUALS, move(candidates), boost::join(filepath, "::") };
    }

    const auto iter = currentMap->cbegin();
    const string& part = iter->first;
    filepath.push_front(part);
    child = iter->second.get();
  } while (true);
}
