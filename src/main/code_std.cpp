#include <bits/stdc++.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#ifndef ONLINE_JUDGE
#ifndef BUG
#define DEBUG
#endif
#endif
using namespace std;

uint HexStringToInt(const char* str) {
  uint ret = 0;
  for (int i = 0; i < strlen(str); ++i) ret = (ret << 4) + (isalpha(str[i]) ? (str[i] - 'A' + 10) : (str[i] - '0'));
  return ret;
}

void ScanMem(u_char* mem) {
  char tmp[20];
  u_char* now_ptr = mem;
  while (scanf("%s", tmp) != EOF) {
    if (tmp[0] == '@') {
      now_ptr = mem + HexStringToInt(tmp + 1);
      continue;
    }
    *now_ptr++ = u_char(HexStringToInt(tmp));
  }
}
const int total_mem = 1 << 20;
u_char mem_in[total_mem * 4], mem_out[total_mem * 4];
uint reg_in[33], reg_out[33];
uint &pc_in = reg_in[32], &pc_out = reg_out[32];

void ShowMem(const string& file_name) {
  fstream fout(file_name + ".dat", ios::out);
  fout.write(reinterpret_cast<char*>(mem_in), sizeof(mem_in));
  fout.close();
}
// 0-base
uint GetBits(uint obj, int st, int ed) { return (obj >> ed) & ((1 << (st - ed + 1)) - 1); }
bool GetBit(uint obj, int bit_pos) { return obj & (1 << bit_pos); }

bool Sig(uint obj) { return GetBit(obj, 31); }

uint CompToOri(uint obj) {
  return obj;
  // if (!Sig(obj)) return obj;
  // return ((~obj) + 1) ^ 0x80000000;
}

uint Ext(uint obj, int sig_pos) {
  if (!GetBit(obj, sig_pos)) return obj & ((1 << (sig_pos + 1)) - 1);
  return obj | (0xFFFFFFFF << (sig_pos + 1));
}

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

enum class BasicOpType : uint {
  kLoadMem = 0x3,    // op_code = 0000011 , ImmType = I
  kStoreMem = 0x23,  // op_code = 0010011 , ImmType = S
  kCalc = 0x33,      // op_code = 0110011 , ImmType = R
  kCalcI = 0x13,     // op_code = 0001011 , ImmType = I
  kBControl = 0x63,  // op_code = 0111011 , ImmType = B
  kJAL = 0x6f,       // op_code = 0111111 , ImmType = J
  kJALR = 0x67,      // op_code = 0110111 , ImmType = I
  kLUI = 0x37,       // op_code = 0100111 , ImmType = U
  kAUIPC = 0x17,     // op_code = 0001111 , ImmType = U
};

uint GetByte(const u_char* mem, uint addr) { return mem[addr]; }

uint GetHalfWord(const u_char* mem, uint addr) { return *(ushort*)(mem + addr); }

uint GetWord(const u_char* mem, uint addr) { return *(uint*)(mem + addr); }

struct WriteRecord {
  uint addr;
  uint val;
  enum WriteType { kByte, kHalfWord, kWord } write_type;
  bool is_available = false;
} temp_record;

void RecordWriteByte(uint addr, u_char val) {
  temp_record.is_available = true;
  temp_record.addr = addr;
  temp_record.val = val;
  temp_record.write_type = WriteRecord::kByte;
}

void RecordWriteHalfWord(uint addr, ushort val) {
  temp_record.is_available = true;
  temp_record.addr = addr;
  temp_record.val = val;
  temp_record.write_type = WriteRecord::kHalfWord;
}

void RecordWriteWord(uint addr, uint val) {
  temp_record.is_available = true;
  temp_record.addr = addr;
  temp_record.val = val;
  temp_record.write_type = WriteRecord::kWord;
}

void WriteByte(u_char* mem, uint addr, u_char val) { mem[addr] = val; }

void WriteHalfWord(u_char* mem, uint addr, ushort val) { *(ushort*)(mem + addr) = val; }

void WriteWord(u_char* mem, uint addr, uint val) { *(uint*)(mem + addr) = val; }

