#include "src/language/ctx.hpp"
#include "src/language/utils.hpp"

#include <list>
#include <stdexcept>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

// TODO: Handle absolute paths: How do we distinguish "import /home/file1" and "import home/file1"?

namespace {

vector<string> splitPath(string_view importPath) {
  importPath = importPath.substr(0, importPath.size() - sizeof(".prez") + 1);
  vector<string> pathParts;
  boost::split(pathParts, importPath, [](char c) { return c == '/'; });
  return pathParts;
}

}  // namespace

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


Ctx::FnLookupRes Ctx::CtxTree::lookupFn(
    const vector<string> qualifiers, const string& name, const vector<TypePtr>& paramTypes) const {
  CtxLookupRes ctxLookup = lookupCtx(qualifiers);
  if (ctxLookup.status == LookupStatus::FOUND) {
    FnLookupRes fnLookup = get<Ctx*>(ctxLookup.result)->lookupFn(name, paramTypes);
    if (fnLookup.status != LookupStatus::FOUND) {
      fnLookup.searchedPath = ctxLookup.searchedPath;
    }
    return fnLookup;
  } else {
    if (holds_alternative<vector<const string*>>(ctxLookup.result)) {
      return { ctxLookup.status,
               move(get<vector<const string*>>(ctxLookup.result)),
               move(ctxLookup.searchedPath) };
    }
    return { ctxLookup.status, vector<const FnInfo*>(), move(ctxLookup.searchedPath) };
  }
}


Ctx::ClsLookupRes Ctx::CtxTree::lookupClass(
    const vector<string> qualifiers, const string& name) const {
  CtxLookupRes ctxLookup = lookupCtx(qualifiers);
  if (ctxLookup.status == LookupStatus::FOUND) {
    ClsLookupRes classLookup = get<Ctx*>(ctxLookup.result)->lookupClass(name);
    if (classLookup.status != LookupStatus::FOUND) {
      classLookup.searchedPath = ctxLookup.searchedPath;
    }
    return classLookup;
  } else {
    if (holds_alternative<vector<const string*>>(ctxLookup.result)) {
      return { ctxLookup.status,
               move(get<vector<const string*>>(ctxLookup.result)),
               move(ctxLookup.searchedPath) };
    }
    return { ctxLookup.status, nullptr, move(ctxLookup.searchedPath) };
  }
}


Ctx::CtxLookupRes Ctx::CtxTree::lookupCtx(const vector<string> qualifiers) const {
  list<string> filepath;
  if (qualifiers.empty()) {
    throw runtime_error("Ctx::CtxTree::lookupCtx");
  }

  const unordered_map<string, NodePtr>* currentMap = &roots_;
  const Node* child;
  for (auto revIter = qualifiers.crbegin(); revIter != qualifiers.crend(); ++revIter) {
    const string& part = *revIter;
    filepath.push_front(part);
    auto iter = currentMap->find(part);
    if (iter == currentMap->end()) {
      // Qualifiers point to file that was not imported
      return { LookupStatus::BAD_QUALS, nullptr, "" };
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
    if (child->ctx) {
      return { LookupStatus::FOUND, child->ctx, boost::join(filepath, "/").append(".prez") };
    }

    currentMap = &child->children;
    if (currentMap->empty()) {
      throw runtime_error("Ctx::CtxTree::lookupClass (empty)");
    }

    if (currentMap->size() > 1) {
      // Qualifiers are ambiguous based on imports
      vector<const string*> result;
      for (const auto& [part, _] : *currentMap) {
        result.push_back(&part);
      }
      return { LookupStatus::AMBIG_QUALS, move(result), boost::join(filepath, "::") };
    }

    const auto iter = currentMap->cbegin();
    const string& part = iter->first;
    filepath.push_front(part);
    child = iter->second.get();
  } while (true);
}
