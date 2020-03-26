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
  struct Node {
  public:
    friend class DFA;

    Node(V value) : value_(std::move(value)) {}

    bool operator==(const Node& other) const { return value_ == other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) {
      return out << node.value_;
    }

    const V& getValue() const { return value_; }

    /* FOR GENERATED DFA ONLY. DO NOT CALL THIS FUNCTION */
    void setTransitions(std::unordered_map<T, const Node*>&& tranMap) {
      transitions_ = move(tranMap);
    }

    const std::unordered_map<T, const Node*>& getTransitions() const {
      return transitions_;
    }

  private:
    V value_;
    // "mutable" allows us to modify the map even when a node is const
    mutable std::unordered_map<T, const Node*> transitions_;
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
  static const Node* step(const Node* node, const T& inputToken) {
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
    // TODO: If we make valueToNode_ hold V*s and insert the address of
    // newNode.value, we can avoid expensive copies. Would have to define
    // equality and hash of V* to be that of V
    const Node* newNode = new Node(newNodeValue);
    node->transitions_.emplace(std::move(transition), newNode);
    valueToNode_.emplace(newNodeValue, newNode);
    ++size_;
    return newNode;
  }


  // TODO: Universal ref params
  const Node* addTransitionNoCheck(const Node* node, T transition, V newNodeValue) {
    const Node* newNode = new Node(newNodeValue);
    node->transitions_.emplace(std::move(transition), newNode);
    valueToNode_.emplace(newNodeValue, newNode);
    ++size_;
    return newNode;
  }


  // Requires v1 == v2 -> f(v1) == f(v2)
  template <typename NewValue, typename F>
  DFA<NewValue, T> convert(const F& valueConversion) {
    using namespace std;
    using OldNode = const typename DFA::Node*;
    using NewNode = const typename DFA<NewValue, T>::Node*;

    DFA<NewValue, T> newDfa(valueConversion(root_->value_));

    unordered_set<OldNode> visited = { root_ };
    queue<pair<OldNode, NewNode>> q;
    q.push(make_pair(root_, newDfa.getRoot()));

    while (!q.empty()) {
      OldNode oldNode = q.front().first;
      NewNode newNode = q.front().second;
      q.pop();
      for (const auto& tran : oldNode->transitions_) {
        OldNode oldSuccessor = tran.second;
        if (!visited.contains(oldSuccessor)) {
          visited.insert(oldSuccessor);
          // We don't use addTransition because it is possible that the value conversion will create
          // nodes with duplicate values that were not duplicates before the conversion. We want these
          // to remain separate.
          NewNode newSuccessor = newDfa.addTransitionNoCheck(newNode, tran.first, valueConversion(oldSuccessor->getValue()));
          q.push(make_pair(oldSuccessor, newSuccessor));
        } else {
          // If we have already visited this node in the old DFA, then don't create a new node
          newDfa.addTransition(newNode, tran.first, valueConversion(oldSuccessor->getValue()));
        }
      }
    }

    return newDfa;
  }


  template <typename F1, typename F2, typename F3>
  void writeToFile(
      const std::string& filename,
      const std::string& namespaceName,
      const std::string& includes,
      const std::string& valueType,
      const std::string& tranType,
      const F1& convertValue,
      const F2& valueToStr,
      const F3& tranToStr) {
    std::ostringstream init;
    std::ostringstream nodeDecls;
    std::ostringstream tranStmts;

    std::string headerGuard = filename;
    std::replace(headerGuard.begin(), headerGuard.end(), '/', '_');
    std::replace(headerGuard.begin(), headerGuard.end(), '.', '_');
    std::transform(
        headerGuard.begin(),
        headerGuard.end(),
        headerGuard.begin(),
        [](unsigned char c) { return std::toupper(c); });

    init << "#ifndef " << headerGuard << '\n'
         << "#define " << headerGuard << '\n'
         << "#include <unordered_map>\n"
         << "#include <memory>\n"
         << includes << '\n'
         << "namespace " << namespaceName << " {\n"
         << "struct "
         << "Node {\n"
         << "Node(" << valueType << "&& v) : v_(std::move(v)) {}\n"
         << "const "
         << "Node* step(const " << tranType << "& t) const {\n"
         << "auto iter = ts_.find(t);\n"
            "if (iter == ts_.end()) {\n"
            "return nullptr;\n"
            "}\n"
            "return iter->second;\n"
            "}\n"
         << valueType << " v_;\n"
         << "std::unordered_map<" << tranType << ", const Node*> ts_;};\n";

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
    std::ofstream outFile;
    outFile.open(filename);
    // TODO: Change to .view() when implemented
    outFile << init.str() << '\n'
            << nodeDecls.str() << tranStmts.str() << "}\n#endif\n";
    outFile.close();
  }


  size_t size() const noexcept { return size_; }

  friend std::ostream& operator<<(std::ostream& out, const DFA& dfa) {
    std::unordered_set<const Node*> visited;
    return dfa.doStream(out, dfa.getRoot(), 0, visited);
  }

private:
  std::ostream& doStream(
      std::ostream& out,
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
  // TODO: When P0919R3 is added to clang/gcc, use is_transparent
  std::unordered_map<V, const Node*> valueToNode_;
  // We don't use valueToNode_.size() because of the comment in convert.
  size_t size_;
};

#endif
