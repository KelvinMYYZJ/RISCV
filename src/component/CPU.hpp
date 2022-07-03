#pragma once
#include <bits/stdc++.h>
using std::cerr;
using std::cout;
using std::endl;
#include "buffer.hpp"
#include "data.hpp"
#include "mem_operator.hpp"
#include "mem_scanner.hpp"
#include "number_operator.hpp"
#include "queue.hpp"
#include "random.hpp"
const int kTotalMem = 1 << 20;
const int kRegNum = 32;
class CPU {
  // All the changes to the Memory will not come into effect until UpdateValue() is called.
  uint clk;
  u_char mem[kTotalMem << 2];
  struct MemModifyRecord {
    uint addr;
    uint val;
    MemAccessType store_type;
  };
  struct PredictInfo {
    static const uint kSize = 5;
    uint two_bit_buffer[1 << kSize] = {1};
    uint pc_of_idx[1 << kSize];
    void RecordResult(bool result, uint pc) {
			// as most pc end with 0b00, the effective part actually begin at the index 2
			pc >>= 2;
      uint now_idx = GetBits(pc, kSize - 1, 0);
      uint& now_buffer = two_bit_buffer[now_idx];
      if (pc == pc_of_idx[now_idx]) {
        pc_of_idx[now_idx] = pc;
        now_buffer = result ? 2 : 1;
        return;
      }
      if (result) {
        if (now_buffer != 3) ++now_buffer;
      } else {
        if (now_buffer) --now_buffer;
      }
    }
    bool Predict(uint pc) const {
			pc >>= 2;
      uint now_idx = GetBits(pc, 4, 0);
      const uint& now_buffer = two_bit_buffer[now_idx];
      return GetBit(now_buffer, 1);
    }
  };
  std::vector<MemModifyRecord> mem_modify_records;
  uint reg_nxt[kRegNum + 1];
  uint& pc_nxt = reg_nxt[kRegNum];
  uint reg_rename_nxt[kRegNum];
  Queue<InstrQueueInfo> instr_queue_nxt;
  Buffer<ALUInfo> alu_nxt;
  Buffer<RSInfo> rs_nxt;
  Queue<LInfo> load_buffer_nxt;
  bool clear_flag_nxt;
  uint clear_pc_nxt;
  PredictInfo predict_info_nxt;

  uint reg[kRegNum + 1];
  uint& pc = reg[kRegNum];
  uint reg_rename[kRegNum];
  Queue<InstrQueueInfo> instr_queue;
  Buffer<ALUInfo> alu;
  Buffer<RSInfo> rs;
  Queue<LInfo> load_buffer;
  bool clear_flag;
  uint clear_pc;
  PredictInfo predict_info;

  uint commit_num = 0;
  unsigned long long predict_success_num = 0;
  unsigned long long predict_fail_num = 0;
  // set true if get the instruction 0x0ff00513
  bool reach_end = false;
  void UpdateValue() {
    // mem_modify_records only contain one piece of record
    if (!mem_modify_records.empty()) {
      MemModifyRecord record = mem_modify_records.front();
      switch (record.store_type) {
        case MemAccessType::kByte:
          WriteByte(mem, record.addr, record.val);
          break;
        case MemAccessType::kHalfWord:
          WriteHalfWord(mem, record.addr, record.val);
          break;
        case MemAccessType::kWord:
          WriteWord(mem, record.addr, record.val);
          break;
      }
      mem_modify_records.clear();
    }
    memcpy(reg, reg_nxt, sizeof(reg));
    memcpy(reg_rename, reg_rename_nxt, sizeof(reg_rename));
    memcpy(&instr_queue, &instr_queue_nxt, sizeof(instr_queue));
    memcpy(&alu, &alu_nxt, sizeof(alu));
    memcpy(&rs, &rs_nxt, sizeof(rs));
    memcpy(&load_buffer, &load_buffer_nxt, sizeof(load_buffer));
    memcpy(&predict_info, &predict_info_nxt, sizeof(predict_info));
    clear_flag = clear_flag_nxt;
    clear_pc = clear_pc_nxt;
  }
  // check whether a range of memory address is readable
  bool MemReadable(uint addr, MemAccessType mem_type, int order = NIDX) {
    if (order == NIDX) order = instr_queue.tail;
    for (int i = instr_queue.head; i != order; i = (i + 1) == kDefaultLength ? 0 : i + 1)
      if (instr_queue[i].instr.op_type == BasicOpType::kStoreMem) {
        // if the instruction is ready to be commited, check the address
        if (instr_queue[i].ready) {
          if (MemAccessConflect(addr, mem_type, instr_queue[i].tar_addr,
                                MemAccessType(instr_queue[i].instr.func3 & 0x3)))
            return false;
          continue;
        }
        // if the address is not yet calcualted, so assume that it will crash with the target address
        if (instr_queue[i].pos_rs == NIDX) return false;
        const RSInfo& rs_info = rs[instr_queue[i].pos_rs];
        if (rs_info.step == 0 || rs_info.rename_rs1 == 0xFF) return false;
        // the address is already calculated, so check the address
        if (MemAccessConflect(addr, mem_type, rs_info.rs1_val, MemAccessType(instr_queue[i].instr.func3 & 0x3)))
          return false;
      }
    return true;
  }

