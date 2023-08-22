#!/bin/bash -x

python3 py-scripts/preprocess_large.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf
python3 py-scripts/preprocess_large.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf
python3 py-scripts/preprocess_large.py --src aarch64-coreutils/coreutils.aarch64.O0.elf
python3 py-scripts/preprocess_large.py --src aarch64-coreutils/coreutils.aarch64.O3.elf
