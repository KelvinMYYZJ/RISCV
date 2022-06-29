#include <bits/stdc++.h>

#include "CPU.hpp"
using namespace std;

int main(int argc, char const *argv[]) {
#ifndef ONLINE_JUDGE
  if (argc > 1 && strcmp(argv[1], "debug") == 0) {
    if (argc > 2)
      freopen(argv[2], "r", stdin);
    else
      freopen("!input.txt", "r", stdin);
    // freopen("!output.txt","w",stdout);
  }
#endif
  CPU().Work();
}