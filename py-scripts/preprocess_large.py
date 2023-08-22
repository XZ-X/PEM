import argparse
from multiprocessing import Pool
import pickle
from load_data import load_fname, load_results
import numpy as np
import sem_features_pb2
from functools import partial
from preprocess import FunctionEntry
import preprocess
import load_data

def preprocess_one(overall, i, addr):
    data = bin_src[addr]
    entry = FunctionEntry(data)
    print("\r preprocess: %d/%d" % (i, overall), end="")
    ret = (addr, entry)
    # bin_src[addr] = None
    # del data
    return ret

def main():
    global bin_src
    args = preprocess.parse_args()
    print(args)
    files = load_data.get_results_filename(args.src)
    preprocessed_all = {}
    N = 3
    begin = 0
    while begin < len(files):
        currenet_files = files[begin:begin+N]
        begin += N
        bin_src = load_data.load_results_large(currenet_files)    
        addr_src = list(bin_src.keys())    
        len_src = len(addr_src)
    
        preprocess_partial = partial(preprocess_one, len_src)
        pool = Pool(processes=6)
        preprocessed = pool.starmap(
            preprocess_partial, [(i, addr) for i, addr in enumerate(addr_src)]
        )
        # preprocessed = [preprocess_partial(i, addr) for i, addr in enumerate(addr_src)]
        for p in preprocessed:
            preprocessed_all[p[0]] = p[1]
        del bin_src
        print("\n Preprocessed %d/%d"%(begin, len(files)))
    # [preprocess_partial(i, bin_src[addr]) for i,addr in enumerate(addr_src)]
    # save preprocess to pickle
    print("\nSaving...")    
    with open(args.src + ".preprocess.pkl", "wb") as f:
        pickle.dump(preprocessed_all, f)

    # preprocessed = [preprocess(len_src, i, bin_src[addr]) for i, addr in enumerate(addr_src)]
    print("Done")


if __name__ == "__main__":
    main()
