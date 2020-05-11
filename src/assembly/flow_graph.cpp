#include "src/assembly/flow_graph.hpp"

#include <prez/print_stuff.hpp>

using namespace std;

/*********
 * utils *
 *********/
template<typename T, template <typename...> class Container>
void setUnion(unordered_set<T>& to, const Container<T>& from) {
  for (const T& n : from) {
    to.insert(n);
  }
}

template<typename T, template <typename...> class Container>
void setMinus(unordered_set<int>& to, const Container<int>& from) {
  for (const T& n : from) {
    to.erase(n);
  }
}


FlowGraph::FlowGraph(const vector<InstrPtr>& instrs)
    : instrs_(instrs) {
  for (const InstrPtr& instr : instrs) {
    nodes_.emplace(instr.get(), Node{unordered_set<int>{}, unordered_set<int>{}});
  }
}

/*
 * Iterate to fixed point to compute liveIn and liveOut of each instruction
 * liveOut = U_desc (desc.liveIn)
 * liveIn = (liveOut - def) + use */
void FlowGraph::computeLiveness() {
  // For last instruction, only need to compute liveIn (liveOut is empty)
  auto iter = instrs_.crbegin();
  const Instruction* lastInstr = iter->get();
  Node& lastNode = nodes_.at(instrs_.back().get());
  switch (lastInstr->getType()) {
    case InstrType::MOVE:
      lastNode.liveIn.insert(static_cast<const Move*>(lastInstr)->getSrc());
      break;
    case InstrType::OPER:
      lastNode.liveIn.insert(
          static_cast<const Operation*>(lastInstr)->getSrcs().cbegin(),
          static_cast<const Operation*>(lastInstr)->getSrcs().cend());
      break;
    default: ;
  }
  ++iter;

  bool changed = true;
  while (changed) {
    changed = false;
    for (; iter != instrs_.crend(); ++iter) {
      const Instruction* instr = iter->get();
      Node& node = nodes_.at(instr);

      // Compute liveOut
      if (instr->getType() == InstrType::OPER
          && static_cast<const Operation*>(instr)->getJumps().has_value()) {
        // Instruction jumps
        unordered_set<int> newLiveOut;
        for (Instruction* jumpInstr : *static_cast<const Operation*>(instr)->getJumps()) {
          setUnion(newLiveOut, nodes_.at(jumpInstr).liveIn);
        }
        if (node.liveOut != newLiveOut) {
          node.liveOut = move(newLiveOut);
          changed = true;
        }
      } else {
        // Instruction falls through
        Node& nextNode = nodes_.at(prev(iter)->get());
        if (node.liveOut != nextNode.liveIn) {
          node.liveOut = nextNode.liveIn;
          changed = true;
        }

      }

      // Compute liveIn
      unordered_set<int> newLiveIn = node.liveOut;
      switch (instr->getType()) {
        case InstrType::MOVE:
          newLiveIn.erase(static_cast<const Move*>(instr)->getDst());
          newLiveIn.insert(static_cast<const Move*>(instr)->getSrc());
          break;
        case InstrType::OPER:
          setMinus<int, vector>(newLiveIn, static_cast<const Operation*>(instr)->getDsts());
          setUnion(newLiveIn, static_cast<const Operation*>(instr)->getSrcs());
          break;
        default: ;
      }
      if (node.liveIn != newLiveIn) {
        node.liveIn = move(newLiveIn);
        changed = true;
      }
    }
  }
}


std::ostream& operator<<(std::ostream& out, const FlowGraph& fgraph) {
  for (const auto& instr : fgraph.instrs_) {
    const FlowGraph::Node& node = fgraph.nodes_.at(instr.get());
    out << *instr << " -> IN:" << node.liveIn << " OUT:" << node.liveOut << '\n';
  }
  return out;
}


const std::vector<InstrPtr>& FlowGraph::getInstrs() const noexcept {
  return instrs_;
}

const std::unordered_map<const Instruction*, FlowGraph::Node>& FlowGraph::getNodes() const noexcept {
  return nodes_;
}
