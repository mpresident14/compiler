#include "src/assembly/reg_alloc/flow_graph.hpp"

#include <prez/print_stuff.hpp>

using namespace std;
using namespace assem;

// TODO: Give errors for uninitialized variables
// and warnings about defined but unused variables

// TODO(all files): Try to replace switch statements with virtual functions
// where possible (possibly add another layer of abstraction like we did
// for Operation, JumpOp and CondJumpOp)

/*********
 * utils *
 *********/

/* Returns true if a new element was inserted */
template <typename T, template <typename...> class Container>
bool setUnion(unordered_set<T>& to, const Container<T>& from) {
  bool newInsertion = false;
  for (const T& n : from) {
    newInsertion |= to.insert(n).second;
  }
  return newInsertion;
}

template <typename T, template <typename...> class Container>
void setMinus(unordered_set<int>& to, const Container<int>& from) {
  for (const T& n : from) {
    to.erase(n);
  }
}

FlowGraph::FlowGraph(const vector<InstrPtr>& instrs) : instrs_(instrs) {
  for (const InstrPtr& instr : instrs) {
    nodes_.emplace(
        instr.get(), Liveness{ unordered_set<int>{}, unordered_set<int>{} });
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
  switch (lastInstr->getType()) {
    case InstrType::MOVE:
      lastLiveness.liveIn.insert(static_cast<const Move*>(lastInstr)->getSrc());
      break;
    case InstrType::OPER:  // Fall Thru
    case InstrType::JUMP_OP:
    case InstrType::COND_JUMP_OP:
      lastLiveness.liveIn.insert(
          static_cast<const Operation*>(lastInstr)->getSrcs().cbegin(),
          static_cast<const Operation*>(lastInstr)->getSrcs().cend());
      break;
    case InstrType::RETURN:
      // %rax is a src if we are returning a value
      if (static_cast<const Return*>(lastInstr)->hasValue()) {
        lastLiveness.liveIn.insert(RAX);
      }
      break;
    default:;
  }


  bool changed = true;
  while (changed) {
    changed = false;
    for (auto iter = next(instrs_.crbegin()); iter != instrs_.crend(); ++iter) {
      const Instruction* instr = iter->get();
      Liveness& node = nodes_.at(instr);
      InstrType type = instr->getType();

      // Nothing is live after a return. %rax is live before
      // only if we return a value
      if (type == InstrType::RETURN) {
        if (static_cast<const Return*>(lastInstr)->hasValue()) {
          node.liveIn.insert(RAX);
          changed = true;
        };
        continue;
      }

      // Compute liveOut
      if (type == InstrType::JUMP_OP || type == InstrType::COND_JUMP_OP) {
        // Instruction jumps
        const Label* jumpDst = static_cast<const JumpOp*>(instr)->getJump();
        changed |= setUnion(node.liveOut, nodes_.at(jumpDst).liveIn);
      }

      if (type != InstrType::JUMP_OP) {
        // Instruction falls through
        changed |= setUnion(node.liveOut, nodes_.at(prev(iter)->get()).liveIn);
      }

      // Compute liveIn
      unordered_set<int> newLiveIn = node.liveOut;
      switch (type) {
        case InstrType::MOVE:
          newLiveIn.erase(static_cast<const Move*>(instr)->getDst());
          newLiveIn.insert(static_cast<const Move*>(instr)->getSrc());
          break;
        case InstrType::OPER:  // Fall thru
        case InstrType::JUMP_OP:
        case InstrType::COND_JUMP_OP:
          setMinus<int, vector>(
              newLiveIn, static_cast<const Operation*>(instr)->getDsts());
          setUnion(newLiveIn, static_cast<const Operation*>(instr)->getSrcs());
          break;
        default:;
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
    const FlowGraph::Liveness& liveness = fgraph.nodes_.at(instr.get());
    out << *instr << " -> IN:" << liveness.liveIn << " OUT:" << liveness.liveOut
        << '\n';
  }
  return out;
}

const std::vector<InstrPtr>& FlowGraph::getInstrs() const noexcept {
  return instrs_;
}

const std::unordered_map<const Instruction*, FlowGraph::Liveness>&
FlowGraph::getNodes() const noexcept {
  return nodes_;
}
