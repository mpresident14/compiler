#ifndef NFA_HPP
#define NFA_HPP

#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

// template<typename T>
// concept Hashable = requires(T a) {
//     { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
// };

template <typename V, typename T>
class NFA {
public:

  struct Node {
  public:
    friend class NFA;

    Node(V value) : value_(std::move(value)) {}

    bool operator==(const Node& other) const { return value_ == other.value_; }
    bool operator!=(const Node& other) const { return value_ != other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
      out << prez::containerToString(node.value_);
      return out;
    }

    const V& getValue() const { return value_; }
    const std::unordered_map<T, const Node*>& getTransitions() const { return transitions_; }

  private:
    V value_;
    // Contains pointers to the elements in the set nodes_
    // "mutable" allows me to change the map even when a Node is marked const
    mutable std::unordered_map<T, const Node*> transitions_;
  };


  NFA(V value) : nodes_{ Node(value) } {
    root_ = &*nodes_.begin();
  }
  // TODO: Change root_ for these
  NFA(const NFA& other) = default;
  NFA(NFA&& other) = default;
  NFA& operator=(const NFA& other) = default;
  NFA& operator=(NFA&& other) = default;

  const Node& getRoot() const { return *root_; }

  const Node* run(const std::vector<T>& input) {
    const Node* currentNode = root_;
    for (const T& transition : input) {
      auto iter = currentNode->transitions_.find(transition);
      if (iter == currentNode->transitions_.end()) {
        return nullptr;
      } else {
        currentNode = iter->second;
      }
    }
    return currentNode;
  }

  const Node* addTransition(const Node& fromNode, T transition, Node newNode) {
    // No duplicate or updated transitions
    if (fromNode.transitions_.contains(transition)) {
      return nullptr;
    }

    // If a node with this value already exists, just add a transition to the
    // existing node
    const auto iter1 = nodes_.find(newNode);
    if (iter1 != nodes_.begin()) {
      fromNode.transitions_.emplace(std::move(transition), &*iter1);
      return nullptr;
    }

    // Otherwise, create a new node
    const auto& iter2 = nodes_.insert(std::move(newNode)).first;
    const Node* newNodePtr = &*iter2;
    fromNode.transitions_.emplace(std::move(transition), newNodePtr);
    return newNodePtr;
  }

  friend std::ostream& operator<<(std::ostream& out, const NFA& nfa) {
    const Node& root = *nfa.root_;
    out << root << ": " << prez::mapToString(root.getTransitions()) << '\n';
    for (const Node& node : nfa.nodes_) {
      if (node != root) {
        out << node << ": " << prez::mapToString(node.getTransitions()) << '\n';
      }
    }
    return out;
  }

private:
  struct nodeHash {
    size_t operator()(Node node) const noexcept {
      return std::hash<V>{}(node.value_);
    }
  };

  const Node* root_;
  std::unordered_set<Node, nodeHash> nodes_;
};

#endif
