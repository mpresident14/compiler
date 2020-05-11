#ifndef FLOW_GRAPH_HPP
#define FLOW_GRAPH_HPP

#include "src/assembly/instruction.hpp"
#include "src/assembly/temp.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class FlowGraph {
public:
  struct Node {
    std::unordered_set<int> liveIn;
    std::unordered_set<int> liveOut;
  };

  FlowGraph(const std::vector<InstrPtr>& instrs);
  ~FlowGraph() = default;
  FlowGraph(const FlowGraph& other) = delete;
  FlowGraph(FlowGraph&& other) = delete;
  FlowGraph& operator=(const FlowGraph& other) = delete;
  FlowGraph& operator=(FlowGraph&& other) = delete;
  friend std::ostream& operator<<(std::ostream& out, const FlowGraph& fgraph);

  std::unordered_map<size_t, MachineRegs> regAlloc();
  void computeLiveness();

  const std::vector<InstrPtr>& getInstrs() const noexcept;
  const std::unordered_map<const Instruction*, Node>& getNodes() const noexcept;


private:
  // Yes, this is a reference. To avoid unnecessary copies, the FlowGraph
  // is subject to the lifetime of the underlying instruction vector.
  const std::vector<InstrPtr>& instrs_;
  std::unordered_map<const Instruction*, Node> nodes_;
};

#endif
