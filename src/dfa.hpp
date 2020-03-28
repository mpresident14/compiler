#ifndef DFA_HPP
#define DFA_HPP

#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>

#include <prez/print_stuff.hpp>

// TODO: If we specialize DFA for when T is an enum, we can use arrays for node
// transitions instead of a hashmap
template <typename V, typename T>
class DFA {
public:
  template <typename V2, typename T2>
  friend class DFA;

  struct Node {
  public:
    friend class DFA;

    Node(V value) : value_(std::move(value)) {}

    bool operator==(const Node& other) const { return value_ == other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
      return out << node.value_;
    }

    const V& getValue() const { return value_; }

    const std::unordered_map<T, Node*>& getTransitions() const {
      return transitions_;
    }

    /* FOR GENERATED DFA ONLY. DO NOT CALL THIS FUNCTION */
    void setTransitions(std::unordered_map<T, Node*>&& tranMap) {
      transitions_ = move(tranMap);
    }

  private:
    V value_;
    // "mutable" allows us to modify the map even when a node is const (e.g., when
    // retrieving it from a map)
    std::unordered_map<T, Node*> transitions_;
    bool deleterCalled_ = false;
  };

  friend struct Node;

  DFA(V value)
      : root_(new Node(value)), valueToNode_{ { std::move(value), root_ } }, size_(1) {}

