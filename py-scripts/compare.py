import argparse
from multiprocessing import Pool
import pickle
from load_data import load_fname, load_results
import numpy as np
import sem_features_pb2
from functools import partial
from preprocess import FunctionEntry


class ComparisonScore:
    def _compare_two_multiset(self, occ0, occ1, threshold=10):
        # s0 = set(occ0.keys())
        # s1 = set(occ1.keys())
        s0 = set([k for k, v in occ0.items() if v > threshold])
        s1 = set([k for k, v in occ1.items() if v > threshold])
        inter = s0 & s1
        s0_only = s0 - inter
        s1_only = s1 - inter
        # s0_err = len([s for s in s0_only if occ0[s] > threshold])
        # s1_err = len([s for s in s1_only if occ1[s] > threshold])
        # all_size = len(inter) + s0_err + s1_err
        all_size = len(inter) + len(s0_only) + len(s1_only)
        if all_size == 0:
            return (0, 0)
        return (len(inter) / all_size, all_size)

    def __init__(self, entry0, entry1):
        THR = 0
        self.libcall_score, self.libcall_size = self._compare_two_multiset(
            entry0.libcall, entry1.libcall, THR
        )        
        self.literal_score, self.literal_size = self._compare_two_multiset(
            entry0.literal, entry1.literal, THR
        )                
        self.overall_score, self.overall_size = self._compare_two_multiset(
            entry0.overall, entry1.overall, THR
        )                
        self.value_score, self.value_size = self._compare_two_multiset(
            entry0.value, entry1.value, THR
        )                
        self.unstable_score, self.unstable_size = self._compare_two_multiset(
            entry0.unstable, entry1.unstable, THR
        )                
        self.vc_score, self.vc_size = self._compare_two_multiset(entry0.vc, entry1.vc, THR)                
        self.vu_score, self.vu_size = self._compare_two_multiset(entry0.vu, entry1.vu, THR)                
        self.uc_score, self.uc_size = self._compare_two_multiset(entry0.uc, entry1.uc, THR)                
        self.uu_score, self.uu_size = self._compare_two_multiset(entry0.uu, entry1.uu, THR)                


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
        "--fout",
        type=str,
        default="",
        help="Result file",
    )
    if add_only:
        return parser
    else:
        return parser.parse_args()


def compare(i):
    src_addr = addr_src[i]
    src_entry = bin_src[src_addr]
    ret = []
    for k, tgt_addr in enumerate(addr_tgt):
        tgt_entry = bin_tgt[tgt_addr]
        ret.append((i, k, ComparisonScore(src_entry, tgt_entry)))

    print("\r%4d/%4d" % (i, len(addr_src)), end="")
    return ret


def main():
    args = parse_args()
    print(args)
    global bin_src, addr_src, bin_tgt, addr_tgt
    f_src = open(args.src + ".preprocess.pkl", "rb")
    bin_src = pickle.load(f_src)
    f_src.close()
    f_tgt = open(args.tgt + ".preprocess.pkl", "rb")
    bin_tgt = pickle.load(f_tgt)
    f_tgt.close()
    # for v in bin_src.values():
    #     v.shrink()
    # for v in bin_tgt.values():
    #     v.shrink()

    addr_src = list(bin_src.keys())
    addr_tgt = list(bin_tgt.keys())
    len_src = len(addr_src)
    len_tgt = len(addr_tgt)
    # compare_partial = partial(compare, addr_src, addr_tgt, bin_src, bin_tgt)
    print("Creating pool...")
    pool = Pool(processes=4)
    print("Creating mapping...")
    ret = pool.map(
        compare,
        [
            i
            for i, _ in enumerate(addr_src)
            # for k, addr_t in enumerate(addr_tgt)
        ],
    )
    # ret = [compare_partial(i) for i, _ in enumerate(addr_src)]
    compare_result = {}
    for results in ret:
        for item in results:
            src_addr_idx = item[0]
            tgt_addr_idx = item[1]
            src_addr = addr_src[src_addr_idx]
            tgt_addr = addr_tgt[tgt_addr_idx]
            score = item[2]
            compare_result[(src_addr, tgt_addr)] = score

    print("Save")
    with open(args.fout, "wb") as f:
        pickle.dump(compare_result, f)


if __name__ == "__main__":
    main()


