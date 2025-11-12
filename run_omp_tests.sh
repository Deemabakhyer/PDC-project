#!/bin/bash

# Usage Example:
# ./run_omp_tests.sh ./kmp_algorithm_parallel i.csv and

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <program_path> <dataset_path> <pattern>"
    exit 1
fi

PROGRAM=$1
DATASET=$2
PATTERN=$3

# Reduced thread counts
THREADS=(1 2 4 8 16)
SCHEDULES=("static" "dynamic" "guided")
CHUNKS=(16 32 64 128 256 512 1024)

echo "Running OpenMP Tuning Tests..."
echo "Program: $PROGRAM"
echo "Dataset: $DATASET"
echo "Pattern: $PATTERN"
echo "--------------------------------------------------------------"
echo "Threads | Schedule & Chunk size | Avg Time ± StdDev (s) | Matches"
echo "--------------------------------------------------------------"

for sched in "${SCHEDULES[@]}"; do
    for chunk in "${CHUNKS[@]}"; do
        for t in "${THREADS[@]}"; do
            export OMP_NUM_THREADS=$t
            export OMP_SCHEDULE="${sched},${chunk}"

            times=()
            total_matches=0

            # Run 3 trials
            for i in {1..3}; do
                output=$(./$PROGRAM $DATASET $PATTERN 2>/dev/null)
                time_taken=$(echo "$output" | grep -i "Time taken" | awk '{print $3}')
                matches=$(echo "$output" | grep -i "Total matches" | awk '{print $3}')

                if [ -n "$time_taken" ]; then
                    times+=($time_taken)
                    total_matches=$matches
                fi
            done

            if [ ${#times[@]} -gt 0 ]; then
                # calculate mean
                sum=0
                for val in "${times[@]}"; do
                    sum=$(echo "$sum + $val" | bc)
                done
                n=${#times[@]}
                mean=$(echo "scale=6; $sum / $n" | bc)

                # calculate stddev
                sumsq=0
                for val in "${times[@]}"; do
                    diff=$(echo "$val - $mean" | bc)
                    sq=$(echo "$diff * $diff" | bc)
                    sumsq=$(echo "$sumsq + $sq" | bc)
                done
                stddev=$(echo "scale=6; sqrt($sumsq / $n)" | bc)

                printf "%-7s | %-22s | %-18s | %s\n" "$t" "${sched},${chunk}" "${mean} ± ${stddev}" "$total_matches"
            else
                printf "%-7s | %-22s | %-18s | %s\n" "$t" "${sched},${chunk}" "N/A" "N/A"
            fi
        done
    done
done


