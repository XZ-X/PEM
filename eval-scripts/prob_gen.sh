#!/bin/bash -x

python3 py-scripts/probabilistic_sampling.py --a=3e-2 --fout pem-x64/prob_path.bin
python3 py-scripts/generate_random_memory.py pem-x64/mem.pmem 