#pragma once
#include <bits/stdc++.h>
#include "number_operator.hpp"
struct RTypeInstr {
  uint instr;
  uint func7;
  uint rs2;
  uint rs1;
  uint func3;
  uint rd;
  RTypeInstr(uint _instr) : instr(_instr) {
    func7 = GetBits(instr, 31, 25);
    rs2 = GetBits(instr, 24, 20);
    rs1 = GetBits(instr, 19, 15);
    func3 = GetBits(instr, 14, 12);
    rd = GetBits(instr, 11, 7);
  }
};

struct RSTypeInstr {
  uint instr;
  uint func7;
  int shamt;
  uint rs1;
  uint func3;
  uint rd;
  RSTypeInstr(uint _instr) : instr(_instr) {
    func7 = GetBits(instr, 31, 25);
    shamt = GetBits(instr, 24, 20);
    rs1 = GetBits(instr, 19, 15);
    func3 = GetBits(instr, 14, 12);
    rd = GetBits(instr, 11, 7);
  }
};

struct ITypeInstr {
  uint instr;
  int imm;
  uint rs1;
  uint func3;
  uint rd;
  ITypeInstr(uint _instr) : instr(_instr) {
    imm = CompToOri(Ext(GetBits(instr, 31, 20), 11));
    rs1 = GetBits(instr, 19, 15);
    func3 = GetBits(instr, 14, 12);
    rd = GetBits(instr, 11, 7);
  }
};

struct STypeInstr {
  uint instr;
  int imm;
  uint rs2;
  uint rs1;
  uint func3;
  STypeInstr(uint _instr) : instr(_instr) {
    imm = CompToOri(Ext(GetBits(instr, 31, 25) << 5 | GetBits(instr, 11, 7), 11));
    rs2 = GetBits(instr, 24, 20);
    rs1 = GetBits(instr, 19, 15);
    func3 = GetBits(instr, 14, 12);
  }
};

struct BTypeInstr {
  uint instr;
  int imm;
  uint rs2;
  uint rs1;
  uint func3;
  BTypeInstr(uint _instr) : instr(_instr) {
    imm = CompToOri(
        Ext(GetBit(instr, 31) << 12 | GetBit(instr, 7) << 11 | GetBits(instr, 20, 25) << 5 | GetBits(instr, 11, 8) << 1,
            12));
    rs2 = GetBits(instr, 24, 20);
    rs1 = GetBits(instr, 19, 15);
    func3 = GetBits(instr, 14, 12);
  }
};

struct UTypeInstr {
  uint instr;
  int imm;
  uint rd;
  UTypeInstr(uint _instr) : instr(_instr) {
    imm = CompToOri(Ext(GetBits(instr, 31, 12), 19));
    rd = GetBits(instr, 11, 7);
  }
};

struct JTypeInstr {
  uint instr;
  int imm;
  uint rd;
  JTypeInstr(uint _instr) : instr(_instr) {
    imm = CompToOri(Ext(
        GetBit(instr, 31) << 20 | GetBits(instr, 30, 21) << 1 | GetBit(instr, 20) << 11 | GetBits(instr, 19, 12) << 12,
        20));
    rd = GetBits(instr, 11, 7);
  }
};