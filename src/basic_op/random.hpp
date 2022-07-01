#pragma once
#include <bits/stdc++.h>
#include <time.h>
uint Random() {
  static uint seed = time(NULL);
  seed = (25214903917 * seed) & ((1ull << 48) - 1);
  return seed;
}

uint RandomInt(uint M) { return Random() % M; }

template <class T>
void RandomShuffle(T* arr, int size) {
  for (int i = 0; i < size - 1; ++i) {
    int tmp_idx = RandomInt(size - i) + i;
		
  }
}