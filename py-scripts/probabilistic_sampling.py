import numpy as np
from matplotlib import pyplot as plt
import argparse
from scipy.stats import beta
import struct


def parse_args(parser=None, add_only=False):
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--fout",
        type=str,        
        default="",
        help="Source binary file",
    )
    parser.add_argument(
        "--a",
        type=float,        
        default=3e-2,
        help="Shape param",
    )
    parser.add_argument(
        "--uniform",
        action="store_true",
    )
    parser.add_argument(
        "--determ",
        action="store_true",
    )
    parser.add_argument(
        "--both",
        action="store_true",
    )
    if add_only:
        return parser
    else:
        return parser.parse_args()


def main(args):
    if args.uniform:
        print("Uniform sampling")
        normalize = np.random.uniform(size=20000)
        a = 0
    elif args.determ:
        print("Deterministic sampling")
        normalize = np.ones(20000)
        a = 233
    elif args.both:
        normalize = np.array([i%2 for i in range(20000)])
        a=255
    else:
        a = args.a
        vars = beta.rvs(a, a, size=20000)
        normalize = np.clip(np.clip(vars - 5e-2, 0, 1) * (1 / 0.9), 0, 1)
    plt.close()
    plt.hist(normalize, label="%.3f"%a)
    plt.legend()
    plt.savefig('smpl-%.3f.png'%a)
    bin = struct.pack("f" * len(normalize), *normalize)
    fout = open(args.fout, "wb")
    fout.write(bin)
    fout.close()
    print("Done")


# main
if __name__ == "__main__":
    # parse args
    args = parse_args()
    print(args)
    main(args)
