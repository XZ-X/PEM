#!/bin/bash -x

pem-x64/qemu-x86_64 eval-dataset/eval-coreutils/coreutils.gcc94.O0.elf -M pem-x64/mem.pmem -P pem-x64/prob_path.bin
pem-x64/qemu-x86_64 eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf -M pem-x64/mem.pmem -P pem-x64/prob_path.bin
pem-aarch64/qemu-aarch64 aarch64-coreutils/coreutils.aarch64.O0.elf -M pem-aarch64/mem.pmem -P pem-aarch64/prob_path.bin
pem-aarch64/qemu-aarch64 aarch64-coreutils/coreutils.aarch64.O3.elf -M pem-aarch64/mem.pmem -P pem-aarch64/prob_path.bin