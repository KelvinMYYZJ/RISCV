#pragma once
#include <bits/stdc++.h>

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