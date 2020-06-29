#include "src/assembly/reg_alloc/interference_graph.hpp"

#include <prez/print_stuff.hpp>

using namespace std;
using namespace assem;


namespace {

auto removeNode(
    unordered_map<int, unordered_set<int>>& graphCopy,
    const unordered_map<int, unordered_set<int>>::const_iterator& iter) {
  for (int temp : iter->second) {
    graphCopy.at(temp).erase(iter->first);
  }
  return graphCopy.erase(iter);
}

}  // namespace


void buildStack(unordered_map<int, unordered_set<int>>& graphCopy, stack<int>& stk) {
  size_t fewestNeighbors = SIZE_MAX;
  int tempFewestNeighbors;
  size_t origLen = graphCopy.size();

  auto iter = graphCopy.begin();
  while (iter != graphCopy.end()) {
    int temp = iter->first;
    // Machine registers are already colored
    if (isRegister(temp)) {
      ++iter;
      continue;
    }

    size_t numNeighbors = iter->second.size();
    if (numNeighbors < NUM_AVAIL_REGS) {
      // If the variable interferes with fewer than the number of available
      // registers, then we know we can color it, so we want it towards the
      // bottom of the stack (i.e. colored last)
      stk.push(temp);
      iter = removeNode(graphCopy, iter);
    } else {
      // Otherwise, keep track of the node with the fewest neighbors
      fewestNeighbors = min(fewestNeighbors, numNeighbors);
      tempFewestNeighbors = temp;
      ++iter;
    }
  }

  if (origLen == graphCopy.size()) {
    // We didn't find any variables interfering with fewer than the number of
    // available registers
    if (fewestNeighbors == SIZE_MAX) {
      // Didn't find any variables interfering with at least the number of
      // available registers either. Only machine registers left, so all done.
      return;
    } else {
      // Otherwise, remove the node with the fewest neighbors
      stk.push(tempFewestNeighbors);
      removeNode(graphCopy, graphCopy.find(tempFewestNeighbors));
    }
  }

  buildStack(graphCopy, stk);
}

/*
 * General idea: Look what I wrote to and see what is live afterward. Those
 * temporaries interfere. The only exception is that for moves, the src and dst
 * do not interfere.
 */
InterferenceGraph::InterferenceGraph(const FlowGraph& fgraph) {
  for (const auto& [instr, liveness] : fgraph.getNodes()) {
    instr->calcInterference(liveness.liveOut, graph_, moveMultimap_);
  }
}

pair<unordered_map<int, MachineReg>, vector<int>> InterferenceGraph::color() const {
  unordered_map<int, unordered_set<int>> graphCopy(graph_);
  stack<int> stk;
  buildStack(graphCopy, stk);

  unordered_map<int, MachineReg> coloring;
  vector<int> spilled;

  // Color the machine registers their own color
  for (size_t reg = 0; reg < NUM_AVAIL_REGS; ++reg) {
    coloring.emplace(reg, static_cast<MachineReg>(reg));
  }

  // Is this color reg available for temp?
  auto canColor = [this, &coloring](MachineReg reg, int temp) {
    const unordered_set<int>& neighbors = graph_.at(temp);
    return none_of(neighbors.cbegin(), neighbors.cend(), [&coloring, &reg](int neighbor) {
      auto iter = coloring.find(neighbor);
      return iter != coloring.end() && iter->second == reg;
    });
  };

mainLoop:
  while (!stk.empty()) {
    int temp = stk.top();
    stk.pop();
    // Find a color for the variable that none of its neighbors have been
    // assigned

    // Biased coloring: if we have "movq %t1 %t2", try to put them in the same
    // register
    std::bitset<NUM_AVAIL_REGS> moveRegs;
    // Get all the assigned colors of the temporaries that temp was moved
    // to/from
    auto movePartners = moveMultimap_.equal_range(temp);
    for (auto iter = movePartners.first; iter != movePartners.second; ++iter) {
      auto coloringIter = coloring.find(iter->second);
      if (coloringIter != coloring.end()) {
        moveRegs.set(coloringIter->second);
      }
    }

    // First try the registers of the move partners
    for (size_t reg = 0; reg < NUM_AVAIL_REGS; ++reg) {
      if (moveRegs[reg]) {
        if (canColor(static_cast<MachineReg>(reg), temp)) {
          coloring.emplace(temp, static_cast<MachineReg>(reg));
          goto mainLoop;
        }
      }
    }

    // Then try the rest
    for (size_t reg = 0; reg < NUM_AVAIL_REGS; ++reg) {
      if (!moveRegs[reg]) {
        if (canColor(static_cast<MachineReg>(reg), temp)) {
          coloring.emplace(temp, static_cast<MachineReg>(reg));
          goto mainLoop;
        }
      }
    }

    // No way to assign this variable to a register, so spill it
    spilled.push_back(temp);
  }

  return { move(coloring), move(spilled) };
}

std::ostream& operator<<(std::ostream& out, const InterferenceGraph& fgraph) {
  for (const auto& [temp, neighbors] : fgraph.graph_) {
    streamTemp(out, temp);
    out << " -> [";
    for (int t : neighbors) {
      streamTemp(out, t);
      out << ", ";
    }
    out << "]\n";
  }
  return out;
}
