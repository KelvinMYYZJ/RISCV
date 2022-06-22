#pragma once
#include <bits/stdc++.h>
using std::fstream;
using std::string;

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

void ShowMem(const string& file_name, const u_char* mem_in) {
  fstream fout(file_name + ".dat", std::ios::out);
  fout.write(reinterpret_cast<const char*>(mem_in), sizeof(mem_in));
  fout.close();
}