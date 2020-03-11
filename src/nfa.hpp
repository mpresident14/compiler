#ifndef NFA_HPP
#define NFA_HPP

#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// template<typename T>
// concept Hashable = requires(T a) {
//     { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
// };

template <typename V, typename T>
class NFA {
public:
  struct Node {
    friend class NFA;

    ~Node() {
      for (auto& keyVal : transitions_) {
        Node* node = keyVal.second;
        if (node) {
          delete node;
        }
      }
    }
    Node* addTransition(T transition, V newNodeValue) {
      if (transitions_.contains(transition)) {
        return nullptr;
      }

      Node* newNode = new Node(std::move(newNodeValue));
      transitions_.emplace(std::move(transition), newNode);
      return newNode;
    }

    V value_;
    std::unordered_map<T, Node*> transitions_;

  private:
    Node(V&& value) : value_(std::move(value)) {}
  };

  NFA(V value) : root_(new Node(std::move(value))) {}
  ~NFA() {
    if (root_) delete root_;
  };
  NFA(const NFA& other) = default;
  NFA(NFA&& other) = default;
  NFA& operator=(const NFA& other) = default;
  NFA& operator=(NFA&& other) = default;

  Node* getRoot() { return root_; }
  Node* run(const std::vector<T>& input) {
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

  // friend std::ostream& operator<<(std::ostream& out, const NFA& nfa) {
  //   Node* currentNode = root_;

  // }

private:
  Node* root_;
};

#endif
