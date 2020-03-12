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

    ~Node() {
      for (auto& keyVal : transitions_) {
        Node* node = keyVal.second;
        if (node) {
          delete node;
        }
      }
    }

    bool operator==(const Node& other) const { return value_ == other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
      out << prez::containerToString(node.value_);
      return out;
    }

    // Make these const
    const V& getValue() { return value_; }
    const std::unordered_map<T, Node*>& getTransitions() { return transitions_; }

  private:
    // Node(V value) : value_(std::move(value)) {}
    Node(V value) : value_(value) {}

    V value_;
    std::unordered_map<T, Node*> transitions_;
  };

  NFA(V value) : root_(new Node(value)) {
    // nodes_.emplace(std::move(value), root_);
    nodes_.emplace(value, root_);
    std::cout << prez::mapToString(nodes_) << std::endl;
  }//, nodes_({ {std::move(value), root_} }) {}
  ~NFA() {
    if (root_) delete root_;
  };
  NFA(const NFA& other) = default;
  NFA(NFA&& other) = default;
  NFA& operator=(const NFA& other) = default;
  NFA& operator=(NFA&& other) = default;

  Node* getRoot() const { return root_; }
  const Node* run(const std::vector<T>& input) {
    Node* currentNode = root_;
    for (const T& trans : input) {
      auto iter = currentNode->transitions_.find(trans);
      if (iter == currentNode->transitions_.end()) {
        return nullptr;
      } else {
        currentNode = iter->second;
      }
    }
    return currentNode;
  }

  Node* addTransition(Node* node, T transition, V newNodeValue) {
    std::cout << "newNodeValue: " << prez::containerToString(newNodeValue) << std::endl;
    // No duplicate or updated transitions
    if (node->transitions_.contains(transition)) {
      return nullptr;
    }

    // If a node with this value already exists, just add a transition to the
    // existing node
    if (nodes_.contains(newNodeValue)) {
      std::cout << "RAN" << std::endl;
      Node* successor = nodes_.at(newNodeValue);
      node->transitions_.emplace(std::move(transition), successor);
      return nullptr;
    }

    // Otherwise, create a new node
    Node* newNode = new Node(newNodeValue);
    node->transitions_.emplace(std::move(transition), newNode);
    auto pp = nodes_.emplace(newNodeValue, newNode);
    std::cout << pp.second << std::endl;
    // std::cout << prez::containerToString((pp.first)->first) << std::endl;
    // std::cout << (pp.first)->second << std::endl;
    std::cout << prez::mapToString(nodes_) << std::endl;
    return newNode;
  }

  friend std::ostream& operator<<(std::ostream& out, const NFA& nfa) {
    std::unordered_set<Node*> visited;
    return nfa.doStream(out, nfa.getRoot(), 0, visited);
  }

private:
  std::ostream& doStream(std::ostream& out, Node* node, size_t depth, std::unordered_set<Node*>& visited) const {
    out << std::string(depth, '\t') << *node << '\n';
    visited.insert(node);
    for (auto& transSuccs : node->transitions_) {
      if (!visited.contains(transSuccs.second)) {
        doStream(out, transSuccs.second, depth + 1, visited);
      }
    }
    return out;
  }

  // struct nodeHash {
  //   size_t operator()(Node* node) const noexcept {
  //     return std::hash<V>{}(node->value_);
  //   }
  // };

  // struct nodeEq {
  //   bool operator()(Node* m, Node* n) { return *m == *n; }
  // };

  Node* root_;
  // Store pointers but hash by actual object
  std::unordered_map<V, Node*> nodes_;
};

#endif
