#!/bin/bash

python3 py-scripts/dump_csv.py  --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.gcc94O0.vs.gcc94O3.pkl
python3 py-scripts/dump_csv.py  --src eval-dataset/eval-how-help/find47.locate.O0.elf --tgt eval-dataset/eval-how-help/find47.locate.O3.elf --cmp cmp-ret/eval-how-help/find47.locate.O0vsO3.pkl
python3 py-scripts/dump_csv.py  --src eval-dataset/eval-trex/libmagick7.so.O0.elf --tgt eval-dataset/eval-trex/libmagick7.so.O3.elf --cmp cmp-ret/eval-trex/libmagick7.so.O0vsO3.pkl
python3 py-scripts/dump_csv.py  --src eval-dataset/eval-trex/libz.so.O0.elf --tgt eval-dataset/eval-trex/libz.so.O3.elf --cmp cmp-ret/eval-trex/libz.so.O0vsO3.pkl
python3 py-scripts/dump_csv.py --src eval-dataset/eval-trex/openssl101f.O0.elf --tgt eval-dataset/eval-trex/openssl101f.O3.elf --cmp cmp-ret/eval-trex/openssl101f.O0vsO3.pkl