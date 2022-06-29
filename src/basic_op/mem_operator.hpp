#pragma once
#include <bits/stdc++.h>

#include "data.hpp"

bool MemAccessConflect(uint addr1_st, MemAccessType op1, uint addr2_st, MemAccessType op2) {
  uint addr1_ed = addr1_st;
  if (op1 == MemAccessType::kHalfWord) addr1_ed += 1;
  if (op1 == MemAccessType::kWord) addr1_ed += 3;
  uint addr2_ed = addr2_st;
  if (op2 == MemAccessType::kHalfWord) addr2_ed += 2;
  if (op2 == MemAccessType::kWord) addr2_ed += 3;
  return !(addr1_ed < addr2_st || addr2_ed < addr1_st);
}

uint GetByte(const u_char* mem, uint addr) { return mem[addr]; }

uint GetHalfWord(const u_char* mem, uint addr) { return *(ushort*)(mem + addr); }

uint GetWord(const u_char* mem, uint addr) { return *(uint*)(mem + addr); }

void WriteByte(u_char* mem, uint addr, u_char val) { mem[addr] = val; }

void WriteHalfWord(u_char* mem, uint addr, ushort val) { *(ushort*)(mem + addr) = val; }

void WriteWord(u_char* mem, uint addr, uint val) { *(uint*)(mem + addr) = val; }