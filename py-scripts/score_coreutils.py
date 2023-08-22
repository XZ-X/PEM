import argparse
import functools
import pickle
import random
from typing import Dict, Tuple
from load_data import load_fname, load_results, load_ruleout
import numpy as np
from compare import ComparisonScore
from os import path

def parse_args(parser=None, add_only=False):
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--src",
        type=str,
        default="qemu-tci-linux-user/benchmarks/eval-dataset/eval-coreutils/coreutils.clang12.O2.elf",
        help="Source binary file",
    )
    parser.add_argument(
        "--tgt",
        type=str,
        default="qemu-tci-linux-user/benchmarks/eval-dataset/eval-coreutils/coreutils.clang12.O3.elf",
        help="Target binary file",
    )
    parser.add_argument(
        "--cmp",
        type=str,        
        default="qemu-tci-linux-user/benchmarks/cmp-ret/eval-dataset/eval-coreutils/coreutils.clang12O2.vs.clang12O3.pkl",
        # default="qemu-tci-linux-user/ablation-study/p400-new-prob-both/cmp-ret.pkl",
        help="Comparison Result file",
    )
    parser.add_argument("--sample", type=float, default=-1, help="Ratio")
    parser.add_argument("--ablation", action="store_true", help="Ablation study")
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
    # zero = 0
    # for current_name in has_answer:
    #     addr_src = name2addr_src[current_name]
    #     addr_tgt = name2addr_tgt[current_name]
    #     src_idx = src_addr2idx[addr_src]
    #     tgt_idx = tgt_addr2idx[addr_tgt]
    #     if score_matrix[tgt_idx, src_idx] == 0:
    #         zero += 1
    correct, error, zero, tie = cal_partial(n=1, allow_tie=False)
    correct_ret = correct
    error_ret = error
    all_func = len(correct) + len(error)
    print("====== %s =====" % name)
    print("PR@1: %f = %d/%d" % (len(correct) / (all_func), len(correct), all_func))
    # dbg_dump = {'correct':correct, 'error':error}
    # fout = open('p400-new-prob-both.dbg.txt', 'w')
    # fout.write(str(dbg_dump))
    # fout.flush()
    # fout.close()
    # print('p400-new-prob-both.dbg.txt')
    # exit(0)
    unq = len(set(correct.keys()) - set(tie.keys()))
    print("UN@1: %f = %d/%d" % (unq / (all_func), unq, all_func))
    tie_cnt = [x[1] for x in tie.values()]
    tie_cnt = np.array(tie_cnt)
    print("TIE: ", end="")
    if tie_cnt.size > 0:
        avg_match = (np.sum(tie_cnt) + unq)/(len(correct))        
        print(np.percentile(tie_cnt, q=[0, 25, 50, 75, 98, 100]))
        print("Avg: %f" % avg_match)
        too_many = np.sum(tie_cnt > 10)
        if too_many > 0:            
            print("Too many: %d/%d" % (too_many, len(tie_cnt)))
            # adjusted precision
            print("AP@1: %f = %d/%d" % ((len(correct) - too_many) / (all_func), len(correct) - too_many, all_func))
    else:
        print("None")    
    print("ZERO: %f = %d/%d" % (len(zero) / len(has_answer), len(zero), len(has_answer)))
    correct, error, zero, tie = cal_partial(n=3, allow_tie=True)
    all_func = len(correct) + len(error)
    print("correct:%d, err:%d, zero:%d"%(len(correct), len(error), len(zero)))
    print("PR@3: %f = %d/%d" % (len(correct) / (all_func), len(correct), all_func))
    correct, error, zero, tie = cal_partial(n=5, allow_tie=True)
    all_func = len(correct) + len(error)
    print("correct:%d, err:%d, zero:%d"%(len(correct), len(error), len(zero)))
    print("PR@5: %f = %d/%d" % (len(correct) / (all_func), len(correct), all_func))
    print("====== END %s =====" % name)
    return correct_ret, error_ret


