#ifndef DFA_HPP
#define DFA_HPP

#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

template <typename V, typename T>
class DFA {
public:
  struct Node {
  public:
    friend class DFA;

    bool operator==(const Node& other) const { return value_ == other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
      return out << node.value_;
    }

    const V& getValue() const { return value_; }
    const std::unordered_map<T, const Node*>& getTransitions() const { return transitions_; }

  private:
    Node(V value) : value_(std::move(value)) {}

    V value_;
    // "mutable" allows us to modify the map even when a node is const
    mutable std::unordered_map<T, const Node*> transitions_;
    bool deleterCalled_ = false;
  };

  friend struct Node;

  DFA(V value) : root_(new Node(value)), valueToNode_{{std::move(value), root_}} {}

  ~DFA() {
    // Delete via BFS
    std::queue<const Node*> q;
    q.push(root_);
    std::unordered_set<const Node*> visited = {root_};
    while (!q.empty()) {
      const Node* node = q.front();
      q.pop();
      for (auto& keyVal : node->transitions_) {
        const Node* successor = keyVal.second;
        if (!visited.contains(successor)) {
          q.push(successor);
          visited.insert(successor);
        }
      }
      delete node;
    }
  };

  DFA(const DFA& other) = delete;
  DFA(DFA&& other) = default;
  DFA& operator=(const DFA& other) = delete;
  DFA& operator=(DFA&& other) = default;

  const Node* getRoot() const { return root_; }

  const Node* run(const std::vector<T>& input) const {
    const Node* currentNode = root_;
    for (const T& inputToken : input) {
      currentNode = step(currentNode, inputToken);
      if (currentNode == nullptr) {
        return nullptr;
      }
    }
    return currentNode;
  }

  /* Step from node with transition inputToken */
  const Node* step(const Node* node, const T& inputToken) const {
    auto iter = node->transitions_.find(inputToken);
    if (iter == node->transitions_.end()) {
      return nullptr;
    }
    return iter->second;
  }

  // TODO: Make newNodeValue a universal reference so that we can forward it
  const Node* addTransition(const Node* node, T transition, V newNodeValue) {
    // No duplicate or updated transitions
    if (node->transitions_.contains(transition)) {
      return nullptr;
    }

    // If a node with this value already exists, just add a transition to the
    // existing node
    if (valueToNode_.contains(newNodeValue)) {
      const Node* successor = valueToNode_.at(newNodeValue);
      node->transitions_.emplace(std::move(transition), successor);
      return nullptr;
    }

    // Otherwise, create a new node
    // TODO: If we make valueToNode_ hold V*s and insert the address of newNode.value,
    // we can avoid expensive copies. Would have to define equality and hash of V* to be
    // that of V
    const Node* newNode = new Node(newNodeValue);
    node->transitions_.emplace(std::move(transition), newNode);
    valueToNode_.emplace(newNodeValue, newNode);
    return newNode;
  }

  friend std::ostream& operator<<(std::ostream& out, const DFA& dfa) {
    std::unordered_set<const Node*> visited;
    return dfa.doStream(out, dfa.getRoot(), 0, visited);
  }

private:
  std::ostream& doStream(std::ostream& out,
      const Node* node,
      size_t depth,
      std::unordered_set<const Node*>& visited) const {
    out << std::string(depth, '\t') << *node << '\n';
    visited.insert(node);
    for (auto& transSuccs : node->transitions_) {
      if (!visited.contains(transSuccs.second)) {
        doStream(out, transSuccs.second, depth + 1, visited);
      }
    }
    return out;
  }

  const Node* root_;
  // Allows us to check whether a node with some value exists in the
  // DFA and grab a pointer to it.
  std::unordered_map<V, const Node*> valueToNode_;
};

#endif
