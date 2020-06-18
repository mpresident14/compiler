#include "src/assembly/temp.hpp"

using namespace std;


vector<int> regsAsInts(const vector<MachineReg>& regs) {
  vector<int> ints;
  transform(
      regs.cbegin(), regs.cend(), back_inserter(ints), [](MachineReg reg) {
        return static_cast<int>(reg);
      });
  return ints;
}

string regToString(MachineReg machineReg, u_char bytesChar) {
  string ret = "%";
  bool isOriginal;

  switch (machineReg) {
    case RSP:
      isOriginal = true;
      ret.append("sp");
      break;
    case RBP:
      isOriginal = true;
      ret.append("bp");
      break;
    case RAX:
      isOriginal = true;
      ret.append("ax");
      break;
    case RBX:
      isOriginal = true;
      ret.append("bx");
      break;
    case RCX:
      isOriginal = true;
      ret.append("cx");
      break;
    case RDX:
      isOriginal = true;
      ret.append("dx");
      break;
    case RDI:
      isOriginal = true;
      ret.append("di");
      break;
    case RSI:
      isOriginal = true;
      ret.append("si");
      break;
    case R8:
      isOriginal = false;
      ret.append("r8");
      break;
    case R9:
      isOriginal = false;
      ret.append("r9");
      break;
    case R10:
      isOriginal = false;
      ret.append("r10");
      break;
    case R11:
      isOriginal = false;
      ret.append("r11");
      break;
    case R12:
      isOriginal = false;
      ret.append("r12");
      break;
    case R13:
      isOriginal = false;
      ret.append("r13");
      break;
    case R14:
      isOriginal = false;
      ret.append("r14");
      break;
    case R15:
      isOriginal = false;
      ret.append("r15");
      break;
    default:
      throw invalid_argument(to_string(machineReg) + " is not a register.");
  }

  switch (bytesChar) {
    case 8:
      if (isOriginal) {
        ret.insert(ret.begin() + 1, 'r');
      }
      break;
    case 4:
      if (isOriginal) {
        ret.insert(ret.begin() + 1, 'e');
      } else {
        ret.push_back('d');
      }
      break;
    case 2:
      if (!isOriginal) {
        ret.push_back('w');
      }
      break;
    case 1:
      if (isOriginal) {
        if (ret[2] == 'i') {
          ret.push_back('l');
        } else {
          ret[2] = 'l';
        }
      } else {
        ret.push_back('b');
      }
      break;
    default:
      throw invalid_argument(
          "regToString: " + to_string(bytesChar) +
          " is not a valid register offset.");
  }

  return ret;
}
