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
    if add_only:
        return parser
    else:
        return parser.parse_args()



if __name__ == "__main__":
  args = parse_args()
  src_base_name = path.basename(args.src)
  ruleout_src = load_ruleout(args.src + ".ruleout")
  fin = " ".join(open(args.src + ".cfg").readlines())
  fname = " ".join(open(args.src + ".fname").readlines())
  fname: Dict = eval(fname)
  program: Dict = eval(fin)
  func_num = len(program)
  selected_num = 0
  blk_num = 0
  for func_entry, func in program.items():
    if fname[func_entry] in ruleout_src:
      continue
    bb_number = len(func["bbs"])
    selected_num += 1
    blk_num += bb_number
  print("%s: func:%d, selected:%d, blk:%d" % (src_base_name, func_num, selected_num, blk_num))
