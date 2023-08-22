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

tsec = ida_segment.get_segm_by_name(".text")
if tsec is None:
    lowest_addr = 0
    highest_addr = 0xffffffff
else:
    lowest_addr = tsec.start_ea
    highest_addr = tsec.end_ea

prog_name = idc.ARGV[1]
fout = open(prog_name+".fname", 'w')

funcs = [f for f in Functions()]
function_name = {}

for f in funcs:
  name = GetFunctionName(f)
  if name!= "":
    function_name[int(f)] = name
fout.write(str(function_name))
fout.flush()


qexit(0)