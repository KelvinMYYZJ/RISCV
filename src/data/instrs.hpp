#pragma once
#include <bits/stdc++.h>

#include "number_operator.hpp"

enum class BasicOpType : uint {
  kLoadMem = 0x3,    // op_code = 0000011 , ImmType = I
  kStoreMem = 0x23,  // op_code = 0010011 , ImmType = S
  kCalc = 0x33,      // op_code = 0110011 , ImmType = R
  kCalcI = 0x13,     // op_code = 0001011 , ImmType = I or RS
  kBControl = 0x63,  // op_code = 0111011 , ImmType = B
  kJAL = 0x6f,       // op_code = 0111111 , ImmType = J
  kJALR = 0x67,      // op_code = 0110111 , ImmType = I
  kLUI = 0x37,       // op_code = 0100111 , ImmType = U
  kAUIPC = 0x17,     // op_code = 0001111 , ImmType = U
};

// TODO

struct InstrInfo {
  uint instr;
  BasicOpType op_type;
  uint func3;
  uint func7;
  uint imm;
  uint shamt;
  uint rs1 = 0;
  uint rs2 = 0;
  uint rd = 0;
  InstrInfo(uint _instr = 0) : instr(_instr), op_type(BasicOpType(GetBits(_instr, 6, 0))) {
    if (op_type == BasicOpType::kCalcI) func3 = GetBits(instr, 14, 12);
    if (op_type == BasicOpType::kLoadMem || op_type == BasicOpType::kJALR ||
        (op_type == BasicOpType::kCalcI && func3 != 1 && func3 != 5)) {  // I-Type
      imm = CompToOri(Ext(GetBits(instr, 31, 20), 11));
      rs1 = GetBits(instr, 19, 15);
      func3 = GetBits(instr, 14, 12);
      rd = GetBits(instr, 11, 7);
    } else if (op_type == BasicOpType::kCalcI) {  // RS-Type
      func7 = GetBits(instr, 31, 25);
      shamt = GetBits(instr, 24, 20);
      rs1 = GetBits(instr, 19, 15);
      func3 = GetBits(instr, 14, 12);
      rd = GetBits(instr, 11, 7);
    } else if (op_type == BasicOpType::kCalc) {  // R-Type
      func7 = GetBits(instr, 31, 25);
      rs2 = GetBits(instr, 24, 20);
      rs1 = GetBits(instr, 19, 15);
      func3 = GetBits(instr, 14, 12);
      rd = GetBits(instr, 11, 7);
    } else if (op_type == BasicOpType::kStoreMem) {  // S-Type
      imm = CompToOri(Ext(GetBits(instr, 31, 25) << 5 | GetBits(instr, 11, 7), 11));
      rs2 = GetBits(instr, 24, 20);
      rs1 = GetBits(instr, 19, 15);
      func3 = GetBits(instr, 14, 12);
    } else if (op_type == BasicOpType::kBControl) {  // B-Type
      imm = CompToOri(Ext(
          GetBit(instr, 31) << 12 | GetBit(instr, 7) << 11 | GetBits(instr, 20, 25) << 5 | GetBits(instr, 11, 8) << 1,
          12));
      rs2 = GetBits(instr, 24, 20);
      rs1 = GetBits(instr, 19, 15);
      func3 = GetBits(instr, 14, 12);
    } else if (op_type == BasicOpType::kJAL) {  // J-Type
      imm = CompToOri(Ext(GetBit(instr, 31) << 20 | GetBits(instr, 30, 21) << 1 | GetBit(instr, 20) << 11 |
                              GetBits(instr, 19, 12) << 12,
                          20));
      rd = GetBits(instr, 11, 7);
    } else if (op_type == BasicOpType::kLUI || op_type == BasicOpType::kAUIPC) {  // U-Type
      imm = CompToOri(Ext(GetBits(instr, 31, 12), 19));
      rd = GetBits(instr, 11, 7);
    }
  }
};