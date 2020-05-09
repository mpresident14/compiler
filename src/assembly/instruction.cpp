#include "instruction.hpp"


Label::Label(const std::string& name) : name_(name) {}
Move::Move(
    const std::string& asmCode,
    std::unique_ptr<Temp> src,
    std::unique_ptr<Temp> dst)
  : asmCode_(asmCode), src_(move(src)), dst_(move(dst)) {}
Operation::Operation(const std::string& asmCode,
    std::vector<std::unique_ptr<Temp>> reads,
    std::vector<std::unique_ptr<Temp>> writes)
  : asmCode_(asmCode), reads_(move(reads)), writes_(writes) {}

InstrType Label::getType() const noexcept { return InstrType::LABEL; }
InstrType Move::getType() const noexcept { return InstrType::MOVE; }
InstrType Operation::getType() const noexcept { return InstrType::OPER; }


vector<unique_ptr<Instruction>> spillInstructions(vector<unique_ptr<Instruction>>& instrs) {

}
