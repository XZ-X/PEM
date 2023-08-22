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

        default="",

        help="Source binary file",
    )
    parser.add_argument(
        "--tgt",
        type=str,

        default="",

        help="Target binary file",
    )
    parser.add_argument(
        "--cmp",
        type=str,

        default="",

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


def calculate_score(
    score_matrix, fname_src, fname_tgt, tgt_idx2addr, src_idx2addr, n=1, allow_tie=True
):
    if not allow_tie:
        assert n == 1
    score_idx_sorted = np.argsort(score_matrix, axis=-1)
    correct = {}
    error = {}
    zero = {}
    tie = {}
    for tgt_idx, tgt_src_list in enumerate(score_idx_sorted):
        current_max_score = 999999
        current_max_counter = 0
        found = False
        tgt_addr = tgt_idx2addr[tgt_idx]
        last = tgt_src_list[-1]
        second_last = tgt_src_list[-2]
        last_score = score_matrix[tgt_idx, last]
        second_last_score = score_matrix[tgt_idx, second_last]
        if last_score == 0:
            zero[tgt_addr] = fname_tgt[tgt_addr]
            continue
        for src_idx in tgt_src_list[::-1]:
            if score_matrix[tgt_idx, src_idx] < current_max_score:
                current_max_counter += 1
                current_max_score = score_matrix[tgt_idx, src_idx]
            if current_max_counter > n:
                break
            src_addr = src_idx2addr[src_idx]
            if fname_src[src_addr] == fname_tgt[tgt_addr]:
                found = True
                break
        if found:
            correct[tgt_addr] = fname_tgt[tgt_addr]
            if not allow_tie and last_score == second_last_score:
                cnt = 0
                for src_idx in tgt_src_list[::-1]:
                    if score_matrix[tgt_idx, src_idx] < last_score:
                        break
                    cnt += 1
                tie[tgt_addr] = (fname_tgt[tgt_addr], cnt)
        else:
            error[tgt_addr] = fname_tgt[tgt_addr]
    return correct, error, zero, tie


def calculate_one_score(
    cmp_dict, src_addr2idx, tgt_addr2idx, src_idx2addr, tgt_idx2addr, score_func, name
):
    score_matrix = create_score_matrix(
        cmp_dict=cmp_dict,
        src_addr2idx=src_addr2idx,
        tgt_addr2idx=tgt_addr2idx,
        score_func=score_func,
    )
    cal_partial = functools.partial(
        calculate_score,
        score_matrix=score_matrix,
        fname_src=fname_src,
        fname_tgt=fname_tgt,
        tgt_idx2addr=tgt_idx2addr,
        src_idx2addr=src_idx2addr,
    )

    correct, error, zero, tie = cal_partial(n=1, allow_tie=False)
    correct_ret = correct
    error_ret = error
    all_func = len(correct) + len(error)
    print("====== %s =====" % name)
    
    print_pr = True
    tie_cnt = [x[1] for x in tie.values()]
    tie_cnt = np.array(tie_cnt)
    if tie_cnt.size > 0:        
        too_many = np.sum(tie_cnt > 10)
        if too_many > 0:            
            print_pr = False
            # count too many tie functions as wrong
            print("PR@1: %f = %d/%d" % ((len(correct) - too_many) /
                  (all_func), len(correct) - too_many, all_func))        
    if print_pr:
        print("PR@1: %f = %d/%d" %
          (len(correct) / (all_func), len(correct), all_func))    
    correct, error, zero, tie = cal_partial(n=3, allow_tie=True)
    all_func = len(correct) + len(error)    
    print("PR@3: %f = %d/%d" % (len(correct) / (all_func), len(correct), all_func))
    correct, error, zero, tie = cal_partial(n=5, allow_tie=True)
    all_func = len(correct) + len(error)    
    print("PR@5: %f = %d/%d" % (len(correct) / (all_func), len(correct), all_func))
    return correct_ret, error_ret



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
    # create idx maps
    src_addr2idx, src_idx2addr, tgt_addr2idx, tgt_idx2addr = create_idx_maps(
        new_cmp_dict
    )
    cal_one_score_partial = functools.partial(
        calculate_one_score,
        cmp_dict=new_cmp_dict,
        src_addr2idx=src_addr2idx,
        tgt_addr2idx=tgt_addr2idx,
        src_idx2addr=src_idx2addr,
        tgt_idx2addr=tgt_idx2addr,
    )

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


    cal_one_score_partial(score_func=dbg_opt_score, name="SIM_SCORE")
    
    print()
