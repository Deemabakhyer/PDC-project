#!/bin/bash

# Usage: ./run_omp_tests.sh <program> <input_file> <pattern>
# Usage: ./run_omp_tests.sh ./kmp_algorithm_parallel i.csv and
# Example: ./run_omp_tests.sh ./kmp_algorithm_parallel i.csv and

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <program_path> <dataset_path> <pattern>"
    exit 1
fi

PROGRAM=$1
DATASET=$2
PATTERN=$3

THREADS=(1 2 4 6 8 10 12 14 16 18 20)
SCHEDULES=("static" "dynamic" "guided")

echo "Running OpenMP Tuning Tests..."
echo "Program: $PROGRAM"
echo "Dataset: $DATASET"
echo "Pattern: $PATTERN"
echo "-----------------------------------------------"

for sched in "${SCHEDULES[@]}"; do
    echo "Schedule Type: $sched"
    echo "Threads | Trial1 | Trial2 | Trial3 | Average | Matches"
    echo "------------------------------------------------------"
    
    for t in "${THREADS[@]}"; do
        export OMP_NUM_THREADS=$t
        export OMP_SCHEDULE=$sched
        
        total_time=0
        total_matches=0
        
        for i in {1..3}; do
            output=$(./$PROGRAM $DATASET $PATTERN)
            
            # استخراج الوقت وعدد الماتشز من ناتج البرنامج
            time_taken=$(echo "$output" | grep "Time taken" | awk '{print $3}')
            matches=$(echo "$output" | grep "Total matches" | awk '{print $3}')
            
            if [ -z "$time_taken" ]; then
                echo "⚠️ Warning: Failed to parse output for trial $i (threads=$t, schedule=$sched)"
                continue
            fi
            
            total_time=$(echo "$total_time + $time_taken" | bc)
            total_matches=$matches
            
            eval "time_$i=$time_taken"
        done
        
        avg_time=$(echo "scale=6; $total_time / 3" | bc)
        
        printf "%-7s | %-7s | %-7s | %-7s | %-8s | %s\n" "$t" "$time_1" "$time_2" "$time_3" "$avg_time" "$total_matches"
    done
    
    echo ""
done
