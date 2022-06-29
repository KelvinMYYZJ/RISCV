#pragma once
#include <bits/stdc++.h>

#include "const_value.hpp"
template <class T, int Len = kDefaultLength>
struct Buffer {
  T val[Len];
  // avl[i] means there IS SOMETHING on index i
  bool avl[Len];
  int size;
  Buffer() : size(0), avl{0} {}
  int Push(const T& obj) {
    if (Full()) return NIDX;
    for (int i = 0; i < Len; ++i)
      if (!avl[i]) {
        val[i] = obj;
        avl[i] = true;
        ++size;
        return i;
      }
		return NIDX;
  }
  void Pop(int idx) {
    if (!avl[idx]) throw;
    --size;
    avl[idx] = false;
    return;
  }
  bool Full() { return size == Len; }
  bool Empty() { return !size; }
  T& operator[](int idx) { return val[idx]; }
  void Clear() {
    size = 0;
    for (int i = 0; i < Len; ++i) avl[i] = false;
  }
};