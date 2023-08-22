import argparse
import functools
from multiprocessing import Pool
import pickle
import random
from typing import Dict, Tuple
from load_data import load_fname, load_results, load_ruleout
import numpy as np
from functools import partial
from preprocess import FunctionEntry
from compare import ComparisonScore
from os import path


def parse_args(parser=None, add_only=False):
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--src",
        type=str,

        default="qemu-tci-linux-user/benchmarks/eval-dataset/eval-trex/libmagick7.so.O0.elf",

        help="Source binary file",
    )
    parser.add_argument(
        "--tgt",
        type=str,

        default="qemu-tci-linux-user/benchmarks/eval-dataset/eval-trex/libmagick7.so.O3.elf",

        help="Target binary file",
    )
    parser.add_argument(
        "--cmp",
        type=str,

        default="qemu-tci-linux-user/benchmarks/cmp-ret/eval-dataset/eval-trex/libmagick7.so.O0vsO3.pkl",

        help="Comparison Result file",
    )
    parser.add_argument("--sample", type=float, default=-1, help="Ratio")
    parser.add_argument("--ablation", action="store_true",
                        help="Ablation study")
    if add_only:
        return parser
    else:
        return parser.parse_args()


def create_idx_maps(cmp_dict):
    key_set = set(cmp_dict.keys())
    src_addrs = list(set([i[0] for i in key_set]))
    tgt_addrs = list(set([i[1] for i in key_set]))
    src_addr2idx = {addr: i for i, addr in enumerate(src_addrs)}
    tgt_addr2idx = {addr: i for i, addr in enumerate(tgt_addrs)}
    src_idx2addr = {i: addr for i, addr in enumerate(src_addrs)}
    tgt_idx2addr = {i: addr for i, addr in enumerate(tgt_addrs)}
    return src_addr2idx, src_idx2addr, tgt_addr2idx, tgt_idx2addr


def create_score_matrix(cmp_dict, src_addr2idx, tgt_addr2idx, score_func):
    len_src = len(src_addr2idx)
    len_tgt = len(tgt_addr2idx)
    score_matrix = np.zeros((len_tgt, len_src), dtype=np.float32)
    for (src_addr, tgt_addr), score in cmp_dict.items():
        src_idx = src_addr2idx[src_addr]
        tgt_idx = tgt_addr2idx[tgt_addr]
        score_matrix[tgt_idx, src_idx] = score_func(score)
    return score_matrix


# main
if __name__ == "__main__":    
    # parse args
    args = parse_args()
    # load pickle
    f_cmp = open(args.cmp, "rb")
    cmp_dict: Dict[Tuple[int, int], ComparisonScore] = pickle.load(f_cmp)
    f_cmp.close()
    # load fname
    fname_src = load_fname(args.src + ".fname")
    name2addr_src = {x: i for i, x in fname_src.items()}
    fname_tgt = load_fname(args.tgt + ".fname")
    name2addr_tgt = {x: i for i, x in fname_tgt.items()}
    # load ruleout
    ruleout_src = load_ruleout(args.src + ".ruleout")
    ruleout_tgt = load_ruleout(args.tgt + ".ruleout")
    # ruleout_src = {}
    # ruleout_tgt = {}
    cmp_fname_src = set()
    cmp_fname_tgt = set()
    src_keys = set([k[0] for k in cmp_dict.keys()])
    tgt_keys = set([k[1] for k in cmp_dict.keys()])
    for k in src_keys:
        cmp_fname_src.add(fname_src[k])
    for k in tgt_keys:
        cmp_fname_tgt.add(fname_tgt[k])
    has_answer = cmp_fname_src & cmp_fname_tgt - \
        set(ruleout_src) - set(ruleout_tgt)
    print("Has answer: %d" % len(has_answer))
    new_cmp_dict = {}
    if args.sample > 0:
        print("Sampling in 1:%d..." % args.sample)
        candidates = []
        for src, tgt in cmp_dict.keys():            
            if fname_tgt[tgt] not in has_answer or fname_src[src] not in has_answer:
                continue
            # if the compared function is too small
            if fname_src[src] in ruleout_src or fname_tgt[tgt] in ruleout_tgt:
                continue
            if fname_src[src] == fname_tgt[tgt]:
                new_cmp_dict[(src, tgt)] = cmp_dict[(src, tgt)]
            else:
                candidates.append((src, tgt))
        sample_num = int(args.sample * len(has_answer))
        sample_num = min(sample_num, len(candidates))
        candidates = random.sample(candidates, sample_num)
        for src, tgt in candidates:
            new_cmp_dict[(src, tgt)] = cmp_dict[(src, tgt)]
    elif args.ablation:
        print("We are in the ablation mode")
        current_dirname = path.dirname(path.dirname(args.src))
        select_fin = path.join(current_dirname, "select.txt")
        select = False
        if not path.exists(select_fin):
            print("We don't have select file")
        else:
            select_fin = open(select_fin, "r")
            select_funcs = eval(' '.join(select_fin.readlines()))
            select_addrs = set([name2addr_tgt[x] for x in select_funcs])
            select = True

        for (src_addr, tgt_addr), score in cmp_dict.items():
            if (
                fname_tgt[tgt_addr] not in has_answer
                or fname_src[src_addr] not in has_answer
            ):
                continue
            # if the compared function is too small
            if fname_src[src_addr] in ruleout_src or fname_tgt[tgt_addr] in ruleout_tgt:
                continue
            if select and tgt_addr not in select_addrs:
                continue
            new_cmp_dict[(src_addr, tgt_addr)] = score

        if select:
            has_answer = has_answer & select_funcs
        print("Has answer: %d" % len(has_answer))
    else:
        print("Removing ruleout entries...")
        for (src_addr, tgt_addr), score in cmp_dict.items():            
            if (
                fname_tgt[tgt_addr] not in has_answer
                or fname_src[src_addr] not in has_answer
            ):
                continue
            # if the compared function is too small
            if fname_src[src_addr] in ruleout_src or fname_tgt[tgt_addr] in ruleout_tgt:
                continue
            new_cmp_dict[(src_addr, tgt_addr)] = score    

    def dbg_opt_score(s: ComparisonScore):        
        opt_value = 0.5 * (s.value_score) + 0.5 * (
            0.5 * s.unstable_score + 0.5 * s.uu_score
        )        

        libcall_weight = (
            0.5 if s.libcall_size > 20 or s.libcall_size * 10 >= s.value_size else 0.2
        )    
        
        opt_value = np.round(opt_value, decimals=3)
        score = (
            0
            + libcall_weight * s.literal_score
            + 0.5 * s.libcall_score            
            + (1-libcall_weight) * opt_value
        )
        score = np.round(score, decimals=2)
        return score

    # dump cmp file to csv
    print("Dumping to csv...")
    fout = open(args.cmp + ".dumped.csv", "w")
    # fout.write("query_name,candidate_name,gt,score,score,score,score\n")
    
    for (src_addr, tgt_addr), score in new_cmp_dict.items():
        fout.write(
            "%s,%s,%d,%f,%f,%f,%f\n"
            % (
                fname_tgt[tgt_addr],
                fname_src[src_addr],
                fname_src[src_addr] == fname_tgt[tgt_addr],
                dbg_opt_score(score),
                dbg_opt_score(score),
                dbg_opt_score(score),
                dbg_opt_score(score)
            )
        )
    print("Dumped to %s" % (args.cmp + ".dumped.csv"))
