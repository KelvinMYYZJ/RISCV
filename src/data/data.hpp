#pragma once
#include <bits/stdc++.h>

#include "const_value.hpp"
#include "instrs.hpp"

// enum Stat { kIdle, kBusy };
enum class CalcType : uint {
  kAdd = 0,   // Time : 2
  kSub = 1,   // Time : 3
  kSll = 2,   // Time : 1
  kSlt = 3,   // Time : 2
  kSltu = 4,  // Time : 2
  kXor = 5,   // Time : 1
  kSrl = 6,   // Time : 1
  kSra = 7,   // Time : 1
  kOr = 8,    // Time : 1
  kAnd = 9,   // Time : 1
  kEq = 10,   // Time : 2
  kNeq = 11,  // Time : 2
  kLt = 12,   // Time : 2
  kGe = 13,   // Time : 2
  kLtu = 14,  // Time : 2
  kGeu = 15,  // Time : 2
};
// the number of clk cycle it takes to finish certain calculation.
const int kTimeOfCalc[] = {2, 3, 1, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
const int kTimeOfLoad = 5;
const int kTimeOfWrite = 5;

struct RSInfo {
  InstrInfo instr;
  uint pc;
  uint rs1_val;
  uint rs2_val;
  uint rename_rs1 = NIDX;
  uint rename_rs2 = NIDX;
  uint order;     // the index in the instr_queue
  uint step = 0;  // the step of LS instruction
  RSInfo() = default;
  RSInfo(const InstrInfo& _instr, uint _pc) : instr(_instr), pc(_pc) {}
  bool Ready() {
    return rename_rs1 == NIDX && rename_rs2 == NIDX ||
           ((instr.op_type == BasicOpType::kStoreMem || instr.op_type == BasicOpType::kLoadMem) && step == 0 &&
            rename_rs1 == NIDX);
  }
};

struct InstrQueueInfo {
  bool ready;
  // NIDX if not in rs
  uint pos_rs;
  bool need_cdb;
  InstrInfo instr;
  uint pc;
  uint result;
  uint tar_addr;
  bool prediction;
  InstrQueueInfo(uint _instr = 0, uint _pc = 0)
      : instr(_instr), pc(_pc), pos_rs(NIDX), ready(false), need_cdb(false), prediction(false) {}
};

struct ALUInfo {
  CalcType calc_type;
  uint lhs;
  uint rhs;
  uint ret;
  uint ret_pos;  // the index in the instr_queue or rs (if is an instruction that should return to rs)
  bool return_to_rs;
  int remain_time;
  ALUInfo() = default;
  ALUInfo(CalcType _calc_type, uint _lhs, uint _rhs, uint _ret_pos, bool _return_to_rs = false)
      : calc_type(_calc_type),
        lhs(_lhs),
        rhs(_rhs),
        ret_pos(_ret_pos),
        return_to_rs(_return_to_rs),
        remain_time(kTimeOfCalc[int(calc_type)]) {
    Calc();
  }
  void Calc() {
    switch (calc_type) {
      case CalcType::kAdd:
        ret = lhs + rhs;
        break;
      case CalcType::kSub:
        ret = lhs - rhs;
        break;
      case CalcType::kSll:
        ret = lhs << rhs;
        break;
      case CalcType::kSlt:
        ret = (int(lhs) < int(rhs)) ? 1 : 0;
        break;
      case CalcType::kSltu:
        ret = (lhs < rhs) ? 1 : 0;
        break;
      case CalcType::kXor:
        ret = (lhs ^ rhs);
        break;
      case CalcType::kSrl:
        ret = lhs >> rhs;
        break;
      case CalcType::kSra:
        ret = lhs >> rhs | (GetBit(lhs, 31) ? 0xFFFFFFF << (32 - rhs) : 0);
        break;
      case CalcType::kOr:
        ret = lhs | rhs;
        break;
      case CalcType::kAnd:
        ret = lhs & rhs;
        break;
      case CalcType::kEq:
        ret = lhs == rhs ? 1 : 0;
        break;
      case CalcType::kNeq:
        ret = lhs != rhs ? 1 : 0;
        break;
      case CalcType::kLt:
        ret = int(lhs) < int(rhs) ? 1 : 0;
        break;
      case CalcType::kGe:
        ret = int(lhs) >= int(rhs) ? 1 : 0;
        break;
      case CalcType::kLtu:
        ret = lhs < rhs ? 1 : 0;
        break;
      case CalcType::kGeu:
        ret = lhs >= rhs ? 1 : 0;
        break;
    }
  }
};

enum class MemAccessType : uint { kByte = 0x0, kHalfWord = 0x1, kWord = 0x2 };
struct LInfo {
  MemAccessType mem_type;
  bool is_unsigned;
  uint addr;
  uint order;
  int remain_time = kTimeOfLoad;
  LInfo() = default;
  LInfo(uint func3, uint _addr, uint _order)
      : mem_type(MemAccessType(func3 & 0x3)), is_unsigned(func3 & 0x4), addr(_addr), order(_order) {}
};

struct SInfo {
  InstrInfo instr;
  MemAccessType mem_type;
  uint addr;
  uint val;
  int remain_time = kTimeOfWrite;
  SInfo(InstrInfo _instr) : instr(_instr) { /* TDOO */
  }
};
