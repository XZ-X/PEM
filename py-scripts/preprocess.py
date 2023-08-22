import argparse
from multiprocessing import Pool
import pickle
from load_data import load_fname, load_results
import numpy as np
import sem_features_pb2
from functools import partial


class FunctionEntry:
    def _process_values(self, r0, r1):
        r0_value = {}
        r0_unstable = {}
        for p in r0.paths:
            for v in p.value:
                if v not in r0_value:
                    r0_value[v] = 1
                else:
                    r0_value[v] += 1
            for v in p.unstable:
                if v not in r0_unstable:
                    r0_unstable[v] = 1
                else:
                    r0_unstable[v] += 1
        r1_value = {}
        r1_unstable = {}
        for p in r1.paths:
            for v in p.value:
                if v not in r1_value:
                    r1_value[v] = 1
                else:
                    r1_value[v] += 1
            for v in p.unstable:
                if v not in r1_unstable:
                    r1_unstable[v] = 1
                else:
                    r1_unstable[v] += 1
        self.vc = {}
        self.vu = {}
        for k, v in r0_value.items():
            if k in r1_value:
                self.vc[k] = v
            else:
                self.vu[k] = v
        for k, v in r1_value.items():
            if k not in r0_value:
                self.vu[k] = v
        self.uc = {}
        self.uu = {}
        for k, v in r0_unstable.items():
            if k in r1_unstable:
                self.uc[k] = v
            else:
                self.uu[k] = v
        for k, v in r1_unstable.items():
            if k not in r0_unstable:
                self.uu[k] = v
        self.value = {}
        self.unstable = {}
        for k, v in r0_value.items():
            if k not in self.value:
                self.value[k] = v
            else:
                self.value[k] += v
        for k, v in r1_value.items():
            if k not in self.value:
                self.value[k] = v
            else:
                self.value[k] += v
        for k, v in r0_unstable.items():
            if k not in self.unstable:
                self.unstable[k] = v
            else:
                self.unstable[k] += v
        for k, v in r1_unstable.items():
            if k not in self.unstable:
                self.unstable[k] = v
            else:
                self.unstable[k] += v
        self.r0_value = r0_value
        self.r1_value = r1_value
        self.r0_unstable = r0_unstable
        self.r1_unstable = r1_unstable
    def shrink(self):
        # allow a max of 50000 values
        self.overall = dict(sorted(self.overall.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.value = dict(sorted(self.value.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.unstable = dict(sorted(self.unstable.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.vc = dict(sorted(self.vc.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.vu = dict(sorted(self.vu.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.uc = dict(sorted(self.uc.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.uu = dict(sorted(self.uu.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.r0_value = dict(sorted(self.r0_value.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.r1_value = dict(sorted(self.r1_value.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.r0_unstable = dict(sorted(self.r0_unstable.items(), key=lambda item: item[1], reverse=True)[:50000])
        self.r1_unstable = dict(sorted(self.r1_unstable.items(), key=lambda item: item[1], reverse=True)[:50000])


    def __init__(self, function_entry):
        # let's not save this for memory efficiency
        # self.function_entry = function_entry
        libcall = {}
        literal = {}
        overall = {}
        for r in function_entry.rounds:
            for p in r.paths:
                for v in p.libcall:
                    if v not in libcall:
                        libcall[v] = 1
                    else:
                        libcall[v] += 1
                for v in p.literal:
                    if v not in literal:
                        literal[v] = 1
                    else:
                        literal[v] += 1
                for v in p.overall:
                    if v.type == sem_features_pb2.DataType.NUMBER:
                        val = v.num
                    elif v.type == sem_features_pb2.DataType.STRING:
                        val = v.str
                    if val not in overall:
                        overall[val] = 1
                    else:
                        overall[val] += 1
        self.libcall = libcall
        self.literal = literal
        self.overall = overall

        self._process_values(function_entry.rounds[0], function_entry.rounds[1])


def parse_args(parser=None, add_only=False):
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--src",
        type=str,        
        default="",
        help="Source binary file",
    )
    if add_only:
        return parser
    else:
        return parser.parse_args()


def preprocess(overall, i, addr):
    data = bin_src[addr]
    entry = FunctionEntry(data)
    print("\r preprocess: %d/%d" % (i, overall), end="")
    ret = (addr, entry)
    bin_src[addr] = None
    del data
    return ret


def main():
    global bin_src
    args = parse_args()
    print(args)
    bin_src = load_results(args.src)
    # bin_tgt = load_results(args.tgt + ".pb")
    addr_src = list(bin_src.keys())
    # addr_tgt = list(bin_tgt.keys())
    len_src = len(addr_src)
    # len_tgt = len(addr_tgt)
    # distance = np.array([len_src, len_tgt], dtype=np.float32)
    preprocess_partial = partial(preprocess, len_src)
    pool = Pool(processes=24)
    preprocessed = pool.starmap(
        preprocess_partial, [(i, addr) for i, addr in enumerate(addr_src)]
    )
    # [preprocess_partial(i, bin_src[addr]) for i,addr in enumerate(addr_src)]
    # save preprocess to pickle
    print("\nSaving...")
    preprocessed = dict(preprocessed)
    with open(args.src + ".preprocess.pkl", "wb") as f:
        pickle.dump(preprocessed, f)

    # preprocessed = [preprocess(len_src, i, bin_src[addr]) for i, addr in enumerate(addr_src)]
    print()


if __name__ == "__main__":
    main()
