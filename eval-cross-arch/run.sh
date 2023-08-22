#!/bin/bash

mkdir -p cmp-ret/cross-arch
mkdir -p score-ret/cross-arch
eval-cross-arch/prob_gen.sh
eval-cross-arch/sample.sh
eval-cross-arch/preprocess.sh
eval-cross-arch/compare.sh
eval-cross-arch/score.sh