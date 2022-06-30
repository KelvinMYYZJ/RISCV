#pragma once
#include <bits/stdc++.h>

#include "const_value.hpp"
// simple queue
// return a 0-base index to staisfy special need
template <class T, int Len = kDefaultLength>
struct Queue {
  T val[Len];
  int head, tail;
  Queue() : head(0), tail(0) {}
  uint Push(const T& obj) {
    if (Full()) return NIDX;
    uint ret = tail;
    val[tail++] = obj;
    if (tail == Len) tail = 0;
    return ret;
  }
  void Pop() {
    if (Empty()) return;
    ++head;
    if (head == Len) head = 0;
  }
  bool Full() { return tail + 1 == head || tail + 1 == head + 32; }
  bool Empty() { return head == tail; }
  T& operator[](const int& idx) { return val[idx]; }
  T& Front() { return val[head]; }
  void Clear() {
    head = 0;
    tail = 0;
  }
};