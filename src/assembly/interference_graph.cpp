#include "src/assembly/interference_graph.hpp"

#include <prez/print_stuff.hpp>

using namespace std;


auto InterferenceGraph::insertIfNotExists(int temp) {
  auto iter = graph_.find(temp);
  if (iter == graph_.end()) {
    return graph_.emplace(temp, unordered_set<int>{}).first;
  }
  return iter;
}

auto removeNode(
    unordered_map<int, unordered_set<int>>& graphCopy,
    const unordered_map<int, unordered_set<int>>::const_iterator& iter) {
  for (int temp : iter->second) {
    graphCopy.at(temp).erase(temp);
  }
  return graphCopy.erase(iter);
}

void buildStack(
      unordered_map<int, unordered_set<int>>& graphCopy,
      stack<int>& stk) {
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
      // If the variable interferes with fewer than the number of available registers,
      // then we know we can color it, so we want it towards the bottom of the stack
      // (i.e. colored last)
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
    // We didn't find any variables interfering with fewer than the number of available
    // registers
    if (fewestNeighbors == SIZE_MAX) {
      // Didn't find any variables interfering with at least the number of available
      // registers either. Only machine registers left, so all done.
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
 * General idea: Look what I wrote to and see what is live afterward. Those temporaries
 * interfere. The only exception is that for moves, the src and dst do not interfere.
 */
InterferenceGraph::InterferenceGraph(const FlowGraph& fgraph) {
  for (const auto& [instr, liveness] : fgraph.getNodes()) {
    switch (instr->getType()) {
      case InstrType::MOVE: {
        int moveSrc = static_cast<const Move*>(instr)->getSrc();
        int moveDst = static_cast<const Move*>(instr)->getDst();
        unordered_set<int>& moveNeighbors = insertIfNotExists(moveDst)->second;
        for (int temp : liveness.liveOut) {
          if (temp != moveSrc && temp != moveDst) {
            insertIfNotExists(temp)->second.insert(moveDst);
            moveNeighbors.insert(temp);
          }
        }
        break;
      }
      case InstrType::OPER:
        for (int opDst : static_cast<const Operation*>(instr)->getDsts()) {
          unordered_set<int>& opNeighbors = insertIfNotExists(opDst)->second;
          for (int temp : liveness.liveOut) {
            if (temp != opDst) {
              insertIfNotExists(temp)->second.insert(opDst);
              opNeighbors.insert(temp);
            }
          }
        }
        break;
      default: ;
    }
  }
}


pair<unordered_map<int, MachineReg>, vector<int>> InterferenceGraph::color() const {
  unordered_map<int, unordered_set<int>> graphCopy(graph_);
  stack<int> stk;
  buildStack(graphCopy, stk);

  unordered_map<int, MachineReg> coloring;
  vector<int> spilled;
  mainLoop:
    while (!stk.empty()) {
      int temp = stk.top();
      stk.pop();
      // Find a color for the variable that none of its neighbors have been assigned
      const unordered_set<int>& neighbors = graph_.at(temp);
      for (MachineReg reg : AVAIL_REGS) {
        if (none_of(
            neighbors.cbegin(),
            neighbors.cend(),
            [&coloring, &reg](int neighbor){
              if (neighbor == reg) {
                return true;
              }
              auto iter = coloring.find(neighbor);
              return iter != coloring.end() && iter->second == reg;
            })) {
          coloring.emplace(temp, reg);
          goto mainLoop;
        }
      }

      // No way to assign this variable to a register, so spill it
      spilled.push_back(temp);
    }

  return { move(coloring), move(spilled) };
}

std::ostream& operator<<(std::ostream& out, const InterferenceGraph& fgraph) {
  for (const auto& [temp, neighbors] : fgraph.graph_) {
    out << temp << " -> " << neighbors << '\n';
  }
  return out;
}
