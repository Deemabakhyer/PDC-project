# Parallel and Distributed Computing Project

## Parallelized Large-Scale String Search
This project implements **serial**, **parallel (OpenMP)**, and **distributed (MPI)** versions of the **KMP (Knuth–Morris–Pratt)** algorithm to efficiently search for and count string patterns inside large-scale text datasets.

## Dataset
* **Dataset Name:** Wikipedia Plaintext Articles (2023-07-01).
* **Source:** https://www.kaggle.com/datasets/jjinho/wikipedia-20230701?select=i.parquet

## Run Instructions
To clone this repository locally, run the following instructions:

1. **In CMD** cd to your preferred diractory
2. Run 'git clone https://github.com/Deemabakhyer/PDC-project.git'
3. Run 'code .' to launch VS Code editor
4. **In MSYS2 MINGW64 shell** cd to the diractory where the project is saved

These steps will clone and open the repository on your machine locally.

### Serial Code
**In macOS Terminal**

1. Run the instruction 'Makefile' to compile and run the source files.
2. Run './kmp_algorithm_serial' followed by the dataset name (e.g. 'i.csv') and the string pattern you want to search (e.g. 'and').

### Parallel (OpenMP) Code
**In macOS Terminal**

1. Run the instruction 'Makefile' to compile and run the source files.
2. Run './kmp_algorithm_parallel' followed by the dataset name (e.g. 'i.csv') and the string pattern you want to search (e.g. 'and').

### Distributed (MPI) Code
**In macOS Terminal**
1. Compile MPI code: 'mpicc kmp_mpi.c.c -o kmp_mpi'
2. Run MPI program: 'mpirun -np 4 ./kmp_mpi i.csv "and"'

## Additional Scripts
1. **Makefile** - used for compiling c code files.
2. **MPI folder -> Makefile** - to compile MPI version.
3. **Shell script (run_omp_tests.sh)** - used to reproduce parallel experiments.
4. **Shell script (mpi_scaling_times.sh/)** - used to reproduce distibuted experiments.
