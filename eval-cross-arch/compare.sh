#!/bin/bash -x

python3 py-scripts/compare.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --fout cmp-ret/cross-arch/coreutils.gcc94O0.vs.gcc94O3.pkl
python3 py-scripts/compare.py --src aarch64-coreutils/coreutils.aarch64.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --fout cmp-ret/cross-arch/coreutils.aarch64O0.vs.aarch64O3.pkl
python3 py-scripts/compare.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --fout cmp-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O3.pkl
python3 py-scripts/compare.py --src aarch64-coreutils/coreutils.aarch64.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --fout cmp-ret/cross-arch/coreutils.aarch64O0.vs.gcc94O3.pkl
python3 py-scripts/compare.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt aarch64-coreutils/coreutils.aarch64.O0.elf --fout cmp-ret/cross-arch/coreutils.gcc94O0.vs.aarch64O0.pkl
python3 py-scripts/compare.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --tgt aarch64-coreutils/coreutils.aarch64.O3.elf --fout cmp-ret/cross-arch/coreutils.gcc94O3.vs.aarch64O3.pkl


