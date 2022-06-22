from curses.ascii import isdigit
import sys
import os

os.chdir(sys.path[0])
files = os.listdir('.')
c_files = []
for file in files:
    suffix = os.path.splitext(file)[1]
    print(suffix)
    if (suffix == '.c'):
        c_files.append(file)

for c_file in c_files:
    case_name = os.path.splitext(c_file)[0]
    fin = open(c_file,"r")
    code = fin.read()
    idx = code.find("// ")
    st_idx = idx + 3
    ed_idx = idx + 3
    while (isdigit(code[ed_idx])):
        ed_idx += 1
    std_out = code[st_idx : ed_idx] + '\n'
    fout = open(case_name+'.ans',"w")
    fout.write(std_out)
    fin.close()
    fout.close()
