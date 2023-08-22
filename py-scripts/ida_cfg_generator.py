# get information about function(s)
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


no_return_funcs = set()

NO_RETURN_LIB_FUNC = {'exit', '_exit', 'abort','_abort', 'error', '_error', '__assert_fail'}

def is_exception_block(block):
    terminal_addr = PrevHead(block.endEA)
    disasm = GetDisasm(terminal_addr)
    if 'ret' in disasm:
        return False
    disasm = set(GetDisasm(terminal_addr).split())
    for no_ret in NO_RETURN_LIB_FUNC:
        if no_ret in disasm:
            return True
    if 'call' in disasm:
        callee = get_operand_value(terminal_addr, 0)
        if callee in no_return_funcs:
            return True
    return False

def is_no_return_funcs(f):
    func = idaapi.get_func(f)
    if not func.does_return():
        return True
    # potential plt!
    if f < lowest_addr or f > highest_addr:
        END_BR_64_SIZE = 4
        call_op = get_operand_value(f+END_BR_64_SIZE, 0)
        if call_op in plt:
            callee_name = plt[call_op]
            if callee_name in NO_RETURN_LIB_FUNC:
                return True
    return False

prog_name = idc.ARGV[1]
fin = open(prog_name + '.gswitch')
additional_edges = eval(' '.join(fin.readlines()))
fin = open(prog_name + '.plt')
plt = eval(' '.join(fin.readlines()))


def merge(funcs):
    ret = []
    for f in funcs:
        if f < lowest_addr or f > highest_addr:
            continue
        func = idaapi.get_func(f)
        bbs = [b for b in idaapi.FlowChart(func, flags=FC_PREDS)]        
        bb_starts = [b.startEA for b in bbs]
        for b in bbs:
            for s in b.succs():
                if s.startEA not in bb_starts:
                    func.end_ea = max(func.end_ea, s.endEA)
                    func.extend(s.startEA)
        for b in bbs:
            terminal_addr = PrevHead(b.endEA)
            if terminal_addr in additional_edges:
                additional_succs = additional_edges[terminal_addr]
                for additional_succ_start in additional_succs:
                    if additional_succ_start not in bb_starts:
                        func.extend(additional_succ_start)
                        func.end_ea = max(func.end_ea, additional_succ_start+0x10)
        ret.append(func)
    return ret


with open(prog_name + '.cfg', 'w') as output:
    funcs = [f for f in Functions()]
    function_objects = merge(funcs)
    def writeln(s):
        output.write(s)
        output.write("\n")
    # Dict {FunctionAddr -> ((start * end) * dict{BB.start -> BB} * cfg)}
    # BB: (start * end)
    # cfg: {BB.start -> ({predBBStartEA} * {succBBStartEA})}
    result = {}
    print("Find no return functions!")
    for f in funcs:
        if is_no_return_funcs(f):
            no_return_funcs.add(f)
            

    print("Iterate over functions!")    
    for func in function_objects:
                        
        function_addr = int(func.startEA)
        function_addr_end = int(func.endEA)
        func_addr_pair = (function_addr, function_addr_end)
        bbs = {}
        cfg = {}
        exception_blocks = set()
        ida_bbs = dict()
        for b in idaapi.FlowChart(func, flags=FC_PREDS):
            ida_bbs[b.startEA] = b
        
        for _, b in ida_bbs.items():
            bb_start = int(b.startEA)
            bb_end = int(b.endEA)
            bb_addr_pair = (bb_start, bb_end)
            bbs[bb_start] = bb_addr_pair
            preds = set()
            succs = set()
            if is_exception_block(b):
                exception_blocks.add(int(b.startEA))
            else:    
                for s in b.succs():
                    succs.add(int(s.startEA))

            for p in b.preds():
                if not is_exception_block(p):
                    preds.add(int(p.startEA))
            cfg[bb_start] = {'succ': succs, 'pred': preds}
        # enhance
        for bb_start, bb_range in bbs.items():
            bb_end = bb_range[1]            
            terminal_addr = PrevHead(bb_end)
            
            if terminal_addr in additional_edges:
                additional_succs = additional_edges[terminal_addr]
                for additional_succ_start in additional_succs:
                    if additional_succ_start in bbs:
                        cfg[bb_start]['succ'].add(int(additional_succ_start))
                        cfg[additional_succ_start]['pred'].add(int(bb_start))

        result[function_addr] = {
            'range': func_addr_pair,
            'bbs': bbs,
            'cfg': cfg,
            'exception': exception_blocks,
            'ret': func.does_return(),
        }
    print("Write to file!")
    output.write(str(result))
    output.flush()

qexit(0)
