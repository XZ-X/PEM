import binascii

import ida_kernwin
import ida_funcs
from idautils import *
from idaapi import *
from idc import *


import string
import sys

idaapi.auto_wait()

if len(idc.ARGV) < 2:
    print("Specify additional cfg edges and output!")
    exit(-1)

print(idc.ARGV)
prog_name = idc.ARGV[1]

def get_char_len(t):
  if t is None:
    return None
  if 'char[' in t:
    length = t.replace("char[", "").replace("]", "").strip()
    length = int(length)
    return length
  else:
    return None

def collect_string(start_addr, end_addr, ret):
  current_addr = start_addr
  while current_addr <= end_addr:
    t = idc.guess_type(current_addr)
    length_opt = get_char_len(t)
    if length_opt:
      ret[current_addr] = length_opt
    current_addr = NextHead(current_addr)

ret = {}
ro_sec = ida_segment.get_segm_by_name(".rodata")
collect_string(ro_sec.start_ea, ro_sec.end_ea, ret)
data_sec = ida_segment.get_segm_by_name(".data")
collect_string(data_sec.start_ea, data_sec.end_ea, ret)

fout = open(prog_name + '.str', 'w')
for addr,length in ret.items():
  fout.write("%x %d\n"%(addr, length))
fout.flush()

fout.close()
qexit(0)