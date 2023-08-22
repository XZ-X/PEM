import sys
from tkinter.messagebox import NO
from turtle import back
from typing import Dict, Tuple
from collections import deque

# debug = True
debug = False
CFG = 'cfg'
RANGE = 'range'
SUCC = 'succ'
PRED = 'pred'
EXCEPTION = 'exception'
BBS = 'bbs'
RET = 'ret'


def detect_back_edge(func: Dict):
    visited = set()
    working_set = set()
    back_edges = set()
    working_stack = []
    cfg = func[CFG]
    entry = func[RANGE][0]
    working_stack.append(entry)

    while len(working_stack) > 0:
        current = working_stack[len(working_stack) - 1]
        if current in visited:
            working_stack.pop(len(working_stack) - 1)
            if current in working_set:
                working_set.remove(current)
            continue
        else:
            working_set.add(current)
            visited.add(current)

        for succ in cfg[current][SUCC]:
            if succ in working_set:
                back_edges.add((current, succ))
            elif succ not in visited:
                working_stack.append(succ)
    if debug:
      for e in back_edges:
          print("%x --> %x" % (e[0], e[1]))
    return back_edges


class ReachabilityRelation:
    _reachable_blocks: Dict[int, set]
    _reach_me_blocks: Dict[int, set]

    def __init__(self, cfg, exclude_edges=set()):
        self.cfg = cfg
        self.exclude_edges = exclude_edges
        self._reachable_blocks = {}
        self._reach_me_blocks = {}

    def _compute_reachability_relation(self, bb, cache: Dict[int, set], next, edge):
        change = True
        working_list = []
        visiting = set()
        visited = set()
        while change:
            change = False
            # initialize
            working_list.clear()
            visited.clear()
            visiting.clear()
            working_list.append(bb)
            visiting.add(bb)
            if bb not in cache:
                cache[bb] = set()
                cache[bb].add(bb)
            # DFS
            while len(working_list) != 0:
                current = working_list[-1]
                if debug:
                    print("WorkList: %s" % ["%x" % x for x in working_list])
                if current in visited:
                    # all the children are visited, merge results
                    working_list.pop()
                    prev_size = len(cache[current])
                    for succ in next(current):
                        if edge(current, succ) not in self.exclude_edges:
                            cache[current].update(cache[succ])
                    after_size = len(cache[current])
                    if prev_size != after_size:
                        change = True
                    continue
                visiting.remove(current)
                visited.add(current)

                for succ in next(current):
                    if succ in visited or succ in visiting:
                        continue
                    if edge(current, succ) in self.exclude_edges:
                        continue
                    if succ not in cache:
                        cache[succ] = set()
                        cache[succ].add(succ)

                    visiting.add(succ)
                    working_list.append(succ)

    def _compute_reachable_blocks_from(self, bb):
        self._compute_reachability_relation(bb, self._reachable_blocks,
                                            next=lambda bb: self.cfg[bb][SUCC],
                                            edge=lambda a, b: (a, b))

    def _compute_reach_me_blocks_from(self, bb):
        self._compute_reachability_relation(bb, self._reach_me_blocks,
                                            next=lambda bb: self.cfg[bb][PRED],
                                            edge=lambda a, b: (b, a))

    def get_reachable_blocks(self, bb):
        if bb in self._reachable_blocks:
            return self._reachable_blocks[bb]
        self._compute_reachable_blocks_from(bb)
        return self._reachable_blocks[bb]

    def get_reach_me_blocks(self, bb):
        if bb in self._reach_me_blocks:
            return self._reach_me_blocks[bb]
        self._compute_reach_me_blocks_from(bb)
        return self._reach_me_blocks[bb]


class LoopInfo:
    current_back_edge: Tuple
    reachability_no_back: ReachabilityRelation
    reachability_with_back: ReachabilityRelation
    function: Dict
    all_back_edge: set()
    _body: set()
    _exit: set()

    def __init__(self, current_back_edge, reachability_no_back,
                 reachability_with_back, all_back_edge, function):
        self.current_back_edge = current_back_edge
        self.reachability_no_back = reachability_no_back
        self.reachability_with_back = reachability_with_back
        self.all_back_edge = all_back_edge
        self.function = function
        self._get_loop_body()
        self._get_loop_exit()

    def _get_loop_body(self):
        reachable = reachability_no_back.get_reachable_blocks(
            self.current_back_edge[1])
        reach_me = reachability_no_back.get_reach_me_blocks(
            self.current_back_edge[0])
        self._body = reachable & reach_me

    def _get_loop_exit(self):
        # we assume that loop body has already been computed
        self._exit = set()
        for b in self._body:
            for succ in self.function[CFG][b][SUCC]:
                if succ not in self._body:
                    self._exit.add(succ)

    def _is_non_trivial_exit(self, bb):
        work_list = []
        visited = set()
        work_list.append(bb)
        visited.add(bb)
        while len(work_list) != 0:
            current = work_list[-1]
            work_list.pop()
            if current in self.function[EXCEPTION]:
                continue
            if len(self.function[CFG][current][SUCC]) == 0:
                return True
            for succ in self.function[CFG][current][SUCC]:
                if succ not in visited and succ not in self._body:
                    visited.add(succ)
                    work_list.append(succ)

    def _get_non_trivial_exit(self):
        if len(self._exit) == 1:
            return self._exit
        return set([exit for exit in self._exit if self._is_non_trivial_exit(exit)])

    def _compute_continuation(self, exits, reachability: ReachabilityRelation):
        exit_reachable_bbs = {}
        start_from = None
        for e in exits:
            start_from = e
            exit_reachable_bbs[e] = reachability.get_reachable_blocks(e)

        if len(exit_reachable_bbs) == 0:
            return None
        visiting = deque()
        visited = set()
        visiting.append(start_from)
        while len(visiting) != 0:
            current = visiting[0]
            visiting.popleft()
            if current in visited:
                continue

            success = True
            for e, bbs in exit_reachable_bbs.items():
                if current not in bbs:
                    success = False
                    break

            if success:
                return current

            visited.add(current)

            for succ in self.function[CFG][current][SUCC]:
                if succ not in visited and succ not in self._body:
                    visiting.append(succ)
         # end of while
        return None

    def get_continuation(self):
        non_trivial_exits = self._get_non_trivial_exit()
        ret = self._compute_continuation(
            non_trivial_exits, self.reachability_no_back)
        if ret is None:
            ret = self._compute_continuation(
                non_trivial_exits, self.reachability_with_back)
        return ret

    def get_loop_body(self):
        return self._body

    def get_loop_exit(self):
        return self._exit


