#ifndef INTERFERENCE_GRAPH_HPP
#define INTERFERENCE_GRAPH_HPP

#include "src/assembly/flow_graph.hpp"
#include "src/assembly/temp.hpp"

#include <cstddef>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

class InterferenceGraph {
public:
  InterferenceGraph(const FlowGraph& fgraph);
  ~InterferenceGraph() = default;
  InterferenceGraph(const InterferenceGraph& other) = delete;
  InterferenceGraph(InterferenceGraph&& other) = delete;
  InterferenceGraph& operator=(const InterferenceGraph& other) = delete;
  InterferenceGraph& operator=(InterferenceGraph&& other) = delete;
  friend std::ostream& operator<<(std::ostream& out, const InterferenceGraph& fgraph);

  std::pair<std::unordered_map<int, MachineReg>, std::vector<int>> color() const;

private:
  std::unordered_map<int, std::unordered_set<int>> graph_;
  std::unordered_multimap<int, int> moveMultimap_;
};

#endif
