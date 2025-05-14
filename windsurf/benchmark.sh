#!/bin/bash

# Check for correct number of arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <QUERY_FILE> <REFERENCE_FILE> <THREAD_COUNT>"
    exit 1
fi

# Assign arguments to variables
QUERY_FILE=$1
REFERENCE_FILE=$2
THREAD_COUNT=$3

# Paths to binaries (update these paths accordingly)
SSW_BINARY="./external/ssw/ssw_test"
SWIPE_BINARY="./external/swipe/swipe"
SERIAL="./benchmark/benchmark_iteration_1/serial/sw_serial_omp"
ROWOMP_BINARY="./benchmark/benchmark_iteration_1/rowomp/rowparallel_omp"
ROWOMPV2_BINARY="./benchmark/benchmark_iteration2/rowomp/rowparallel_omp_v2"
ANTIDIAG="./benchmark/benchmark_iteration_1/antiomp/sw_anti_omp"
ANTIDIAGV2="./benchmark/benchmark_iteration2/antiomp/sw_anti_omp_v2"

# Run SSW
echo "Running SSW..."
$SSW_BINARY "$REFERENCE_FILE" "$QUERY_FILE"

# Run SWIPE
echo "Creating BLAST database for SWIPE..."
makeblastdb -in "$REFERENCE_FILE" -dbtype nucl -parse_seqids -blastdb_version 4

echo "Running SWIPE..."
$SWIPE_BINARY -d "$REFERENCE_FILE" -i "$QUERY_FILE" -p 0 -r 1 -q -3 -G 5 -E 2 -a "$THREAD_COUNT"

# Run SERIAL
echo "Running Serial..."
$SERIAL "$QUERY_FILE" "$REFERENCE_FILE" "$THREAD_COUNT"

# Run ROWOMP
echo "Running RowOMP..."
$ROWOMP_BINARY "$QUERY_FILE" "$REFERENCE_FILE" "$THREAD_COUNT"

# Run ROWOMPV2
echo "Running RowOMPV2..."
$ROWOMPV2_BINARY "$QUERY_FILE" "$REFERENCE_FILE" "$THREAD_COUNT"

# Run ANTIDIAG
echo "Running Antidiag..."
$ANTIDIAG "$QUERY_FILE" "$REFERENCE_FILE" "$THREAD_COUNT"

# Run ANTIDIAGV2
echo "Running AntidiagV2..."
$ANTIDIAGV2 "$QUERY_FILE" "$REFERENCE_FILE" "$THREAD_COUNT"
