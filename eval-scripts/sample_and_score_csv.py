import pandas as pd
import sys
import random
import numpy as np
from tqdm import tqdm


def precision_at_n(src_funcs, tgt_funcs, scores, n=1, single=False):
    correct = 0
    max_ans = 0
    zeros_cnt = 0
    func_matches = {}
    correct_funcs = set()
    wrong_funcs = set()
    zero_funcs = set()
    possible_funcs = set()
    for i, src in enumerate(src_funcs):
        tgt = tgt_funcs[i]
        s = scores[i]
        if s == 0 and src == tgt:
            zeros_cnt += 1
            possible_funcs.add(src)
        elif src == tgt:
            max_ans += 1
            possible_funcs.add(src)
        if src not in func_matches:
            func_matches[src] = []

        func_matches[src].append((s, tgt))

    for f, matches in func_matches.items():
        matches.sort(key=lambda x: x[0], reverse=True)

    for f, matches in func_matches.items():
        highest_score = matches[0][0]
        if highest_score == 0 and f in possible_funcs:
            zero_funcs.add(f)
            continue
        done = False
        if not single:
            for current_s, current_f in matches:
                if current_s < highest_score:
                    break
                if current_f == f:
                    correct_funcs.add(f)
                    correct += 1
                    done = True
                    break
        else:
            #     single, rule out not single
            for i, (current_s, _) in enumerate(matches):
                if current_s < highest_score:
                    break
                if current_s == highest_score and i > 0:
                    done = True
                    break

        if not done:
            max_possible = min(n - 1, len(matches) - 1)
            score_max = matches[max_possible][0]
            for i in range(0, len(matches)):
                if matches[i][0] < score_max:
                    break
                if matches[i][1] == f:
                    correct_funcs.add(f)
                    correct += 1
                    done = True
                    break

        if not done and f in possible_funcs:
            wrong_funcs.add(f)

    return correct / max_ans, (correct_funcs, wrong_funcs, zero_funcs)


def split_pos_and_neg(data):
    pos = []
    neg = []
    for i in data:
        if i[GT] == 1:
            pos.append(i)
        else:
            neg.append(i)
    return pos, neg


def sample(data, number):
    random.shuffle(data)
    return data[:number]
    # ret = []
    # ret.extend(pos)
    # random.shuffle(neg)
    # ret.extend(neg[:ratio*len(pos)])
    # return ret


def batch_sample_with_ratio(pos, neg, fout):
    if batch:
        ratio_list = [1, 5, 10, 20, 50, 100, 500, 1000, 99999]
        len_pos = len(pos)
        random.shuffle(neg)
        ret = []
        for ratio in tqdm(ratio_list):
            neg_num = ratio * len_pos
            to_test = np.array(pos + neg[:neg_num])
            pr, _ = precision_at_n(
                src_funcs=to_test[:, TGT],
                tgt_funcs=to_test[:, SRC],
                scores=to_test[:, SCORE],
            )
            ret.append(pr)
        for score, ratio in zip(ret, ratio_list):
            fout.write("PR@1: %.3f;\t\tMax Possible Negative Sample Ratio:\t 1:%d\n"%(score, ratio))
        fout.write("\n")
        print("Done. Results are written to %s"%fout.name)
    else:
        to_test = np.array(pos + neg)
        pr, _ = precision_at_n(
            src_funcs=to_test[:, TGT],
            tgt_funcs=to_test[:, SRC],
            scores=to_test[:, SCORE],
        )
        fout.write("PR@1: %.3f;\t\tInclude All Samples\n"%(pr))
        print("Done. Results are written to %s"%fout.name)

if __name__ == "__main__":

    SCORE = 3
    GT = 2
    SRC = 0
    TGT = 1
    batch = True
    if len(sys.argv) < 2:
        print("Specify input csv file")
        exit(-1)
    if len(sys.argv) >=3 and sys.argv[2] == "single":
        batch = False

    data = pd.read_csv(sys.argv[1], header=None).fillna(0).to_numpy()
    pos, neg = split_pos_and_neg(data)

    fout = open(sys.argv[1]+".sret", 'w')        
    batch_sample_with_ratio(pos, neg, fout)
    fout.flush()
    fout.close()    
