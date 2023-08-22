import sys
from typing import Dict, Tuple
from collections import deque

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Specify input file!")
        exit(-1)
    fin = ' '.join(open(sys.argv[1]+".cfg").readlines())    
    program: Dict = eval(fin)
    fname = ' '.join(open(sys.argv[1]+".fname").readlines())
    fname: Dict = eval(fname)
    ret = set()
    fout = open(sys.argv[1]+".ruleout", 'w')
    for func_entry, func in program.items():
      bb_number = len(func['bbs'])
      if bb_number <= 3:
        if func_entry not in fname:
          continue
        else:
          ret.add(fname[func_entry])
    fout.write(str(ret))
    fout.close()