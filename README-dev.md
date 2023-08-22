# README-dev

This repository contains the source code of PEM.
PEM is a dynamic-analysis-based tool for detecting similar functions across different binary programs.
Specifically, given a query binary function, PEM searches for the most similar function in a target binary.

To detect the similar function among a large number of candidate functions,
PEM leverages dynamic analysis to collect the execution information 
(e.g., values of registers and memory) from both the query function and the candidate functions.
PEM then compares the execution information of the query function with each candidate function.
The candidate function with the most similar execution information is considered the most similar function.

In this file, we first introduce the structure of the repository.
Then, we illustrate the PEM's core components from the perspective of the code.
Finally, we provide the instructions for compiling PEM from the source code.

Additionally, we upload a runnable demo of PEM on Docker Hub.
Please refer to `README-docker.md` in this repository for more details.

## Introduction to the Repository

The repository is organized as follow:

```
├── README-docker.md
├── README-dev.md
├── eval-scripts
├── eval-cross-arch
├── py-scripts
└── qemu-6.2.0
```

`README-dev.md` is the file you are reading now.

`README-docker.md` contains the instructions for running PEM with a pre-configured Docker image.

`eval-scripts` and `eval-cross-arch` contains the scripts for running PEM on the evaluation dataset. They can be used as templates for running PEM on other datasets.

`py-scripts` contains the python scripts used by PEM.

`qemu-6.2.0` contains the source code of PEM. It is based on QEMU 6.2.0.


## Introduction to Core Components

We briefly go through the core components of PEM.

### Pre-Analysis

PEM leverages IDA and Ghidra to pre-analyze the input binary files.
The pre-analysis aims to extract the control flow graph (CFG),
library functions, and string constants from the binary files.
Please refer to `py-scripts/README.md` for more details.

### Probabilistic Execution

The probabilistic execution engine takes as input a binary file (and a set of static information, e.g., the CFG).
It iteratively samples each function in the binary file and stores the observable values in a file.
In this part, we mainly illustrate on three core parts of the engine:
(1) the interpreter, (2) the probabilistic path sampling algorithm, and (3) the probabilistic memory model.

### Interpretation Rules

The front-end of QEMU translates `x86` instructions into an intermediate representation(IR)
called `Tiny Code(TC)`.
The form of `TC` is similar to our illustrative language in Fig. 5.
They are defined in `qemu-6.2.0/linux-user/pr_emulator/inst.txt`.

The interpretation rules in Fig. 7 of the paper are defined in 
`qemu-6.2.0/linux-user/pr_emulator/emulator.hh`.
They are implemented in 
```
qemu-6.2.0/linux-user/pr_emulator/emulator.cc
qemu-6.2.0/linux-user/pr_emulator/emu_flow.cc
qemu-6.2.0/linux-user/pr_emulator/emu_value.cc
qemu-6.2.0/linux-user/pr_emulator/emu_libcall.cc
qemu-6.2.0/linux-user/pr_emulator/emu_operation.cc
```

The main function of the interpreter is defined at line 144 of `.../emulator.cc`.
It iteratively interpret each instruction inside a basic block (lines 167-204).
When the interpreter reaches the end of a basic block, it calls the function
`switchToNewBasicBlock` to decide the address of the next basic block.
Specifically, the statement at line 268 shows how the path sampling algorithm interacts with the interpreter.
```c
eip = sampler->guideBranch(currentEndPC - load_info.load_bias, EMU_PC - load_info.load_bias, ctx);
```
As shown in the rule `JccGT` of Fig. 7 of the paper,
the `sampler` use the program counter of current basic block (the first parameter)
to decide whether to flip the current branch or not.
If the branch is flipped, the `sampler` returns the address of the target basic block.
Otherwise, it returns the address of the original branch outcome (the second parameter).


### Probabilistic Path Sampling

PEM generates 20000 random numbers following a Beta distribution before the probabilistic execution.
At each step, PEM sorts all the predicates according to their dynamic selectivity.
Then it randomly picks a number from the pre-generated random numbers.
The number is further used to select the predicate to be flipped.

