#ifndef TEMP_HPP
#define TEMP_HPP

#include <bitset>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * Machine registers are enumerate from 0 through -15.
 * Variables are always positive
 */

enum MachineReg {
  RAX = 0,
  RBX,
  RCX,
  RDX,
  RDI,
  RSI,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  RSP,
  RBP,
};

/* All registers except RSP and RBP are available for variables */
constexpr size_t NUM_AVAIL_REGS = 14;

// NOTE: These are not vector<MachineReg> because I'm lazy and I want
// a syntactically succinct copy when I pass them into Instruction constructors.
const std::vector<int> ARG_REGS{ RDI, RSI, RDX, RCX, R8, R9 };

/* Not preserved across function calls */
const std::vector<int> CALLER_SAVE_REGS{ RAX, RCX, RDX, RDI, RSI,
                                         R8,  R9,  R10, R11 };

/* Preserved across function calls */
const std::vector<int> CALLEE_SAVE_REGS{ R12, R13, R14, R15, RBX, RBP, RSP };

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