def dbg_opt_score(s: ComparisonScore):
    # if s.overall_size < 20:
    #     return 0
    opt_value = 0.5 * (s.value_score) + 0.5 * (
        0.5 * s.unstable_score + 0.5 * s.uu_score
    )
    # SCALE_FACTOR = 20
    # W_SMALL = 0.2
    # W_LARGE = 0.5
    # libcall_weight = W_SMALL + (W_LARGE - W_SMALL) * min(
    #     s.libcall_size * SCALE_FACTOR / (s.value_size + 1e-5), 1
    # )
    # literal_weight = W_SMALL + (W_LARGE - W_SMALL) * min(
    #     s.literal_size * SCALE_FACTOR / (s.value_size + 1e-5), 1
    # )

    libcall_weight = (
        0.5 if s.libcall_size > 20 or s.libcall_size * 10 >= s.value_size else 0.2
    )    
    
    opt_value = np.round(opt_value, decimals=3)
    score = (
        0
        + libcall_weight * s.literal_score
        + 0.5 * s.libcall_score
        # + 0.05*s.overall_score
        + (1-libcall_weight) * opt_value
        # + 0.10 * s.value_score
        # + 0.10 * s.unstable_score
    )
    score = np.round(score, decimals=2)
    return score


# main
if __name__ == "__main__":
    print("====== START =====")
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
    has_answer = cmp_fname_src & cmp_fname_tgt - set(ruleout_src) - set(ruleout_tgt)
    print("Has answer: %d" % len(has_answer))
    new_cmp_dict = {}
    if args.sample > 0:
        print("Sampling in 1:%d..." % args.sample)
        candidates = []
        for src, tgt in cmp_dict.keys():
            # if the compared function has no match in source
            # XXX: should we remove src functions that are no match in target?
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
            # if the compared function has no match in source
            # XXX: should we remove src functions that are no match in target?
            if (
                fname_tgt[tgt_addr] not in has_answer
                or fname_src[src_addr] not in has_answer
            ):
                continue
            # if the compared function is too small
            if fname_src[src_addr] in ruleout_src or fname_tgt[tgt_addr] in ruleout_tgt:
                continue
            new_cmp_dict[(src_addr, tgt_addr)] = score
    print("Remaining: %d" % len(new_cmp_dict))
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

    def opt_score(s: ComparisonScore):
        # if s.overall_score == 0:
        #     return 0
        opt_value = 0.7 * (s.value_score) + 0.3 * (
            0.7 * s.unstable_score + 0.3 * s.uu_score
        )
        # opt_value = 0.5 * s.value_score + 0.5 * s.unstable_score
        opt_value = np.round(opt_value, decimals=3)
        score = (
            0
            + 0.50 * s.literal_score
            + 0.50 * s.libcall_score
            # + 0.05*s.overall_score
            + 0.20 * opt_value
            # + 0.10 * s.value_score
            # + 0.10 * s.unstable_score
        )
        score = np.round(score, decimals=2)
        return score

    cal_one_score_partial(score_func=dbg_opt_score, name="OPT")
    # cal_one_score_partial(score_func=opt_score, name="OPT")
    # cal_one_score_partial(score_func=lambda s: s.overall_score, name="OVERALL")
    # cal_one_score_partial(score_func=lambda s: s.libcall_score, name="LIBCALL")
    # cal_one_score_partial(score_func=lambda s: s.literal_score, name="LITERAL")
    # cal_one_score_partial(score_func=lambda s: s.value_score, name="VALUE")
    # cal_one_score_partial(score_func=lambda s: s.unstable_score, name="UNSTABLE")
    # cal_one_score_partial(score_func=lambda s: s.uc_score, name="UC")
    # cal_one_score_partial(score_func=lambda s: s.uu_score, name="UU")
    # cal_one_score_partial(score_func=lambda s: s.vu_score, name="VU")
    # cal_one_score_partial(score_func=lambda s: s.vc_score, name="VC")
    print()


