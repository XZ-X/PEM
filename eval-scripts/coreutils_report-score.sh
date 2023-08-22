#!/bin/bash

echo "clangO0 vs clangO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0c3-pr1.txt
echo "clangO0 vs clangO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0c3-pr3.txt
echo "clangO0 vs clangO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0c3-pr5.txt

echo "clangO2 vs clangO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c2c3-pr1.txt
echo "clangO2 vs clangO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c2c3-pr3.txt
echo "clangO2 vs clangO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c2c3-pr5.txt

echo "gccO0 vs gccO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0g3-pr1.txt
echo "gccO0 vs gccO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0g3-pr3.txt
echo "gccO0 vs gccO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0g3-pr5.txt

echo "gccO2 vs gccO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g2g3-pr1.txt
echo "gccO2 vs gccO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g2g3-pr3.txt
echo "gccO2 vs gccO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g2g3-pr5.txt

echo "clangO0 vs gccO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0g3-pr1.txt
echo "clangO0 vs gccO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0g3-pr3.txt
echo "clangO0 vs gccO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/c0g3-pr5.txt

echo "gccO0 vs clangO3 PR1"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0c3-pr1.txt
echo "gccO0 vs clangO3 PR3"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0c3-pr3.txt
echo "gccO0 vs clangO3 PR5"
python3 py-scripts/cal_score_avg_coreutils.py --in-score cmp-ret/eval-coreutils/g0c3-pr5.txt



