#ifndef TEMP_HPP
#define TEMP_HPP

#include <iostream>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <vector>
#include <bitset>

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


const std::vector<MachineReg> callerSaveRegs{
  RAX, RCX, RDX, RDI, RSI, R8, R9, R10, R11
};

const std::vector<MachineReg> calleeSaveRegs{
  R12, R13, R14, R15, RBX, RSP, RBP
};

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
#endif
