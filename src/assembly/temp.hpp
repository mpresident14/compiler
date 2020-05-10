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

// const std::vector<int> callerSaveRegs{RAX, RCX, RDX, RDI, RSI, R8, R9, R10, R11};

constexpr bool isRegister(int temp) noexcept {
  return temp < 0;
}

inline std::string regToString(int machineReg) {
  switch (machineReg) {
    case RSP: return "%rsp";
    case RBP: return "%rbp";
    case RAX: return "%rax";
    case RBX: return "%rbx";
    case RCX: return "%rcx";
    case RDX: return "%rdx";
    case RDI: return "%rdi";
    case RSI: return "%rsi";
    case R8: return "%r8";
    case R9: return "%r9";
    case R10: return "%r10";
    case R11: return "%r11";
    case R12: return "%r12";
    case R13: return "%r13";
    case R14: return "%r14";
    case R15: return "%r15";
    default:
      throw std::invalid_argument(std::to_string(machineReg) + " is not a register.");
  }
}
#endif
