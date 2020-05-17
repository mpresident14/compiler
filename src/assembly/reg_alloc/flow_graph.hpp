#ifndef FLOW_GRAPH_HPP
#define FLOW_GRAPH_HPP

#include "src/assembly/assembly.hpp"
#include "src/assembly/temp.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FlowGraph {
public:
  struct Liveness {
    std::unordered_set<int> liveIn;
    std::unordered_set<int> liveOut;
  };

  FlowGraph(const std::vector<assem::InstrPtr>& instrs);
  ~FlowGraph() = default;
  FlowGraph(const FlowGraph& other) = delete;
  FlowGraph(FlowGraph&& other) = delete;
  FlowGraph& operator=(const FlowGraph& other) = delete;
  FlowGraph& operator=(FlowGraph&& other) = delete;
  friend std::ostream& operator<<(std::ostream& out, const FlowGraph& fgraph);

  void computeLiveness();

  const std::vector<assem::InstrPtr>& getInstrs() const noexcept;
  const std::unordered_map<const assem::Instruction*, Liveness>& getNodes()
      const noexcept;

private:
  // Yes, this is a reference. To avoid unnecessary copies, the FlowGraph
  // is subject to the lifetime of the underlying instruction vector.
  const std::vector<assem::InstrPtr>& instrs_;
  std::unordered_map<const assem::Instruction*, Liveness> nodes_;
};

#endif