# DBG CODE:

# fout = open('tmp-cmp.csv', 'w')
# for (src_addr, tgt_addr), score in new_cmp_dict.items():
#   src_name = fname_src[src_addr]
#   tgt_name = fname_tgt[tgt_addr]
#   if src_name == tgt_name:
#     gt = 1
#   else:
#     gt = 0
#   fout.write("%s,%s,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"%(
#     src_name, tgt_name, gt, score.overall_score, score.literal_score,
#     score.libcall_score, score.value_score, score.unstable_score,
#     score.vc_score, score.vu_score, score.uc_score, score.uu_score))

# dbg_idx = 12
# tgt_dbg_addr = tgt_idx2addr[dbg_idx]
# sorted_indcies = np.argsort(score_matrix[dbg_idx])
# sorted_indcies = sorted_indcies[::-1]
# dbg_ret = [(fname_src[src_idx2addr[i]], score_matrix[dbg_idx, i], i) for i in sorted_indcies]

# src_dbg_idx = src_idx2addr[98]
# new_cmp_dict[(src_dbg_addr, tgt_dbg_addr)]

# dbg_idx = 20
# scores = score_matrix[dbg_idx]
# addr_scores = [(src_idx2addr[i], s) for i, s in enumerate(scores)]
# fname_scores = [(fname_src[addr], s) for addr, s in addr_scores]


# def dbg_opt_score(s: ComparisonScore):
#     if s.overall_size < 20:
#         return 0
#     opt_value = 0.5 * (s.value_score) + 0.5 * (
#         0.5 * s.unstable_score + 0.5 * s.uu_score
#     )
#     # SCALE_FACTOR = 20
#     # W_SMALL = 0.2
#     # W_LARGE = 0.5
#     # libcall_weight = W_SMALL + (W_LARGE - W_SMALL) * min(
#     #     s.libcall_size * SCALE_FACTOR / (s.value_size + 1e-5), 1
#     # )
#     # literal_weight = W_SMALL + (W_LARGE - W_SMALL) * min(
#     #     s.literal_size * SCALE_FACTOR / (s.value_size + 1e-5), 1
#     # )
#     libcall_weight = 0.6 if s.libcall_size * 10 >= s.value_size else 0.2
#     literal_weight = 0.6 if s.literal_size * 10 >= s.value_size else 0.2

#     opt_value = np.round(opt_value, decimals=3)
#     score = (
#         0
#         + literal_weight * s.literal_score
#         + libcall_weight * s.libcall_score
#         # + 0.05*s.overall_score
#         + 0.60 * opt_value
#         # + 0.10 * s.value_score
#         # + 0.10 * s.unstable_score
#     )
#     score = np.round(score, decimals=2)
#     return score


# cal_one_score_partial(score_func=dbg_opt_score, name="DBG_OPT")

# dbg_dump = {'correct':correct, 'error':error}
# fout = open('p150-prob-both.dbg.txt', 'w')
# fout.write(str(dbg_dump))
# fout.flush()
# fout.close()


# dbg_fin = open('p200-prob-both.dbg.txt', 'r')
# dbg_data_both = eval(' '.join(dbg_fin.readlines()))

# target_name = 'sRGBTransformImage'
# faddr_in_tgt = name2addr_tgt[target_name]
# idx_in_tgt = tgt_addr2idx[faddr_in_tgt]
# ret = [(i,src_idx2addr[i], fname_src[src_idx2addr[i]], score_matrix[idx_in_tgt][i]) for i in  np.argsort(score_matrix[idx_in_tgt])[::-1]]
# tmpout = open('find_cve_%s'%target_name, 'w')
# tmpout.write(str(ret))
# tmpout.flush()
# tmpout.close()

# [(i,r) for i,r in enumerate(ret) if target_name in r[2]]