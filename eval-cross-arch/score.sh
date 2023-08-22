#!/bin/bash -x

# SAMPLE="--sample 100"
# FNAME=".100"

SAMPLE=""
FNAME=".all"


python3 py-scripts/score.py $SAMPLE --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/cross-arch/coreutils.gcc94O0.vs.gcc94O3.pkl > score-ret/cross-arch/coreutils.gcc94O0.vs.gcc94O3.score$FNAME.txt 2>&1
python3 py-scripts/score.py $SAMPLE --src aarch64-coreutils/coreutils.aarch64.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --cmp cmp-ret/cross-arch/coreutils.aarch64O0.vs.aarch64O3.pkl > score-ret/cross-arch/coreutils.aarch64O0.vs.aarch64O3.score$FNAME.txt 2>&1
python3 py-scripts/score.py $SAMPLE --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --cmp cmp-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O3.pkl > score-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O3.score$FNAME.txt 2>&1
python3 py-scripts/score.py $SAMPLE --src aarch64-coreutils/coreutils.aarch64.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/cross-arch/coreutils.aarch64O0.vs.gcc94O3.pkl > score-ret/cross-arch/coreutils.aarch64O0.vs.gcc94O3.score$FNAME.txt 2>&1
python3 py-scripts/score.py $SAMPLE --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O0.elf --cmp cmp-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O0.pkl > score-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O0.score$FNAME.txt 2>&1
python3 py-scripts/score.py $SAMPLE --src eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --cmp cmp-ret/cross-arch/coreutils.gcc94O3.vs.aarch64O3.pkl > score-ret/cross-arch/coreutils.gcc94O3.vs.aarch64O3.score$FNAME.txt 2>&1

