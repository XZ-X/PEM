import argparse
import random

from tqdm import tqdm
import load_data
import subprocess
from os import path
import re
from matplotlib import pyplot as plt
import numpy as np

random.seed(233)


def parse_args():    
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--src",
        type=str,
        default="ablation-study/branches/coreutils.clang12.O0.elf",
        help="Source binary file",
    )
    parser.add_argument(
        "--tgt",
        type=str,
        default="ablation-study/branches/coreutils.gcc94.O3.elf",
        help="Target binary file",
    )
    
    return parser.parse_args()


def parse_branch_info_one_func(branch_info):
    ret = []
    for r in branch_info.rounds:
        seed = r.seed
        brs = []
        for b in r.branches:
            brs.append((b.addr, abs(b.selectivity)))
        sorted_brs = sorted(brs, key=lambda x: (x[1], x[0]))
        ret.append((seed, sorted_brs))
    return ret


class Addr2line:
    def __init__(self, binary, addr2line="/usr/bin/addr2line"):
        self.process = subprocess.Popen(
            [addr2line, "-e", binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE
        )

    def lookup(self, addr):
        dbg_info = None
        try:
            self.process.stdin.write(b"0x%x\n" % addr)
            self.process.stdin.flush()
            out = self.process.stdout.readline()
            out_str = out.decode("utf-8")
            dbg_info = out_str.rstrip("\n")
            dbg_info = re.sub("\(.*\)", "", dbg_info)
        except IOError:
            raise Exception("Communication error with addr2line.")
        finally:
            ret = self.process.poll()
            if ret != None:
                raise Exception("addr2line terminated unexpectedly (%i)." % (ret))

        (file, line) = dbg_info.rsplit(":", 1)
        file = path.basename(file)
        if "??" in file:
            file = ""
        if "?" in line:
            line = "0"
        return (file, int(line))


def location_equal(loc1, loc2):
    if loc1[0] == loc2[0]:
        if loc1[0] == "":
            return None
        if -2 <= loc1[1] - loc2[1] <= 2:
            return True
    return False


def cal_match_score(scale_src, scale_range_tgt, is_idx=False):
    correct = set()
    wrong = set()
    for name in name_in_both:
        src_addr = name2addr_src[name]
        tgt_addr = name2addr_tgt[name]
        src_branch_info = parse_branch_info_one_func(branch_src[src_addr])
        tgt_branch_info = parse_branch_info_one_func(branch_tgt[tgt_addr])
        for round_id in range(0, 2):
            src_round = src_branch_info[round_id]
            tgt_round = tgt_branch_info[round_id]
            src_branches = src_round[1]
            tgt_branches = tgt_round[1]
            if len(src_branches) == 0 or len(tgt_branches) == 0:
                continue
            src_idx = int(len(src_branches) * scale_src)
            if is_idx:
                src_idx = int(scale_src)
                if src_idx>=0 and src_idx > len(src_branches)-1:
                    continue
                if src_idx<0 and -src_idx > len(src_branches):
                    continue
                src_idx = min(src_idx, len(src_branches) - 1)
                src_idx = max(src_idx, -len(src_branches))                
            src_item = src_branches[src_idx]
            src_info = src_addr2line.lookup(src_item[0] - 2)
            if is_idx:
                tgt_idx0 = scale_range_tgt[0]
                tgt_idx1 = scale_range_tgt[1]
                tgt_idx0 = min(tgt_idx0, len(tgt_branches))
                tgt_idx0 = max(tgt_idx0, -len(tgt_branches))
                tgt_idx1 = min(tgt_idx1, len(tgt_branches))
                tgt_idx1 = max(tgt_idx1, -len(tgt_branches))
                tgt_infos = [
                    tgt_addr2line.lookup(item[0] - 2)
                    for item in tgt_branches[tgt_idx0:tgt_idx1]
                ]
            else:
                tgt_idx0 = int(len(tgt_branches) * scale_range_tgt[0])
                tgt_idx1 = int(len(tgt_branches) * scale_range_tgt[1])
                tgt_selectivity0 = tgt_branches[tgt_idx0][1]
                tgt_selectivity1 = tgt_branches[tgt_idx1][1]
                tgt_infos = [
                    tgt_addr2line.lookup(item[0] - 2)
                    for item in tgt_branches
                    if item[1] <= tgt_selectivity1 and item[1] >= tgt_selectivity0
                ]
            ret = [location_equal(src_info, tgt_info) for tgt_info in tgt_infos]
            ret_nn = [x for x in ret if x != None]
            if len(ret_nn) == 0:
                continue
            if sum(ret_nn) > 0:
                correct.add("%s:%d" % (name, round_id))
            else:
                wrong.add("%s:%d" % (name, round_id))
    if len(correct) == 0:
        return (0, set(), set())
    return (len(correct) / (len(correct) + len(wrong)), correct, wrong)


# main
if __name__ == "__main__":
    args = parse_args()
    tmp = args.src
    args.src = args.tgt
    args.tgt = tmp
    branch_src = load_data.load_branch_info(args.src + ".branch")
    branch_tgt = load_data.load_branch_info(args.tgt + ".branch")
    fname_src = load_data.load_fname(args.src + ".fname")
    fname_tgt = load_data.load_fname(args.tgt + ".fname")
    name2addr_src = {v: k for k, v in fname_src.items()}
    name2addr_tgt = {v: k for k, v in fname_tgt.items()}
    name_in_src = set([fname_src[a] for a in set(branch_src.keys())])
    name_in_tgt = set([fname_tgt[a] for a in set(branch_tgt.keys())])
    name_in_both = name_in_src & name_in_tgt
    name_in_both = set([n for n in name_in_both if "single_binary_main" in n])    
    # used to calculate statistics
    total = 0
    for name in name_in_both:
        addr = name2addr_src[name]
        func = branch_src[addr]
        for round_id in range(0, 2):
            total += len(func.rounds[round_id].branches)
    # print(total)
    src_addr2line = Addr2line(binary=args.src)
    tgt_addr2line = Addr2line(binary=args.tgt)
    scores = []
    scores = []
    tick_strs = []
    for i in tqdm(range(0, 50)):
        score, _, _ = cal_match_score(i, (i, i + 1), is_idx=True)
        scores.append((i, score))
        tick_strs.append("%d" % (i))
    MAX = 50
    for i in tqdm(range(1, MAX)):
        if i == 1:
            reverse1 = 9999999
        else:
            reverse1 = -i + 1
        score, _, _ = cal_match_score(-i, (-i, reverse1), is_idx=True)
        scores.append((100 - i, score))
        tick_strs.append("%d" % (-(MAX - i)))
    score_sorted = sorted(scores, key=lambda x: x[0])
    print("# Matching Ratio w.r.t. Selectivity Ranking")
    print(score_sorted)
    
    score2 = []
    for i in tqdm(range(1, 50)):
        score, _, _ = cal_match_score(0, (0, i), is_idx=True)
        score2.append((i, score))        
    cnt = [int(s[1] * len(name_in_both)) for s in score2]
    cnt.insert(0, 0)
    bins = np.diff(cnt)
    print()
    print("# Matching Distribution for the Minimal Selectivity")
    print(bins)

    score3 = []
    ticks_strs = []
    for i in tqdm(range(1, 50)):
        score, _, _ = cal_match_score(-1, (-i, 99999), is_idx=True)
        score3.append((50 - i, score))
        ticks_strs.append("%d" % (-i))        
    cnt3 = [int(s[1] * len(name_in_both)) for s in score3]
    cnt3.insert(0, 0)
    bins3 = np.diff(cnt3)
    print()
    print("# Matching Distribution for the Maximal Selectivity")
    print(bins3)
