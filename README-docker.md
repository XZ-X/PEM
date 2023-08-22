# Overview

This file contains the instructions for running PEM and reproducing the results in the paper.
Reviewers will need to install Docker and pull the Docker image from [Docker Hub](https://hub.docker.com/r/pemauthors/pem-demo).
Then, they can run PEM in the Docker image.

The instruction consists of two parts:
The first part instructs reviewers pull the Docker image, build PEM,
and run a quick example.

The second part contains the instructions for running PEM on the whole evaluation dataset.

# Setup and Run a Quick Example

## Pull the Docker Image

Please use the following command to pull the Docker image.
The size of the image is about 3GB. It may take a few minutes to download the image.

```bash
docker pull pemauthors/pem-demo:ae
```

After pulling the image, please run the container with the following command.
`-it` means the container is interactive.
`zsh` is the shell used in the container.
```bash
mkdir -p pem-shared-data
docker run -it pemauthors/pem-demo:ae zsh
```

The PEM repository is located at `/root/pem-review` in the container.

## File Structure

After entering the directory `/root/pem-review`,
reviewers can see the following files and directories:
```
.
├── aarch64-coreutils
├── eval-cross-arch
├── eval-dataset
├── eval-scripts
├── py-scripts
└── qemu-6.2.0
```

The directory `qemu-6.2.0` contains the source code of PEM.
It is built on top of QEMU 6.2.0.
Please refer to our [GitHub repository](https://github.com/pempaper/pem-review) for detailed information about the source code.

The directory `py-scripts` contains the preprocess and postprocess scripts used in PEM.
It also contains the scripts for sampling random numbers used in the probabilistic path sampling algorithm 
and the probabilistic memory model.

The directory `eval-scripts` and `eval-cross-arch` contains the scripts for reproducing the results in the paper.

The directory `eval-dataset` and `aarch64-coreutils` contains the dataset used for evaluation.

## Build PEM

First, please make the following directories under `pem-review`
```
# Directories for building the x64 and aarch64 executable of PEM
mkdir -p pem-x64
mkdir -p pem-aarch64

# Directories for storing the intermediate comparison results between binary projects
mkdir -p cmp-ret
mkdir -p cmp-ret/eval-coreutils
mkdir -p cmp-ret/eval-trex
mkdir -p cmp-ret/eval-how-help

# Directories for storing the scores of comparison results (e.g., PR@1)
mkdir -p score-ret
mkdir -p score-ret/eval-coreutils
mkdir -p score-ret/eval-trex
mkdir -p score-ret/eval-how-help
```

Please run the following commands to build PEM. The build process may take around 15 minutes in total.
```
# Generate the protobuf files
cd py-scripts
./proto_gen.sh
cd ..
```

```
# Build the x64 executable of PEM
cd pem-x64
../qemu-6.2.0/configure --enable-tcg-interpreter --enable-debug --disable-pie --disable-system --enable-linux-user --target-list=x86_64-linux-user
make -j4
cd ..
```

```
# Build the aarch64 executable of PEM
cd pem-aarch64
../qemu-6.2.0/configure --enable-tcg-interpreter --enable-debug --disable-pie --disable-system --enable-linux-user --target-list=aarch64-linux-user
make -j4
cd ..
```

```
# Build the random numbers for the probabilistic path sampling algorithm
# and the probabilistic memory model
python3 py-scripts/probabilistic_sampling.py --a=3e-2 --fout pem-x64/prob_path.bin
python3 py-scripts/generate_random_memory.py pem-x64/mem.pmem 
cp pem-x64/prob_path.bin pem-aarch64/
cp pem-x64/mem.pmem pem-aarch64/
```


## A Quick Example

In this example, we use PEM to query functions in the `libcurl` binary program.
We will query with functions in the `libcurl` binary program compiled with `-O3`
in a pool of functions in the `libcurl` binary program compiled with `-O0`.

It consists of four steps:
1. The probabilistic execution engine first samples observable values for each function from both programs.
2. Then a preprocess script is used to aggregate the sampled values into multisets.
3. Set comparison is then performed on the multisets, calculating the similarity between every pair of functions.
The intermediate results are stored in the directory `cmp-ret/eval-trex/`.
5. Finally, a metric script is used to calculate the scores of the intermediate results.


**Step 1: Run the probabilistic execution engine on the two binary programs.**

It takes about 5 minutes to run the following commands.
```
pem-x64/qemu-x86_64 eval-dataset/eval-trex/libcurl460.so.O0.elf -M pem-x64/mem.pmem -P pem-x64/prob_path.bin
pem-x64/qemu-x86_64 eval-dataset/eval-trex/libcurl460.so.O3.elf -M pem-x64/mem.pmem -P pem-x64/prob_path.bin
```

After this step, the observable values are stored in `eval-dataset/eval-trex/eval-dataset/eval-trex/libcurl460.so.O0.elf*.pb` and `eval-dataset/eval-trex/libcurl460.so.O3.elf*.pb`.
The `*.pb` files are protobuf files whose format is defined in `py-scripts/sem_features.proto`.

**Step 2: Aggregate the sampled values into multisets.**
It takes about 5 minutes to run the following commands.
```
python3 py-scripts/preprocess_large.py --src eval-dataset/eval-trex/libcurl460.so.O0.elf
python3 py-scripts/preprocess_large.py --src eval-dataset/eval-trex/libcurl460.so.O3.elf
```
After this step, the multisets are stored in `eval-dataset/eval-trex/libcurl460.so.O0.elf.preprocess.pkl`
and `eval-dataset/eval-trex/libcurl460.so.O3.elf.preprocess.pkl`.
These two files are pickle files, which can be directly loaded by python.

**Step 3: Perform set comparison on the multisets.**
The following commands leverage 4 processes. It takes about 5 minutes.
```
python3 py-scripts/compare.py --src eval-dataset/eval-trex/libcurl460.so.O0.elf --tgt eval-dataset/eval-trex/libcurl460.so.O3.elf --fout cmp-ret/eval-trex/libcurl460.so.O0vsO3.pkl
```
The intermediate results are stored in `cmp-ret/eval-trex/libcurl460.so.O0vsO3.pkl`.

**Step 4: Calculate the scores of the intermediate results.**
It takes about 1 minute to run the following command.
```
python3 py-scripts/score.py --src eval-dataset/eval-trex/libcurl460.so.O0.elf --tgt eval-dataset/eval-trex/libcurl460.so.O3.elf --cmp cmp-ret/eval-trex/libcurl460.so.O0vsO3.pkl > score-ret/eval-trex/libcurl460.so.O0vsO3.score.txt 2>&1
```
The scores are stored in `score-ret/eval-trex/libcurl460.so.O0vsO3.score.txt`.

The score script takes an optional argument `--sample`. 
It specifies the number of negative samples (i.e., different function pairs)
to be used for each positive sample (i.e., the same function pair).

For example, the following command uses 100 negative samples for each positive sample.
```
python3 py-scripts/score.py --src eval-dataset/eval-trex/libcurl460.so.O0.elf --tgt eval-dataset/eval-trex/libcurl460.so.O3.elf --cmp cmp-ret/eval-trex/libcurl460.so.O0vsO3.pkl --sample 100 > score-ret/eval-trex/libcurl460.so.O0vsO3.score.100.txt 2>&1
```
The scores are stored in `score-ret/eval-trex/libcurl460.so.O0vsO3.score.100.txt`.


# Evaluation on the Full Dataset

We combine the above steps into a single script to simplify the evaluation process.
Please run the following commands to evaluate PEM on the full dataset.
It takes about 20 hours to finish.
Also, it requires about 190GB of disk space.
```
eval-scripts/run.sh
```

The above script generates most intermediate results used in the following reproducing steps.
Next, we will detail how to reproduce the results in the paper.

## Table 1

Please use the following command to generate the results for Table 1.
It takes around 20 minutes to finish.
```
eval-scripts/eval-coreutils.sh
```

Then the reported scores can be seen with the following command.
```
eval-scripts/coreutils_report-score.sh 
```

The expected output is as follows.
```
clangO0 vs clangO3 PR1
ncorrect: 5173, ntotal: 5476, avg: 0.944668
clangO0 vs clangO3 PR3
ncorrect: 5378, ntotal: 5476, avg: 0.982104
clangO0 vs clangO3 PR5
ncorrect: 5406, ntotal: 5476, avg: 0.987217
...
```

### Detailed Explanation for Table 1

> We directly compare with the reported number of IMF and BLEX on Dataset-I (i.e., Coreutils). 
Thus this experiment will not try to reproduce the results of these two baseline approaches.
We contact the author of IMF to align the experimental setups.
For comparison with BLEX, it is sufficient to align PEM with IMF
because the setup of IMF is aligned with BLEX, .

IMF and BLEX compile Coreutils to 106 separate binaries.
They then run their tools on each pair of corresponding binaries and report the average precision.

In our dataset, Coreutils is compiled into one single binary.
Thus we first split the functions in Coreutils following the function distributions in the 106 binaries.
The split is achieved with the following script.
```
eval-scripts/coreutils_split.sh 
```

After that, we use the following scripts to calculate the precision for each pair of the 106 Coreutils binaries.
```
eval-scripts/coreutils_c0c3.sh
eval-scripts/coreutils_c2c3.sh
eval-scripts/coreutils_g0g3.sh
eval-scripts/coreutils_g2g3.sh
eval-scripts/coreutils_c0g3.sh
eval-scripts/coreutils_g0c3.sh
```

Finally, the following script reports the average precision.
```
eval-scripts/coreutils_report-score.sh
```

## Figure 10: Comparison with ML-based Approaches

The scores stored in `score-ret` are used to compare PEM with ML-based approaches.
These scores are sampled in `1:100`, following the setting in the How-Solve paper.

Use the following command to see the scores.
```
find score-ret -name "*.txt"|xargs tail -n +1 |less
```

A fully-reproduce of the ML-based baseline results requires huge efforts.
For all the baseline approaches, we simply use the code base provided by the baseline authors
and follow the default training setup.
The most effort-consuming part is to preprocess our test dataset into the format required by the baseline methods
and train the baseline models.

Code bases we used for the baseline methods:
- [How-Solve](https://github.com/Cisco-Talos/binary_function_similarity.git)
- [Trex](https://github.com/CUMLSec/trex)
- [SAFE](https://github.com/facebookresearch/SAFEtorch)

Here we provide the intermediate results output by the baseline methods and
the scripts we use to calculate the scores.

The results for the baseline methods are stored in `ml-baseline-results`.

We use the following script to sample the results of the best two models in How-Solve.
We have already constructed their testset in `1:100`. Thus the following script
simply reads all the data and compute the score.
It takes about 2 minutes to finish.
```shell
eval-scripts/ml_baseline_sample.sh
```

Finally, this script prints out results used in figure 10.

```shell
eval-scripts/ml_baseline_report.sh
```


## Figure 26 in the Supplementary Material

By changing the parameters in the top of `eval-scripts/score.sh`  to different numbers,
we can reproduce the results for PEM used in Figure 26. 
We use the following scripts to generate the results for Trex and SAFE.

```shell
eval-scripts/ml_baseline_trex_sample.sh
eval-scripts/ml_baseline_safe_sample.sh
```


## RQ2: Real-World Case Study

In the case study, we select 8 CVE-related functions (listed in Table 7 of the
supplementary material).
For each function, we query with the optimized version of the problematic function 
in the pool of all functions in the unoptimized version (of the same project).

The following commands generate the `.csv` files used in the case study.
It takes around 20 minutes to finish.
```
eval-scripts/eval-case.sh
```
The script mimics the process of querying by collecting all the lines in the intermediate results
with the name of query function equals to the problematic function.
The `.csv` files are stored in `CVE-cases/`.
Then we use Excel to sort the `.csv` files and manually inspect the results.

Each file is named as `case-<case-number>.<tool-name>.csv`.
It represents the query results of a problematic function.
Each line in the file is a comparison result.
The first two columns are the names of the query function and the candidate function.
The third column is the ground truth, i.e., whether the two functions are similar.
The fourth column is the similarity score.
For files with more than 4 columns, the remaining columns can be ignored.

To generate the results in Table 7, we use Excel to read the `.csv` files and 
sort the comparison results by the similarity score. We record the rank of the
ground truth function in the sorted list if it appears in the top 200 results.

## Cross-architecture Evaluation

The following commands evaluate PEM on the cross-architecture dataset.
It takes around 2 hours to finish.
```
eval-cross-arch/run.sh
```
The scores are stored in `score-ret/cross-arch`.
The reported scores contain **all** possible candidate functions in target binaries.

## Coverage

The coverage per function is stored in the `*.cov` files 
for each binary program. They are in the same path as the binary programs (in `eval-dataset` directory).

## Ablation Study

For ablation study, we manually change the code in `qemu-6.2.0/linux-user/pem_config.cc`
and recompile PEM to collect different sets of results.
For ablation study, we use `eval-dataset/eval-coreutils/coreutils.clang12.O0.elf`
as the query binary and `eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf`
as the target binary.

Each time the configuration file is changed, we need to recompile PEM:
```shell
cd pem-x64
make -j4
cd ..
```
PEM can be run with the following command:
```shell
pem-x64/qemu-x86_64 eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf -M pem-x64/mem.pmem -P pem-x64/prob_path.bin
```
where the first parameter `eval-dataset/eval-coreutils/coreutils.gcc94.O3.elf`
is the path to the binary program to analyze.
The second parameter `-M pem-x64/mem.pmem` specifies the path to the memory model.
The third parameter `-P pem-x64/prob_path.bin` specifies the path to the path sampling profile.

After both the query and target binaries are analyzed, please follow the quick example
in the first part of this document to generate the intermediate results and calculate the scores.

Most parameters can be found in Table 2, Table 3 in the paper, and Table 5 
in the Appendix, except for the following experiments:
- For all experiments in Table 2,4,5: set `SAMPLER_ROUND_MAGIC` to `200`.
This number controls the maximum number of paths sampled for each function.
We set it to a smaller number to speed up the evaluation.

- `LastPred` in Table 2: Set `DETERMINISTIC` to `true`.

- `Det.` in Table 2:
Use `python3 py-scripts/probabilistic_sampling.py --both --fout pem-x64/determ-path.bin` to generate the path sampling profile corresponding to the deterministic path sampling algorithm.
Use `pem-x64/qemu-x86_64 ... -P pem-x64/determ-path.bin` to run PEM with the deterministic path sampling profile.

- `No-Mem` in Table 3: Set `PROB_MEM` to `false`.

- `Const` in Table 3: 
Use `python3 py-scripts/generate_random_memory.py pem-x64/const.pmem determ`
to generate the memory model corresponding to the memory model with const values.
Use `pem-x64/qemu-x86_64 ... -M pem-x64/const.pmem` to run PEM with the memory model with const values.

Note that for the results in Table 2, the selected 80 most challenging functions 
are listed in `ablation-study/selected.txt`.
We only calculate the precision for these functions in Table 2.

Please use the following commands to generate data used by Figure 12 and Figure 13.
```shell
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.clang12.O0.elf --tgt ablation-study/branches/coreutils.clang12.O3.elf > ablation-study/branches/c0c3.branch.txt
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.clang12.O2.elf --tgt ablation-study/branches/coreutils.clang12.O3.elf > ablation-study/branches/c2c3.branch.txt
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.gcc94.O0.elf --tgt ablation-study/branches/coreutils.gcc94.O3.elf > ablation-study/branches/g0g3.branch.txt
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.gcc94.O2.elf --tgt ablation-study/branches/coreutils.gcc94.O3.elf > ablation-study/branches/g2g3.branch.txt
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.clang12.O0.elf --tgt ablation-study/branches/coreutils.gcc94.O3.elf > ablation-study/branches/c0g3.branch.txt
python3 py-scripts/analyze_branch_info.py --src ablation-study/branches/coreutils.gcc94.O0.elf --tgt ablation-study/branches/coreutils.clang12.O3.elf > ablation-study/branches/g0c3.branch.txt
```