def non_trivial_exit(bb, cfg, exception_bbs, exclude_bbs):
    work_list = []
    visited = set()
    work_list.append(bb)
    visited.add(bb)
    while len(work_list) != 0:
        current = work_list[-1]
        work_list.pop()
        if current in exception_bbs:
            continue
        if len(cfg[current][SUCC]) == 0:
            return True
        for succ in cfg[current][SUCC]:
            if succ not in visited and succ not in exception_bbs:
                visited.add(succ)
                work_list.append(succ)


FUNCTION_BEGIN = "FBEGIN"
FUNCTION_END = "FEND"
BB_BEGIN = "BBEGIN"
BB_END = "BEND"
SUCC_BEGIN = "VECBE"
SUCC_END = "VECEN"
PRED_BEGIN = SUCC_BEGIN
PRED_END = SUCC_END
LOOP_BEGIN = "LBEGIN"
LOOP_END = "LEND"

def writeln(fout, str):
  fout.write(str)
  fout.write('\n')

def dump_func_info(fout, func):
  writeln(fout, str(func[RANGE][0]))
  writeln(fout, str(func[RANGE][1]))
  if func[RET]:
    writeln(fout, '1')
  else:
    writeln(fout, '0')  
  

def dump_cfg(fout, func):
  bb_number = len(func[BBS])
  writeln(fout, str(bb_number))
  for addr, b in func[BBS].items():
    writeln(fout, BB_BEGIN)
    writeln(fout, str(b[0]))
    writeln(fout, str(b[1]))
    if addr in func[EXCEPTION]:
      writeln(fout, '1')
    else:
      writeln(fout, '0')
    writeln(fout, SUCC_BEGIN)
    for succ in func[CFG][addr][SUCC]:
      fout.write("%d "%succ)
    writeln(fout, "")
    writeln(fout, SUCC_END)
    writeln(fout, PRED_BEGIN)
    for pred in func[CFG][addr][PRED]:
      fout.write("%d "%pred)
    writeln(fout, "")
    writeln(fout, PRED_END)
    writeln(fout, BB_END)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Specify input file!")
        exit(-1)
    fin = ' '.join(open(sys.argv[1]+".cfg").readlines())
    program: Dict = eval(fin)
    fout = open(sys.argv[1]+".pa", 'w')
    all = len(program.items())
    i = 0
    for func_entry, func in program.items():
        if debug and func_entry != 0x2cc80:
            continue
        i += 1
        print("%5d/%d"%(i, all), sep='\r', end='\r')
        writeln(fout, FUNCTION_BEGIN)
        dump_func_info(fout, func)
        dump_cfg(fout, func)
        
        back_edges = detect_back_edge(func)
        reachability_no_back = ReachabilityRelation(
            program[func_entry][CFG], exclude_edges=back_edges)
        reachability_with_back = ReachabilityRelation(
            program[func_entry][CFG])
        
        loop_number = len(back_edges)    
        writeln(fout, str(loop_number))
        for be in back_edges:
            writeln(fout, LOOP_BEGIN)
            loop = LoopInfo(be, reachability_no_back=reachability_no_back,
                            reachability_with_back=reachability_with_back,
                            all_back_edge=back_edges, function=func)
            # tricky! when print back edge, the `from` is the end addr of bb
            writeln(fout, str(func[BBS][be[0]][1]))
            writeln(fout, str(be[1]))
            conti = loop.get_continuation()
            if conti is not None:
              writeln(fout, str(conti))
            else:
              writeln(fout, '0')
            writeln(fout, LOOP_END)
        writeln(fout, FUNCTION_END)

    fout.flush()
    # back_edges = detect_back_edge(program[func_entry])

    # loop = LoopInfo((0x2d5ff, 0x2d46d), reachability_no_back=reachability_no_back,
    #                 reachability_with_back=reachability_with_back,
    #                 all_back_edge=back_edges, function=program[func_entry])
    # conti = loop.get_continuation()
    # print(conti)
    # reachable = reachability_no_back.get_reachable_blocks(func_entry)
    # reachme = reachability_no_back.get_reach_me_blocks(func_entry)
    # print("Reachable: ")
    # print(["%x" % i for i in reachable])

    # print("Reachme: ")
    # print(["%x" % i for i in reachme])
