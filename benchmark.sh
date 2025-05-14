#!/bin/bash

# Define paths to the source files directly in the script
SERIALOMP_PATH="./benchmark/benchmark_iteration_1/serialomp"
ROWOMP_PATH="./benchmark/benchmark_iteration_1/rowomp"
ANTIOMP_PATH="./benchmark/benchmark_iteration_1/antiomp"
SWIPE_PATH="./external/swipe"
SSW_PATH="./external/ssw/src"

# Function to check if the executable exists
check_executable() {
    local path=$1
    if [[ ! -x "$path" ]]; then
        echo "Error: Executable not found or not executable: $path"
        exit 1
    fi
}

echo "Generating test data..."
python3 ./misc/test_data.py || { echo "Error: Failed to generate test data."; exit 1; }

echo "Running GCUPS benchmarks..."
echo "=========================="

QUERY=$(cat dataset/query.txt)
REF=$(cat dataset/ref.txt)
QUERY_LEN=1000
REF_LEN=100000000000000
NUM_RUNS=5  # Run each test 5 times for better average
NUM_THREADS=2

# Function to calculate GCUPS
calculate_gcups() {
    local time=$1  # time in seconds
    echo "scale=2; ($QUERY_LEN * $REF_LEN) / ($time * 1000000000)" | bc
}

# Function to run test multiple times and get average GCUPS
run_test() {
    local name=$1
    local cmd=$2
    local total_gcups=0
    
    echo "Testing $name..."
    for ((i=1; i<=$NUM_RUNS; i++)); do
        start_time=$(date +%s.%N)
        eval $cmd > /dev/null 2>&1
        end_time=$(date +%s.%N)
        time_taken=$(echo "$end_time - $start_time" | bc)
        
        if [[ $(echo "$time_taken > 0" | bc) -eq 1 ]]; then
            gcups=$(calculate_gcups $time_taken)
            total_gcups=$(echo "$total_gcups + $gcups" | bc)
        else
            echo "Warning: Test $i for $name failed or took zero time."
        fi
    done
    
    avg_gcups=$(echo "scale=2; $total_gcups / $NUM_RUNS" | bc)
    echo "$name Average GCUPS: $avg_gcups"
    echo "------------------------"
}

# Check if all executables exist
check_executable "$SERIALOMP_PATH/sw_serial_omp"
check_executable "$ROWOMP_PATH/rowparallel_omp"
check_executable "$ANTIOMP_PATH/sw_anti_omp"
check_executable "$SWIPE_PATH/swipe"
check_executable "$SSW_PATH/ssw_test"

# Test all implementations
run_test "Serial OpenMP" "$SERIALOMP_PATH/sw_serial_omp \"$QUERY\" \"$REF\" $NUM_THREADS"
run_test "Row Parallel OpenMP" "$ROWOMP_PATH/sw_omp \"$QUERY\" \"$REF\" $NUM_THREADS"
run_test "Anti-diagonal OpenMP" "$ANTIOMP_PATH/sw_anti_omp \"$QUERY\" \"$REF\" $NUM_THREADS"
run_test "SWIPE" "$SWIPE_PATH/swipe -M BLOSUM62 -G 11 -E 1 -b 1 -v 1 -i query.txt -d ref.txt"
run_test "Striped Smith-Waterman" "$SSW_PATH/ssw_test -c -1 -x 1 -o 1 -e 1 -m 1 query.txt ref.txt"

