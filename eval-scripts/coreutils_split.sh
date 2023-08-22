#!/bin/bash -x

mkdir -p cmp-ret/eval-coreutils/c0c3/ 
mkdir -p cmp-ret/eval-coreutils/c2c3/ 
mkdir -p cmp-ret/eval-coreutils/g0g3/ 
mkdir -p cmp-ret/eval-coreutils/g2g3/ 
mkdir -p cmp-ret/eval-coreutils/c0g3/
mkdir -p cmp-ret/eval-coreutils/g0c3/

python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt  eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.clang12O0.vs.clang12O3.pkl --name-map eval-dataset/eval-coreutils/name_map.clang12.pkl --out-dir cmp-ret/eval-coreutils/c0c3/ 
python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O2.elf --tgt  eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.clang12O2.vs.clang12O3.pkl --name-map eval-dataset/eval-coreutils/name_map.clang12.pkl --out-dir cmp-ret/eval-coreutils/c2c3/ 
python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt  eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.gcc94O0.vs.gcc94O3.pkl --name-map eval-dataset/eval-coreutils/name_map.gcc94.pkl --out-dir cmp-ret/eval-coreutils/g0g3/ 
python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O2.elf --tgt  eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.gcc94O2.vs.gcc94O3.pkl --name-map eval-dataset/eval-coreutils/name_map.gcc94.pkl --out-dir cmp-ret/eval-coreutils/g2g3/ 
python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt  eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.clang12O0.vs.gcc94O3.pkl --name-map eval-dataset/eval-coreutils/name_map.clang12.pkl --out-dir cmp-ret/eval-coreutils/c0g3/
python3 py-scripts/split_cmp_coreutils.py --src eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf --tgt  eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp cmp-ret/eval-coreutils/coreutils.gcc94O0.vs.clang12O3.pkl --name-map eval-dataset/eval-coreutils/name_map.gcc94.pkl --out-dir cmp-ret/eval-coreutils/g0c3/