  ~DFA() {
    // If invalidated via move construction/assignment
    if (!root_) {
      return;
    }

    // Delete via BFS
    std::queue<const Node*> q;
    q.push(root_);
    std::unordered_set<const Node*> visited = { root_ };
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
  DFA(DFA&& other)
      : root_(other.root_), valueToNode_(move(other.valueToNode_)), size_(other.size_) {
    other.root_ = nullptr;
  }
  DFA& operator=(const DFA& other) = delete;
  DFA& operator=(DFA&& other) {
    root_ = other.root_;
    size_ = other.size_;
    valueToNode_ = move(other.valueToNode_);
    other.root_ = nullptr;
  }

  Node* getRoot() const { return root_; }

  Node* run(const std::vector<T>& input) const {
    Node* currentNode = root_;
    for (const T& inputToken : input) {
      currentNode = step(currentNode, inputToken);
      if (currentNode == nullptr) {
        return nullptr;
      }
    }
    return currentNode;
  }

  /* Step from node with transition inputToken */
  static Node* step(Node* node, const T& inputToken) {
    auto iter = node->transitions_.find(inputToken);
    if (iter == node->transitions_.end()) {
      return nullptr;
    }
    return iter->second;
  }

  // TODO: Make newNodeValue a universal reference so that we can forward it
  Node* addTransition(Node* node, T transition, V newNodeValue) {
    // No duplicate or updated transitions
    if (node->transitions_.contains(transition)) {
      return nullptr;
    }

    // If a node with this value already exists, just add a transition to the
    // existing node
    if (valueToNode_.contains(newNodeValue)) {
      Node* successor = valueToNode_.at(newNodeValue);
      node->transitions_.emplace(std::move(transition), successor);
      return nullptr;
    }

    // Otherwise, create a new node
    // TODO: If we make valueToNode_ hold V*s and insert the address of
    // newNode.value, we can avoid expensive copies. Would have to define
    // equality and hash of V* to be that of V
    Node* newNode = new Node(newNodeValue);
    node->transitions_.emplace(std::move(transition), newNode);
    valueToNode_.emplace(newNodeValue, newNode);
    ++size_;
    return newNode;
  }


  template <typename NewValue, typename F>
  DFA<NewValue, T> convert(const F& valueConversion) const {
    using namespace std;
    using OldNode = typename DFA::Node;
    using NewNode = typename DFA<NewValue, T>::Node;

    DFA<NewValue, T> newDfa(valueConversion(root_->value_));
    unordered_map<const OldNode*, NewNode*> visited = { {root_, newDfa.root_} };
    queue<pair<const OldNode*, NewNode*>> q;
    q.push(pair(root_, newDfa.root_));

    while (!q.empty()) {
      const OldNode* oldNode = q.front().first;
      NewNode* newNode = q.front().second;
      q.pop();
      for (const auto& tran : oldNode->transitions_) {
        const OldNode* oldSuccessor = tran.second;
        NewNode* newSuccessor;
        auto iter = visited.find(oldSuccessor);
        if (iter == visited.end()) {

          // We don't use addTransition because it is possible that the value conversion will create
          // nodes with duplicate values that were not duplicates before the conversion. We want these
          // to remain separate.
          newSuccessor = new NewNode(valueConversion(oldSuccessor->getValue()));
          newDfa.addDirectTransition(newNode, tran.first, newSuccessor);
          ++newDfa.size_;
          q.push(pair(oldSuccessor, newSuccessor));
          visited.emplace(oldSuccessor, newSuccessor);
        } else {
          // If we have already visited this node in the old DFA, then don't create a new node
          newSuccessor = iter->second;
          newDfa.addDirectTransition(newNode, tran.first, newSuccessor);
        }
      }
    }

    return newDfa;
  }


  template <typename F1, typename F2, typename F3>
  void streamAsCode(
      std::ostream& out,
      const std::string& valueType,
      const std::string& tranType,
      const F1& convertValue,
      const F2& valueToStr,
      const F3& tranToStr) const {
    std::ostringstream init;
    std::ostringstream nodeDecls;
    std::ostringstream tranStmts;

    init << "#include <unordered_map>\n"
         << "#include <memory>\n"
         << "struct "
         << "Node {\n"
         << "Node(" << valueType << "&& v) : v_(std::move(v)) {}\n"
         << "Node* step(const " << tranType << "& t) const {\n"
         << "auto iter = ts_.find(t);\n"
            "if (iter == ts_.end()) {\n"
            "return nullptr;\n"
            "}\n"
            "return iter->second;\n"
            "}\n"
         << valueType << " v_;\n"
         << "std::unordered_map<" << tranType << ", Node*> ts_;};\n";

    tranStmts << "auto makeDFA(){\n";

    std::unordered_set<const Node*> visited = { root_ };
    std::queue<const Node*> q;
    q.push(root_);

    while (!q.empty()) {
      const Node* currentNode = q.front();
      q.pop();

      // Add node declaration
      nodeDecls << "auto n" << currentNode << "=std::make_unique<Node>("
                << valueToStr(convertValue(currentNode->value_)) << ");\n";

      // Add the transitions
      tranStmts << 'n' << currentNode << "->ts_={\n";
      for (const auto& tranAndNode : currentNode->transitions_) {
        const Node* successor = tranAndNode.second;
        tranStmts << '{' << tranToStr(tranAndNode.first) << ',' << 'n'
                  << successor << ".get()},\n";
        if (!visited.contains(successor)) {
          visited.insert(successor);
          q.push(successor);
        }
      }
      tranStmts << "};\n";
    }

    tranStmts << "return std::move(n" << root_ << ");\n}\nauto root=makeDFA();";

    // Write declarations and statements to the file
    // TODO: Change to .view() when implemented
    out << init.str() << '\n'
        << nodeDecls.str() << tranStmts.str();
  }


  size_t size() const noexcept { return size_; }

  friend std::ostream& operator<<(std::ostream& out, const DFA& dfa) {
    std::queue<const Node*> q;
    q.push(dfa.root_);
    std::unordered_set<const Node*> visited = { dfa.root_ };
    while (!q.empty()) {
      const Node* node = q.front();
      out << node->value_ << '\n';
      q.pop();
      for (auto& keyVal : node->getTransitions()) {
        const Node* successor = keyVal.second;
        out << "\t[" << keyVal.first << "] -> " << node->getValue() << '\n';
        if (!visited.contains(successor)) {
          q.push(successor);
          visited.insert(successor);
        }
      }
    }
    return out;
  }

private:

  void addDirectTransition(Node* fromNode, T transition, Node* toNode) {
    fromNode->transitions_.emplace(std::move(transition), toNode);
  }

  Node* root_;
  // Allows us to check whether a node with some value exists in the
  // DFA and grab a pointer to it.
  // TODO: When P0919R3 is added to clang/gcc, use is_transparent
  std::unordered_map<V, Node*> valueToNode_;
  // We don't use valueToNode_.size() because of the comment in convert.
  size_t size_;
};

#endif
