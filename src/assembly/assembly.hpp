#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

// #define DEBUG

#include "src/assembly/temp.hpp"

#include <bitset>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


struct Liveness {
  std::unordered_set<int> liveIn;
  std::unordered_set<int> liveOut;
};

namespace assem {

class Instruction;
using InstrPtr = std::unique_ptr<Instruction>;


class Instruction {
public:
  virtual ~Instruction() {}
  /* Returns true if liveOut changed */
  virtual bool updateLiveOut(
      std::unordered_set<int>& liveOut,
      const Instruction* nextInstr,
      const std::unordered_map<const Instruction*, Liveness>& fgraph) const;
  /* Returns true if liveIn changed */
  virtual bool updateLiveIn(std::unordered_set<int>& liveIn, const std::unordered_set<int>& liveOut)
      const;
  virtual void calcInterference(
      const std::unordered_set<int>& liveOut,
      std::unordered_map<int, std::unordered_set<int>>& igraph,
      std::unordered_multimap<int, int>& moveMap) const = 0;
  /* Returns true if this instruction still needs to be added */
  virtual bool spillTemps(std::vector<InstrPtr>& newInstrs) = 0;
  /* Assigns temps to machine registers and keeps track of which machine
   * registers are written */
  virtual void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring,
      std::bitset<NUM_AVAIL_REGS>& writtenRegs) = 0;
  virtual void toCode(std::ostream& out, const std::unordered_map<int, size_t>& varToStackOffset)
      const = 0;
  virtual void toStream(std::ostream& out) const = 0;
  friend std::ostream& operator<<(std::ostream& out, const Instruction& instr);
};

class Decl {
public:
  virtual ~Decl() {}
  virtual void toCode(std::ostream& out) = 0;

private:
  std::vector<InstrPtr> instrs_;
};

using DeclPtr = std::unique_ptr<Decl>;


class Program {
public:
  Program(std::vector<DeclPtr>&& decls);
  void toCode(std::ostream& asmFile);

private:
  std::vector<DeclPtr> decls_;
};


/********
 * Decl *
 ********/

class Function : public Decl {
public:
  Function(std::string_view name, std::vector<InstrPtr>&& instrs);
  void toCode(std::ostream& out) override;

private:
  std::bitset<NUM_AVAIL_REGS> regAlloc(
      const std::unordered_map<int, MachineReg>& coloring,
      const std::vector<int>& spilled);
  std::pair<std::vector<InstrPtr>, std::vector<InstrPtr>> preserveRegs(
      const std::bitset<NUM_AVAIL_REGS>& writtenRegs);

  std::string name_;
  std::vector<InstrPtr> instrs_;
  std::unordered_map<int, size_t> varToStackOffset_;
};


class VTable : public Decl {
public:
  VTable(std::string_view label, std::vector<std::string>&& fnNames);
  void toCode(std::ostream& out) override;

private:
  std::string label_;
  std::vector<std::string> fnNames_;
};


/***************
 * Instruction *
 ***************/

class Label : public Instruction {
public:
  Label(std::string_view name);
  void calcInterference(
      const std::unordered_set<int>& liveOut,
      std::unordered_map<int, std::unordered_set<int>>& igraph,
      std::unordered_multimap<int, int>& moveMap) const override;
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring,
      std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(std::ostream& out, const std::unordered_map<int, size_t>& varToStackOffset)
      const override;
  void toStream(std::ostream& out) const override;


  std::string name_;
};

class Move : public Instruction {
public:
  Move(int src, int dst);
  bool updateLiveIn(std::unordered_set<int>& liveIn, const std::unordered_set<int>& liveOut)
      const override;
  void calcInterference(
      const std::unordered_set<int>& liveOut,
      std::unordered_map<int, std::unordered_set<int>>& igraph,
      std::unordered_multimap<int, int>& moveMap) const override;
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring,
      std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(std::ostream& out, const std::unordered_map<int, size_t>& varToStackOffset)
      const override;
  void toStream(std::ostream& out) const override;


  int src_;
  int dst_;
};


/* An instruction that always falls thru to the next one */
class Operation : public Instruction {
public:
  Operation(
      std::string_view asmCode,
      std::vector<int>&& srcs,
      std::vector<int>&& dsts,
      bool hasMemRefs = false);
  bool updateLiveIn(std::unordered_set<int>& liveIn, const std::unordered_set<int>& liveOut)
      const override;
  void calcInterference(
      const std::unordered_set<int>& liveOut,
      std::unordered_map<int, std::unordered_set<int>>& igraph,
      std::unordered_multimap<int, int>& moveMap) const override;
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring,
      std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(std::ostream& out, const std::unordered_map<int, size_t>& varToStackOffset)
      const override;
  void toStream(std::ostream& out) const override;


  std::string asmCode_;
  std::vector<int> srcs_;
  std::vector<int> dsts_;
  bool hasMemRefs_;
};

/* A jmp instruction. Guaranteed to jump, will not fall thru */
class JumpOp : public Operation {
public:
  JumpOp(std::string_view asmCode, std::vector<int>&& srcs, std::vector<int>&& dsts, Label* jump);
  bool updateLiveOut(
      std::unordered_set<int>& liveOut,
      const Instruction*,
      const std::unordered_map<const Instruction*, Liveness>& nodes) const override;
  virtual void toStream(std::ostream& out) const override;


  Label* jump_;
};


/* Same as JumpOp, but might fall thru */
class CondJumpOp : public JumpOp {
public:
  using JumpOp::JumpOp;
  bool updateLiveOut(
      std::unordered_set<int>& liveOut,
      const Instruction* nextInstr,
      const std::unordered_map<const Instruction*, Liveness>& nodes) const override;
};


class Return : public Instruction {
public:
  explicit constexpr Return(bool hasValue) : hasValue_(hasValue) {}
  bool updateLiveOut(
      std::unordered_set<int>& liveOut,
      const Instruction* nextInstr,
      const std::unordered_map<const Instruction*, Liveness>& nodes) const override;
  bool updateLiveIn(std::unordered_set<int>& liveIn, const std::unordered_set<int>& liveOut)
      const override;
  void calcInterference(
      const std::unordered_set<int>& liveOut,
      std::unordered_map<int, std::unordered_set<int>>& igraph,
      std::unordered_multimap<int, int>& moveMap) const override;
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring,
      std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(std::ostream& out, const std::unordered_map<int, size_t>& varToStackOffset)
      const override;
  void toStream(std::ostream& out) const override;


  bool hasValue_;
};

/* For debugging */
void streamTemp(std::ostream& out, int temp);


}  // namespace assem

#endif  // ASSEMBLY_HPP