int main(int argc, char const* argv[]) {
#ifndef ONLINE_JUDGE
  if (argc > 1 && strcmp(argv[1], "debug") == 0) {
    if (argc > 2)
      freopen(argv[2], "r", stdin);
    else
      freopen("!input.txt", "r", stdin);
    // freopen("!output.txt","w",stdout);
  }
#endif
  memset(mem_in, 0, sizeof(mem_in));
  memset(mem_in, 0, sizeof(reg_in));
  pc_in = 0;
  ScanMem(mem_in);
  ShowMem("input_mem");
  memcpy(mem_out, mem_in, sizeof(mem_in));
  while (1) {
    uint ori_instr = GetWord(mem_in, pc_in);
    if (ori_instr == 0x0ff00513) {
      cout << (reg_in[10] & 0xff) << endl;
      break;
    }
    pc_out = pc_in + sizeof(uint);
    // get the basic type
    BasicOpType basic_op_type = BasicOpType(GetBits(ori_instr, 6, 0));
    if (basic_op_type == BasicOpType::kLoadMem) {
      ITypeInstr instr(ori_instr);
      if (instr.func3 == 0) {  // lb
        reg_out[instr.rd] = Ext(GetByte(mem_in, reg_in[instr.rs1] + instr.imm), 7);
      }
      if (instr.func3 == 1) {  // lh
        reg_out[instr.rd] = Ext(GetHalfWord(mem_in, reg_in[instr.rs1] + instr.imm), 15);
      }
      if (instr.func3 == 2) {  // lw
        reg_out[instr.rd] = GetWord(mem_in, reg_in[instr.rs1] + instr.imm);
      }
      if (instr.func3 == 4) {  // lbu
        reg_out[instr.rd] = GetByte(mem_in, reg_in[instr.rs1] + instr.imm);
      }
      if (instr.func3 == 5) {  // lhu
        reg_out[instr.rd] = GetHalfWord(mem_in, reg_in[instr.rs1] + instr.imm);
      }
    } else if (basic_op_type == BasicOpType::kStoreMem) {
      STypeInstr instr(ori_instr);
      if (instr.func3 == 0) {  // sb
        RecordWriteByte(reg_in[instr.rs1] + instr.imm, GetBits(reg_in[instr.rs2], 7, 0));
      }
      if (instr.func3 == 1) {  // sh
        RecordWriteHalfWord(reg_in[instr.rs1] + instr.imm, GetBits(reg_in[instr.rs2], 15, 0));
      }
      if (instr.func3 == 2) {  // sw
        RecordWriteWord(reg_in[instr.rs1] + instr.imm, reg_in[instr.rs2]);
      }
    } else if (basic_op_type == BasicOpType::kCalc) {
      RTypeInstr instr(ori_instr);
      if (instr.func3 == 0) {    // arithmetic
        if (instr.func7 == 0) {  // add
          reg_out[instr.rd] = reg_in[instr.rs1] + reg_in[instr.rs2];
        }
        if (instr.func7 == 0x20) {  // sub
          reg_out[instr.rd] = reg_in[instr.rs1] - reg_in[instr.rs2];
        }
      }
      if (instr.func3 == 1) {  // sll
        reg_out[instr.rd] = reg_in[instr.rs1] << GetBits(reg_in[instr.rs2], 4, 0);
      }
      if (instr.func3 == 2) {  // slt
        reg_out[instr.rd] = (int(reg_in[instr.rs1]) < int(reg_in[instr.rs2])) ? 1 : 0;
      }
      if (instr.func3 == 3) {  // sltu
        reg_out[instr.rd] = (reg_in[instr.rs1] < reg_in[instr.rs2]) ? 1 : 0;
      }
      if (instr.func3 == 4) {  // xor
        reg_out[instr.rd] = reg_in[instr.rs1] ^ reg_in[instr.rs2];
      }
      if (instr.func3 == 5) {
        if (instr.func7 == 0) {  // srl
          reg_out[instr.rd] = reg_in[instr.rs1] >> GetBits(reg_in[instr.rs2], 4, 0);
        }
        if (instr.func7 == 0x20) {  // sra
          reg_out[instr.rd] =
              reg_in[instr.rs1] >> GetBits(reg_in[instr.rs2], 4, 0) |
              (GetBit(reg_in[instr.rs1], 31) ? 0xFFFFFFF << (32 - GetBits(reg_in[instr.rs2], 4, 0)) : 0);
        }
      }
      if (instr.func3 == 6) {  // or
        reg_out[instr.rd] = reg_in[instr.rs1] | reg_in[instr.rs2];
      }
      if (instr.func3 == 7) {  // and
        reg_out[instr.rd] = reg_in[instr.rs1] & reg_in[instr.rs2];
      }
    } else if (basic_op_type == BasicOpType::kCalcI) {
      uint func3 = GetBits(ori_instr, 14, 12);
      if (func3 == 1 || func3 == 5) {  // RSType
        RSTypeInstr instr(ori_instr);
        if (instr.func3 == 1) {  // slli
          reg_out[instr.rd] = reg_in[instr.rs1] << instr.shamt;
        }
        if (instr.func3 == 5) {
          if (instr.func7 == 0) {  // srli
            reg_out[instr.rd] = reg_in[instr.rs1] >> instr.shamt;
          }
          if (instr.func7 == 0x20) {    // srai
            if (instr.func7 == 0x20) {  // sra
              reg_out[instr.rd] = reg_in[instr.rs1] >> instr.shamt |
                                  (GetBit(reg_in[instr.rs1], 31) ? 0xFFFFFFF << (32 - instr.shamt) : 0);
            }
          }
        }
      } else {  // IType
        ITypeInstr instr(ori_instr);
        if (instr.func3 == 0) {  // addi
          reg_out[instr.rd] = reg_in[instr.rs1] + instr.imm;
        }
        if (instr.func3 == 2) {  // slti
          reg_out[instr.rd] = (int(reg_in[instr.rs1]) < int(instr.imm)) ? 1 : 0;
        }
        if (instr.func3 == 3) {  // sltiu
          reg_out[instr.rd] = (reg_in[instr.rs1] < uint(instr.imm)) ? 1 : 0;
        }
        if (instr.func3 == 4) {  // xori
          reg_out[instr.rd] = reg_in[instr.rs1] ^ instr.imm;
        }
        if (instr.func3 == 6) {  // ori
          reg_out[instr.rd] = reg_in[instr.rs1] | instr.imm;
        }
        if (instr.func3 == 7) {  // andi
          reg_out[instr.rd] = reg_in[instr.rs1] & instr.imm;
        }
      }
    } else if (basic_op_type == BasicOpType::kBControl) {
      BTypeInstr instr(ori_instr);
      if (instr.func3 == 0) {  // beq
        if (reg_in[instr.rs1] == reg_in[instr.rs2]) {
          pc_out = pc_in + instr.imm;
        }
      }
      if (instr.func3 == 1) {  // bne
        if (reg_in[instr.rs1] != reg_in[instr.rs2]) {
          pc_out = pc_in + instr.imm;
        }
      }
      if (instr.func3 == 4) {  // blt
        if (int(reg_in[instr.rs1]) < int(reg_in[instr.rs2])) {
          pc_out = pc_in + instr.imm;
        }
      }
      if (instr.func3 == 5) {  // bge
        if (int(reg_in[instr.rs1]) >= int(reg_in[instr.rs2])) {
          pc_out = pc_in + instr.imm;
        }
      }
      if (instr.func3 == 6) {  // bltu
        if (reg_in[instr.rs1] < reg_in[instr.rs2]) {
          pc_out = pc_in + instr.imm;
        }
      }
      if (instr.func3 == 7) {  // bgeu
        if (reg_in[instr.rs1] >= reg_in[instr.rs2]) {
          pc_out = pc_in + instr.imm;
        }
      }
    } else if (basic_op_type == BasicOpType::kJAL) {  // jal
      JTypeInstr instr(ori_instr);
      if (instr.rd) reg_out[instr.rd] = pc_in + sizeof(uint);
      pc_out = pc_in + instr.imm;
    } else if (basic_op_type == BasicOpType::kJALR) {  // jalr
      ITypeInstr instr(ori_instr);
      if (instr.rd) reg_out[instr.rd] = pc_in + sizeof(uint);
      pc_out = (reg_in[instr.rs1] + instr.imm) & 0xFFFFFFFE;
    } else if (basic_op_type == BasicOpType::kLUI) {  // lui
      UTypeInstr instr(ori_instr);
      reg_out[instr.rd] = instr.imm << 12;
    } else if (basic_op_type == BasicOpType::kAUIPC) {  // auipc
      UTypeInstr instr(ori_instr);
      reg_out[instr.rd] = pc_in + (instr.imm << 12);
    } /*  else
       cerr << "fail!" << endl; */
    // memcpy(mem_in, mem_out, sizeof(mem_in));
    bool tmp = temp_record.is_available;
    if (temp_record.is_available) {
      temp_record.is_available = false;
      if (temp_record.write_type == WriteRecord::kByte) {
        WriteByte(mem_in, temp_record.addr, temp_record.val);
      }
      if (temp_record.write_type == WriteRecord::kHalfWord) {
        WriteHalfWord(mem_in, temp_record.addr, temp_record.val);
      }
      if (temp_record.write_type == WriteRecord::kWord) {
        WriteWord(mem_in, temp_record.addr, temp_record.val);
      }
    }
    static uint commit_num = 0;
    if (commit_num <= 2000) {
      std::cerr << commit_num++ << std::endl;
      std::cerr << "commit instr : " << std::hex << ori_instr << " at pc : " << std::hex << pc_in << std::endl;
      if (tmp) {
        std::cerr << "write mem : addr : " << std::hex << temp_record.addr << ", data : ";
        std::cerr << std::hex << temp_record.val << std::endl;
      }
      std::cerr << "---------reg---------" << endl;
      for (int i = 0; i < 32; ++i) std::cerr << "reg[" << i << "] : " << std::hex << reg_out[i] << endl;
      std::cerr << "--------!reg!--------" << endl;
    }
    memcpy(reg_in, reg_out, sizeof(reg_in));
  }
  return 0;
}