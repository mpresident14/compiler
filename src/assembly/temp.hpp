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
  RAX = 0,
  RCX,
  RDX,
  RDI,
  RSI,
  R8,
  R9,
  // Assign preserved registers last
  RBX,
  R12,
  R13,
  R14,
  R15,
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

std::vector<int> regsAsInts(const std::vector<MachineReg>& regs);
constexpr bool isRegister(int temp) noexcept { return temp >= 0; }

// See
// https://www.google.com/search?q=x86-64+register+sizes&sxsrf=ALeKk00XGcYm9CEIDtkl5b5vE3VJzDF05A:1591242890593&source=lnms&tbm=isch&sa=X&ved=2ahUKEwjywvrZoefpAhUJv54KHaYCCQgQ_AUoAnoECAwQBA&biw=832&bih=851#imgrc=srauU5gulxAS2M
std::string regToString(MachineReg machineReg, char numBytes);

static int temp = -1;
inline int newTemp() noexcept { return temp--; }

#endif
