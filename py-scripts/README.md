#README

## PreAnalysis

1. Use Ghidra to generate `*.gswitch` file. This file contains additional switch
   information used by the IDA script.
2. Use `elf_parser.py` to generate `*.plt` & `*.cxx.plt` file. This file contains libcalls
   used by the IDA script.
3. Use `ida_cfg_generator.py` to generate `*.cfg` file.
4. Use `ida_find_string.py` to generate `*.str` file.
5. Use `pre_analysis.py` to generate `*.pa` file. This file will be further parsed
   by the C++ emulator.
6. Use `rule_out_functions.py` to generate `*.ruleout` file.

## PostAnalysis

1. Use `ida_get_function_name.py` to get `.fname` file. This file contains
map from function addr to function name.
2. Use `preprocess.py` to generate sets from the `.pb` file dumped by PEM.
3. Use `compare.py` to generate the comparison results.
4. Use `score.py` to calculate the score.
