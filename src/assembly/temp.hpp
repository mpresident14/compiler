#ifndef TEMP_HPP
#define TEMP_HPP

#include <bitset>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

/*
 * Machine registers are enumerate from 0 through -15.
 * Variables are always positive
 */

enum MachineReg {
  // Assign preserved registers first (usually more restricted)
  RBX,
  R12,
  R13,
  R14,
  R15,
  RAX,
  RCX,
  RDX,
  RDI,
  RSI,
  R8,
  R9,
  // Not available for variable assignment
  R10,
  R11,
  RSP,
  RBP,
  NREGS,
};

/* All registers except RSP, RBP, and the spill registers (R10 and R11) are
 * available for variables */
constexpr size_t NUM_AVAIL_REGS = 12;

const std::vector<MachineReg> ARG_REGS{ RDI, RSI, RDX, RCX, R8, R9 };

/* Not preserved across function calls */
const std::vector<MachineReg> CALLER_SAVE_REGS{ RAX, RCX, RDX, RDI, RSI,
                                                R8,  R9,  R10, R11 };

/* Preserved across function calls */
const std::unordered_set<MachineReg> CALLEE_SAVE_REGS{ R12,
                                                       R13,
                                                       R14,
                                                       R15,
                                                       RBX };

inline std::vector<int> regsAsInts(const std::vector<MachineReg>& regs) {
  std::vector<int> ints;
  transform(
      regs.cbegin(), regs.cend(), back_inserter(ints), [](MachineReg reg) {
        return static_cast<int>(reg);
      });
  return ints;
}

constexpr bool isRegister(int temp) noexcept { return temp >= 0; }

inline std::ostream& operator<<(std::ostream& out, MachineReg machineReg) {
  switch (machineReg) {
    case RSP:
      out << "%rsp";
      break;
    case RBP:
      out << "%rbp";
      break;
    case RAX:
      out << "%rax";
      break;
    case RBX:
      out << "%rbx";
      break;
    case RCX:
      out << "%rcx";
      break;
    case RDX:
      out << "%rdx";
      break;
    case RDI:
      out << "%rdi";
      break;
    case RSI:
      out << "%rsi";
      break;
    case R8:
      out << "%r8";
      break;
    case R9:
      out << "%r9";
      break;
    case R10:
      out << "%r10";
      break;
    case R11:
      out << "%r11";
      break;
    case R12:
      out << "%r12";
      break;
    case R13:
      out << "%r13";
      break;
    case R14:
      out << "%r14";
      break;
    case R15:
      out << "%r15";
      break;
    default:
      throw std::invalid_argument(
          std::to_string(machineReg) + " is not a register.");
  }
  return out;
}

static int temp = -1;

inline int newTemp() noexcept { return temp--; }

#endif