  void InstrQueueWork() {
    if (clear_flag) {
      clear_flag_nxt = false;
      instr_queue_nxt.Clear();
      alu_nxt.Clear();
      rs_nxt.Clear();
      load_buffer_nxt.Clear();
      for (int i = 0; i < kRegNum; ++i) reg_rename_nxt[i] = NIDX;
      pc_nxt = clear_pc;
      return;
    }
    // push a task into cdb
    for (int order = instr_queue.head; order != instr_queue.tail; order = (order + 1) == kDefaultLength ? 0 : order + 1)
      if (instr_queue[order].need_cdb) {
        const uint& now_val = instr_queue[order].result;
        instr_queue_nxt[order].need_cdb = false;
        for (int i = 0; i < kDefaultLength; ++i)
          if (rs.avl[i]) {
            if (rs[i].rename_rs1 == order) {
              rs_nxt[i].rename_rs1 = NIDX;
              rs_nxt[i].rs1_val = now_val;
            }
            if (rs[i].rename_rs2 == order) {
              rs_nxt[i].rename_rs2 = NIDX;
              rs_nxt[i].rs2_val = now_val;
            }
          }
        break;
      }
    // commit an instruction if possible
    if (!instr_queue.Empty() && instr_queue.Front().instr.instr == 0x0ff00513)
      reach_end = true;
    else if (!instr_queue.Empty() && instr_queue.Front().ready && !instr_queue.Front().need_cdb) {
      const InstrQueueInfo& front_instr = instr_queue.Front();
      const InstrInfo& instr = front_instr.instr;
      instr_queue_nxt.Pop();
      if (instr.op_type == BasicOpType::kStoreMem) {
        MemModifyRecord obj;
        obj.val = front_instr.result;
        obj.addr = front_instr.tar_addr;
        obj.store_type = MemAccessType(instr.func3 & 0x3);
        mem_modify_records.push_back(obj);
      } else if (instr.op_type == BasicOpType::kBControl) {
        // prediction is wrong, clear everything
        predict_info_nxt.RecordResult(front_instr.result, front_instr.pc);
        if (front_instr.result != front_instr.prediction) {
          ++predict_fail_num;
          clear_flag_nxt = true;
          clear_pc_nxt = front_instr.result ? front_instr.pc + instr.imm : front_instr.pc + 4;
        } else
          ++predict_success_num;
      } else {
        // write regester
        if (instr.rd) {
          reg_nxt[instr.rd] = front_instr.result;
          if (reg_rename[instr.rd] == instr_queue.head) reg_rename_nxt[instr.rd] = NIDX;
        }
        if (instr.op_type == BasicOpType::kJALR) {
          clear_flag_nxt = true;
          clear_pc_nxt = front_instr.tar_addr;
        }
      }
      /* if (commit_num <= 2000) {
        // DEBUG
        std::cerr << commit_num++ << std::endl;
        std::cerr << "commit instr : " << std::hex << instr_queue.Front().instr.instr << " at pc : " << std::hex
                  << instr_queue.Front().pc << std::endl;
        if (instr_queue.Front().instr.op_type == BasicOpType::kStoreMem) {
          std::cerr << "write mem : addr : " << std::hex << mem_modify_records[0].addr << ", data : ";
          std::cerr << std::hex << mem_modify_records[0].val << std::endl;
        }
        std::cerr << "---------reg---------" << endl;
        for (int i = 0; i < kRegNum; ++i) std::cerr << "reg[" << i << "] : " << std::hex << reg_nxt[i] << endl;
        std::cerr << "--------!reg!--------" << endl;
      } */
    }

    // Read an instruction if possible
    if (!instr_queue.Full() && MemReadable(pc, MemAccessType::kWord)) {
      pc_nxt = pc + 4;
      InstrQueueInfo obj(GetWord(mem, pc), pc);
      if (obj.instr.op_type == BasicOpType::kJAL) {
        pc_nxt = pc + obj.instr.imm;
      }
      if (obj.instr.op_type == BasicOpType::kBControl) {
        // Branch Predection part
        // decide obj.prediction
        obj.prediction = predict_info.Predict(pc);
        if (obj.prediction) pc_nxt = pc + obj.instr.imm;
      }
      instr_queue_nxt.Push(obj);
    }
  }
  void RSWork() {
    if (clear_flag) return;
    // Try to get an instruction from instr_queue
    // cant get instructions when instr_queue is commiting
    if (!rs.Full() && !instr_queue.Empty() &&
        !(!instr_queue.Empty() && instr_queue.Front().ready && !instr_queue.Front().need_cdb))
      for (int i = instr_queue.head; i != instr_queue.tail; i = (i + 1) == kDefaultLength ? 0 : i + 1)
        if (instr_queue[i].pos_rs == NIDX && !instr_queue[i].ready) {
          const InstrInfo& now_instr = instr_queue[i].instr;
          if (now_instr.rd) reg_rename_nxt[now_instr.rd] = i;
          if (instr_queue[i].instr.op_type == BasicOpType::kJAL && !instr_queue[i].ready) {
            instr_queue_nxt[i].result = instr_queue[i].pc + 4;
            instr_queue_nxt[i].ready = true;
            instr_queue_nxt[i].need_cdb = true;
            break;
          }
          RSInfo obj(&now_instr, instr_queue[i].pc);
          const uint& rename1 = reg_rename[now_instr.rs1];
          if (rename1 == NIDX)
            obj.rs1_val = reg[now_instr.rs1];
          else {
            if (instr_queue[rename1].ready)
              obj.rs1_val = instr_queue[rename1].result;
            else
              obj.rename_rs1 = rename1;
          }
          const uint& rename2 = reg_rename[now_instr.rs2];
          if (rename2 == NIDX)
            obj.rs2_val = reg[now_instr.rs2];
          else {
            if (instr_queue[rename2].ready)
              obj.rs2_val = instr_queue[rename2].result;
            else
              obj.rename_rs2 = rename2;
          }
          obj.order = i;
          instr_queue_nxt[i].pos_rs = rs_nxt.Push(obj);
          // assume that RS only get a single instruction from instr_queue in one clk cycle
          break;
        }

    // push task into buffers
    if (!rs.Empty())
      for (int i = 0; i < kDefaultLength; ++i)
        if (rs.avl[i] && rs[i].Ready()) {
          const RSInfo& rs_info = rs[i];
          const InstrInfo& instr = *rs_info.instr;
          // treat LS instructions
          if ((instr.op_type == BasicOpType::kLoadMem || instr.op_type == BasicOpType::kStoreMem)) {
            if (rs_info.step == 0) {  // Memory access is in the first step : calculate the address
              if (alu.Full()) continue;
              ALUInfo obj(CalcType::kAdd, rs_info.rs1_val, instr.imm, i, true);
              alu_nxt.Push(obj);
              rs_nxt[i].step = 1;
              rs_nxt[i].rename_rs1 = 0xFF;  // an invalid register rename, meaning that the addr is in ALU
              break;
            }
            // Memory access is in the second step : load or store
            // push load task into load_buffer
            if (instr.op_type == BasicOpType::kLoadMem) {
              LInfo obj(instr.func3, rs_info.rs1_val, rs[i].order);
              if (load_buffer.Full() || !MemReadable(obj.addr, obj.mem_type, obj.order)) continue;
              rs_nxt.Pop(i);
              load_buffer_nxt.Push(obj);
              break;
            }
            // push store task into instr_queue
            rs_nxt.Pop(i);
            instr_queue_nxt[rs_info.order].ready = true;
            // use the result to store the exact address of the store instruction
            instr_queue_nxt[rs_info.order].result = rs_info.rs2_val;
            instr_queue_nxt[rs_info.order].tar_addr = rs_info.rs1_val;
            break;
          }
          if (instr.op_type == BasicOpType::kCalc) {
            if (alu.Full()) continue;
            CalcType calc_type;
            if (instr.func3 == 0) {
              if (instr.func7 == 0)
                calc_type = CalcType::kAdd;
              else  // if (instr.func7 == 0x20)
                calc_type = CalcType::kSub;
            } else if (instr.func3 < 5)
              calc_type = CalcType(instr.func3 + 1);
            else if (instr.func3 == 5) {
              if (instr.func7 == 0)
                calc_type = CalcType::kSrl;
              else  // if (instr.func7 == 0x20)
                calc_type = CalcType::kSra;
            } else
              calc_type = CalcType(instr.func3 + 2);
            uint lhs = rs_info.rs1_val;
            uint rhs = rs_info.rs2_val;
            if (instr.func3 == 1 || instr.func3 == 5) rhs = GetBits(rhs, 4, 0);
            ALUInfo obj(calc_type, lhs, rhs, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
          if (instr.op_type == BasicOpType::kCalcI) {
            if (alu.Full()) continue;
            CalcType calc_type;
            uint lhs, rhs;
            if (instr.func3 == 0)
              calc_type = CalcType::kAdd;
            else if (instr.func3 < 5)
              calc_type = CalcType(instr.func3 + 1);
            else if (instr.func3 == 5) {
              if (instr.func7 == 0)
                calc_type = CalcType::kSrl;
              else  // if (instr.func7 == 0x20)
                calc_type = CalcType::kSra;
            } else
              calc_type = CalcType(instr.func3 + 2);

            // get lhs and rhs
            lhs = rs_info.rs1_val;
            rhs = instr.imm;
            if (instr.func3 == 1 || instr.func3 == 5) rhs = instr.shamt;
            ALUInfo obj(calc_type, lhs, rhs, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
          if (instr.op_type == BasicOpType::kBControl) {
            if (alu.Full()) continue;
            CalcType calc_type;
            if (instr.func3 & 0x4)
              calc_type = CalcType(instr.func3 + 8);
            else
              calc_type = CalcType(instr.func3 + 10);
            uint lhs = rs_info.rs1_val;
            uint rhs = rs_info.rs2_val;
            ALUInfo obj(calc_type, lhs, rhs, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
          if (instr.op_type == BasicOpType::kLUI) {
            if (alu.Full()) continue;
            ALUInfo obj(CalcType::kSll, instr.imm, 12, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
          if (instr.op_type == BasicOpType::kAUIPC) {
            if (alu.Full()) continue;
            ALUInfo obj(CalcType::kAdd, rs_info.pc, instr.imm << 12, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
          if (instr.op_type == BasicOpType::kJALR) {
            if (alu.Full()) continue;
            ALUInfo obj(CalcType::kAdd, rs_info.instr->imm, rs_info.rs1_val, rs_info.order);
            alu_nxt.Push(obj);
            rs_nxt.Pop(i);
            break;
          }
        }
  }
  void ALUWork() {
    if (clear_flag) return;
    // treat the tasks in alu
    if (!alu.Empty())
      for (int i = 0; i < kDefaultLength; ++i)
        if (alu.avl[i]) {
          if (!alu[i].remain_time) {  // the task finished
            alu_nxt.Pop(i);
            const ALUInfo& alu_info = alu[i];
            if (alu_info.return_to_rs) {  // return the result to RS
              int pos_in_rs = alu_info.ret_pos;
              // use the rs1_val to store the result
              rs_nxt[pos_in_rs].rs1_val = alu_info.ret;
              rs_nxt[pos_in_rs].rename_rs1 = NIDX;
            } else {  // return the result to the instruction_queue
              int pos_in_instr_queue = alu_info.ret_pos;
              if (instr_queue[pos_in_instr_queue].instr.op_type == BasicOpType::kJALR) {
                instr_queue_nxt[pos_in_instr_queue].result = instr_queue_nxt[pos_in_instr_queue].pc + 4;
                instr_queue_nxt[pos_in_instr_queue].tar_addr = alu_info.ret & 0xFFFFFFFE;
              } else
                instr_queue_nxt[pos_in_instr_queue].result = alu_info.ret;
              instr_queue_nxt[pos_in_instr_queue].ready = true;
              if (instr_queue[pos_in_instr_queue].instr.op_type != BasicOpType::kBControl)
                instr_queue_nxt[pos_in_instr_queue].need_cdb = true;
            }
          } else
            alu_nxt[i].remain_time--;
        }
  }
  void LoadBufferWork() {
    if (clear_flag) return;
    // treat the first task in load_buffer
    if (!load_buffer.Empty()) {
      if (!load_buffer.Front().remain_time) {
        load_buffer_nxt.Pop();
        const LInfo& load_info = load_buffer.Front();
        uint& result = instr_queue_nxt[load_info.order].result;
        switch (load_info.mem_type) {
          case MemAccessType::kByte:
            result = GetByte(mem, load_info.addr);
            if (!load_info.is_unsigned) result = Ext(result, 7);
            break;
          case MemAccessType::kHalfWord:
            result = GetHalfWord(mem, load_info.addr);
            if (!load_info.is_unsigned) result = Ext(result, 15);
            break;
          case MemAccessType::kWord:
            result = GetWord(mem, load_info.addr);
            break;
        }
        instr_queue_nxt[load_info.order].ready = true;
        instr_queue_nxt[load_info.order].need_cdb = true;
      } else {
        load_buffer_nxt.Front().remain_time--;
      }
    }
  }

  // simulate the work done in a clk cycle (except the ValueUpadte)
  void ClkWork() {
    ++clk;
    int exe_order[] = {0, 1, 2, 3};
    std::random_shuffle(exe_order, exe_order + 4);
    for (int i = 0; i < 4; ++i) {
      switch (exe_order[i]) {
        case 0:
          InstrQueueWork();
          break;
        case 1:
          RSWork();
          break;
        case 2:
          ALUWork();
          break;
        case 3:
          LoadBufferWork();
          break;
      }
    }
  }

 public:
  void Work() {
    memset(mem, 0, sizeof(mem));
    memset(reg, 0, sizeof(reg));
    for (int i = 0; i < kRegNum; ++i) reg_rename[i] = NIDX;
    memset(reg_nxt, 0, sizeof(reg_nxt));
    for (int i = 0; i < kRegNum; ++i) reg_rename_nxt[i] = NIDX;
    clear_flag = false;
    clear_flag_nxt = false;
    ScanMem(mem);
    clk = 0;
    srand(time(NULL));
    while (1) {
      ClkWork();
      UpdateValue();
      if (reach_end) {
        cout << (reg[10] & 0xff) << endl;
        cerr << predict_success_num << ' ' << predict_fail_num + predict_success_num << ' ' << clk << endl;
        return;
      }
    }
  }
};