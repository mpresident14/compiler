#ifndef TEMP_HPP
#define TEMP_HPP

#include <iostream>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <vector>

/*
 * Machine registers are enumerate from 0 through -15.
 * Variables are always positive
 */

enum MachineRegs {
  RSP = 0,
  RBP = -1,
  RAX = -2,
  RBX = -3,
  RCX = -4,
  RDX = -5,
  RDI = -6,
  RSI = -7,
  R8 = -8,
  R9 = -9,
  R10 = -10,
  R11 = -11,
  R12 = -12,
  R13 = -13,
  R14 = -14,
  R15 = -15,
};

const std::vector<int> callerSaveRegs{
  RAX, RCX, RDX, RDI, RSI, R8, R9, R10, R11
};

const std::vector<int> calleeSaveRegs{
  R12, R13, R14, R15, RBX, RSP, RBP
};

constexpr bool isRegister(int temp) noexcept { return temp < 0; }

inline std::ostream& operator<<(std::ostream& out, MachineRegs machineReg) {
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
