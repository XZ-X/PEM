import argparse



def parse_args(parser=None, add_only=False):
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--in-score",
        type=str,
        default="coreutils-score-ret/clang12O2.vs.clang12O3-pr1.txt",
        help="Coreutils dir",
    )    
    if add_only:
        return parser
    else:
        return parser.parse_args()



# main
if __name__ == "__main__":    
    # parse args
    args = parse_args()
    # print(args)
    scores, ncorrect, ntotal = [], 0, 0
    for line in open(args.in_score, 'r').readlines():
      line = line.strip()
      fields = line.split(',')
      if len(fields) != 3:
        print("Invalid line: %s" % line)
        exit(1)
      score = float(fields[0])
      ncorrect += int(fields[1])
      ntotal += int(fields[2])
      scores.append(score)
    print("ncorrect: %d, ntotal: %d, avg: %f" % (ncorrect, ntotal, ncorrect/ntotal))
    # print("avg score: %f" % (sum(scores)/len(scores)))