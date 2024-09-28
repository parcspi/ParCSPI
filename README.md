
# Partial Context-Sensity Security Protection - ParCSPI

  

## Description

This repository contains the code related to the paper "Y. Wang, K. Mack, T. Chantem, S. Baruah, N. Zhang and B. Ward. Partial Context-Sensitive Pointer Integrity for Real-time Embedded Systems. In IEEE Real-Time Systems Symposium (RTSS), 2024"

  

ParCSPI provides a flexible framework to balance the trade-off between pointer integrity security protection and real-time schedulability for different computational demands. ParCSPI first conducts context-sensitive static program analysis, and then uses per-task optimization scripts to generate the relationship between security and performance costs. Finally, it allows users to select a policy, and  automatically deploys the corresponding security checks on the target program.


  

## Table of Contents

- [Installation](#installation)

- [Usage](#usage)

- [License](#license)

- [Bibliography](#bibliography)

  
## Code Structure

```
📦 ParCSPI
├─ program_analysis/ # static program analyzer based on SVF, and LLVM software instrumentation code
├─ data_analysis/ # per-task optimization scripts
├─ log_lib/ # runtime library
├─ test/ #test directory 
│  └─ run.sh # automated script to compile and run sample program
│  └─ test_target # sample program to be protected
```
## Installation

#### Directory Setup
  - The running script assumes the  VM has the following file directory.
```
cd ~/

git clone https://github.com/parcspi/ParCSPI.git 

mkdir ~/toolchain && cd ~/toolchain

cp -r ~/ParCSPI/pac-svf SVF 

git clone https://github.com/llvm/llvm-project.git llvm-project
```

#### Environment Setup
- Setup the corresponding environmental variables by copying following command into ``~/.bashrc``.
```
export LLVM_DIR=~/toolchain/llvm-project/build
export SVF_DIR=~/toolchain/SVF
export PATH="$LLVM_DIR/bin:$SVF_DIR/Release-build/bin:$PATH"
export PAC_DIR=~/ParCSPI
```
- Compile LLVM according to the instructions on the official [website](https://github.com/llvm/llvm-project) .
- Compile SVF ccording to the instructions on the official [website](https://github.com/SVF-tools/SVF) .

#### Basic Test
 - The successful setup of an LLVM environment can be verified with the following commands:
```
opt --version
```

- To check the installation of SVF, execute:
```
wpa --version
```

If the final outputs do not indicate any errors, it can be concluded that these dependencies are installed properly.



## Usage

  
#### Compilation

The following command will compile and run a demo on a sample program located in ``$PAC_DIR/test/test_target/eval/evaluation/src/sample_program/sample_program.c``.

  

```

cd $PAC_DIR/test 
source ./run.sh

```
First, the script conducts a context-sensitive analysis using the SVF library ``libpac_analysis.so``. Then, it utilizes the optimization script ``data_analysis/main.py`` to generate the relationship between security (quantified as a score defined in the paper) and performance costs. Meanwhile, with a user-selected security score ``POLICY_SECURITY_SCORE``, the script outputs the corresponding optimal policy in ``optimal_policy.txt``.

Based on the selected security policy, the LLVM instrumentation library ``libpac_enforce.so`` automatically instruments the program code. The final protected program binary is generated as an executable named ``protected_bin``.
  
  #### Execution

```
 cd {path_to_sample_code_directory}
 
 ./protected_bin
```


## License

MIT License, Apache License, etc.

  

## Bibliography

- Y. Wang, K. Mack, T. Chantem, S. Baruah, N. Zhang and B. Ward. Partial Context-Sensitive Pointer Integrity for Real-time Embedded Systems. In IEEE Real-Time Systems Symposium (RTSS 2024).