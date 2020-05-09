#ifndef TEMP_HPP
#define TEMP_HPP

#include <iostream>
#include <cstddef>
#include <string>


enum MachineRegs {
  RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP, R8, R9, R10, R11, R12, R13, R14, R15, R16,
};

enum class TempType { REGISTER, VARIABLE };

struct Temp {
  virtual TempType getType() const noexcept = 0;
};

struct Register : public Temp {
  virtual TempType getType() const noexcept override;
  MachineRegs reg;
};

struct Variable : public Temp {
  virtual TempType getType() const noexcept override;
  size_t id;
};

#endif
