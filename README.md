# Parallel and Distributed Computing Project

## Parallelized Large-Scale String Search
This project implements **serial**, **parallel (OpenMP)**, and **distributed (MPI)** versions of the **KMP (Knuth–Morris–Pratt)** algorithm to efficiently search for and count string patterns inside large-scale text datasets.

## Dataset
**Dataset Name:** Wikipedia Plaintext Articles (2023-07-01).
**Source:** https://www.kaggle.com/datasets/jjinho/wikipedia-20230701?select=i.parquet

## Run Instructions
to clone this repositry locally, run the following instruction:

1. **in CMD** cd to your preferred diractory
2. run 'git clone https://github.com/Deemabakhyer/PDC-project.git'
3. run 'code .' to launch VS Code editor
4. **in MSYS2 MINGW64 shell** cd to the diractory where the project is saved

these steps will clone and open the repositry on your machine locally.

### Serial Code
**in MSYS2 MINGW64 shell**

1. run the instruction 'Makefile' to compile and run the source files.
2. run './kmp_algorithm_serial' follwed by the dataset name (e.g. 'i.csv') and the string pattern you want to search (e.g. 'and').

### Parallel (OpenMP) Code
**in MSYS2 MINGW64 shell**

1. run the instruction 'Makefile' to compile and run the source files.
2. run './kmp_algorithm_parallel' follwed by the dataset name (e.g. 'i.csv') and the string pattern you want to search (e.g. 'and').

### Distributed (MPI) Code
1.
2.
3.

## Additional Scripts
1. Makefile - used for compiling c code files. 
2. Shell script (run_omp_tests.sh) - used to reproduce experiments.

