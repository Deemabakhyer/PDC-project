#!/bin/bash

EXEC="./kmp_mpi"
DATASET="i.csv"
PATTERN="and"
RUNS=3
PROCESSES=(1 2 4 6)

echo "Processes, CommTime(s), SGTime(s), CompTime(s), TotalTime(s)"

for p in "${PROCESSES[@]}"; do
    COMM=()
    SG=()
    COMP=()
    TOTAL=()
    for ((i=1;i<=RUNS;i++)); do
        echo "Run $i with $p processes..."
        OUTPUT=$(mpirun -np $p $EXEC $DATASET $PATTERN)
        # Extract times from output
        CT=$(echo "$OUTPUT" | grep "Communication Time:" | awk '{print $3}')
        ST=$(echo "$OUTPUT" | grep "Scatter+Gather Time:" | awk '{print $3}')
        CPT=$(echo "$OUTPUT" | grep "Computation Time:" | awk '{print $3}')
        TT=$(echo "$OUTPUT" | grep "Total Time:" | awk '{print $3}')
        COMM+=($CT)
        SG+=($ST)
        COMP+=($CPT)
        TOTAL+=($TT)
    done
    # Compute mean and std function
    mean_std() {
        arr=("$@")
        n=${#arr[@]}
        sum=0
        for x in "${arr[@]}"; do sum=$(echo "$sum + $x" | bc -l); done
        mean=$(echo "scale=8; $sum / $n" | bc -l)
        sumsq=0
        for x in "${arr[@]}"; do diff=$(echo "$x - $mean" | bc -l); sumsq=$(echo "$sumsq + ($diff * $diff)" | bc -l); done
        std=$(echo "scale=8; sqrt($sumsq / $n)" | bc -l)
        echo "$mean ± $std"
    }
    echo -n "$p, "
    mean_std "${COMM[@]}"; echo -n ", "
    mean_std "${SG[@]}"; echo -n ", "
    mean_std "${COMP[@]}"; echo -n ", "
    mean_std "${TOTAL[@]}"
done
