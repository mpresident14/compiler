#include "src/assembly/reg_alloc/flow_graph.hpp"

#include <prez/print_stuff.hpp>

using namespace std;
using namespace assem;


FlowGraph::FlowGraph(const vector<InstrPtr>& instrs) : instrs_(instrs) {
  for (const InstrPtr& instr : instrs) {
    nodes_.emplace(instr.get(), Liveness{ unordered_set<int>{}, unordered_set<int>{} });
  }
}

/*
 * Iterate to fixed point to compute liveIn and liveOut of each instruction
 * liveOut = U_desc (desc.liveIn)
 * liveIn = (liveOut - def) + use */
void FlowGraph::computeLiveness() {
  if (instrs_.empty()) {
    return;
  }

  // For last instruction, only need to compute liveIn (liveOut is empty)
  const Instruction* lastInstr = instrs_.back().get();
  Liveness& lastLiveness = nodes_.at(lastInstr);
  lastInstr->updateLiveIn(lastLiveness.liveIn, lastLiveness.liveOut);

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto iter = next(instrs_.crbegin()); iter != instrs_.crend(); ++iter) {
      const Instruction* instr = iter->get();
      Liveness& node = nodes_.at(instr);

      // Compute liveOut
      changed |= instr->updateLiveOut(node.liveOut, prev(iter)->get(), nodes_);

      // Compute liveIn
      changed |= instr->updateLiveIn(node.liveIn, node.liveOut);
    }
  }
}

std::ostream& operator<<(std::ostream& out, const FlowGraph& fgraph) {
  for (const auto& instr : fgraph.instrs_) {
    const Liveness& liveness = fgraph.nodes_.at(instr.get());
    out << *instr << " -> IN: ";
    out << " [";
    for (int t : liveness.liveIn) {
      streamTemp(out, t);
      out << ", ";
    }
    out << "] OUT: ";
    out << " [";
    for (int t : liveness.liveOut) {
      streamTemp(out, t);
      out << ", ";
    }
    out << "]\n\n";
  }
  return out;
}

const std::vector<InstrPtr>& FlowGraph::getInstrs() const noexcept { return instrs_; }

const std::unordered_map<const Instruction*, Liveness>& FlowGraph::getNodes() const noexcept {
  return nodes_;
}