First, a list of random number is generated via `py-scripts/probabilistic_sampling.py`.
The script samples 20000 random numbers following a Beta distribution with parameter a=0.003.
Then the random numbers are normalized from $(0,1)$ to $[0,1]$.

During probabilistic execution, these random numbers are loaded into an array named `randomSamples`.
The main part of the path sampling algorithm is defined in
`qemu-6.2.0/linux-user/pr_sampler/selectivity_path_planner.cc`.
The probabilistic sampling is implemented from line 205 to line 209.

PEM first randomly select a sampled number.
```
auto randNum = rand();  
auto factor = randomSamples[(randNum % SAMPLE_SIZE)];
```
Since the sampled numbers in `randomSamples` follows a Beta distribution and `randNumber` is uniformly distributed, the number
`factor` follows a Beta distribution.


Then the index of the selected predicate is calculated by
```
auto selectedIdx = (int)(factor * (selectivityAllCandidates.size() - 1));
```

At lines 229-232, PEM selects the predicate to flip with the calculated index.
```
auto it = selectivityAllCandidates.begin();
for (auto i = 0; i < selectedIdx; i++) {
  it++;
}
```
Note that `selectivityAllCandidates` is an ordered set of predicates.
The traversal begins from the predicate with the smallest selectivity.

### Probabilistic Memory Model

Similar to the probabilistic path sampling algorithm, 
the initial values of the probabilistic memory model are
pre-generated by `py-scripts/generate_random_memory.py`.
It simply generates 4M random bytes and stores them in a file.

During probabilistic execution, the initial values are mapped to a memory region named `randomMem`.
The main part of the memory model is defined in
`qemu-6.2.0/linux-user/pr_emulator/emu_value.cc`.
The probabilistic memory model is implemented from line 79 to line 82.

```
auto page = new uint8_t[PAGE_SIZE];
...
auto probAddrBase = (addrBase & (PROB_MEM_SIZE - 1)) + randomMem;
memcpy(page, (const void *)probAddrBase, PAGE_SIZE);
```
Whenever an invalid memory access is detected,
PEM inserts a new page to the memory states maintained by PEM (`page`). 
The values are from the probabilisitc memory model (`probAddrBase`) .

### Post-Analysis

For an input binary, the results of the probabilistic execution engine are sequences 
of values yield during execution.
The preprocessing step aggregates the sequences into the form of multi-set.
After that, PEM compares the multi-sets of the query function and the candidate functions.
The comparison results are used to calculate the similarity score.
Please refer to `py-scripts/README.md` for more details.

## Compiling PEM from Source Code

### Pre-Analysis Scripts

The pre-analysis scripts are implemented as IDA scripts in Python 2.
They are tested on IDA Pro 7.0. Please see `requirements.txt` for the required python packages.

### Probabilistic Execution Engine

- PEM leverages Protobuf to serialize the execution information.
The protobuf compiler is required to compile the protobuf files.
Please refer to the [official document](https://github.com/protocolbuffers/protobuf)
of Protobuf to install the protobuf compiler.

- PEM uses Boost in the implementation. 
It is tested on [Boost 1.79.0](https://www.boost.org/users/history/version_1_79_0.html).

- PEM is built on top of QEMU 6.2.0.
Please refer to the [official document](https://wiki.qemu.org/Hosts/Linux)
of QEMU to build the probabilistic execution engine of PEM.

- PEM is tested on Ubuntu 20.04.5 LTS.

### Build

Please follow the steps below to build PEM.
```shell
# Generate the protobuf files
cd py-scripts
./proto_gen.sh
cd ..

# Build the x64 executable of PEM
cd pem-x64
../qemu-6.2.0/configure --enable-tcg-interpreter --enable-debug --disable-pie --disable-system --enable-linux-user --target-list=x86_64-linux-user
make -j4
cd ..

# Build the aarch64 executable of PEM
cd pem-aarch64
../qemu-6.2.0/configure --enable-tcg-interpreter --enable-debug --disable-pie --disable-system --enable-linux-user --target-list=aarch64-linux-user
make -j4
cd ..
```

Please refer to `README-docker.md` for running PEM.