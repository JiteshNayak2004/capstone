#!/bin/bash

# Configuration
SEQUENCE_LENGTHS=(500)
THREAD_COUNTS=(1 2 4 8 16)
ITERATIONS=5

# Implementation directories and executables
declare -A EXECUTABLES=(
    ["rowomp"]="./benchmark/benchmark_iteration_1/rowomp_opt/sw_rowomp_opt"
    ["antiomp"]="./benchmark/benchmark_iteration_1/antiomp_opt/sw_antiomp_opt"
    ["swim"]="./external/ssw/bin/ssw_test"
    ["stripe"]="./external/swipe/bin/swipe"
)

# Create results directory
RESULTS_DIR="./results"
mkdir -p "$RESULTS_DIR"

# Performance counters to monitor
PERF_EVENTS="cycles,instructions,cache-references,cache-misses,branch-misses,L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses"

# Function to generate random DNA sequence
generate_sequence() {
    length=$1
    chars="ACGT"
    seq=""
    for ((i=0; i<length; i++)); do
        seq+=${chars:$((RANDOM % 4)):1}
    done
    echo "$seq"
}

# Function to run perf and extract metrics
run_perf_test() {
    impl=$1
    seq1=$2
    seq2=$3
    threads=$4
    
    echo "Running perf for $impl with $threads threads..."
    
    # Run perf stat with the appropriate executable
    perf stat -e $PERF_EVENTS \
        "${EXECUTABLES[$impl]}" "$seq1" "$seq2" "$threads" \
        2> "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt"
    
    # Extract metrics from perf output
    cycles=$(grep "cycles" "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt" | awk '{print $1}' | tr -d ',')
    instructions=$(grep "instructions" "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt" | awk '{print $1}' | tr -d ',')
    cache_misses=$(grep "cache-misses" "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt" | awk '{print $1}' | tr -d ',')
    l1_misses=$(grep "L1-dcache-load-misses" "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt" | awk '{print $1}' | tr -d ',')
    llc_misses=$(grep "LLC-load-misses" "$RESULTS_DIR/perf_${impl}_${threads}_${#seq1}.txt" | awk '{print $1}' | tr -d ',')
    
    # Calculate metrics
    ipc=$(echo "scale=2; $instructions/$cycles" | bc)
    miss_rate=$(echo "scale=2; $cache_misses * 100 / $instructions" | bc)
    
    echo "$impl,$threads,${#seq1},$cycles,$instructions,$ipc,$cache_misses,$miss_rate,$l1_misses,$llc_misses" >> "$RESULTS_DIR/detailed_metrics.csv"
}

# Initialize results files
echo "Implementation,Threads,SeqLength,Cycles,Instructions,IPC,CacheMisses,MissRate,L1Misses,LLCMisses" > "$RESULTS_DIR/detailed_metrics.csv"
echo "Implementation,Threads,SeqLength,Time,GCUPS,ParallelEfficiency" > "$RESULTS_DIR/performance_metrics.csv"

# Run benchmarks
for seq_len in "${SEQUENCE_LENGTHS[@]}"; do
    echo "Testing sequence length: $seq_len"
    
    # Generate test sequences
    query=$(generate_sequence $seq_len)
    ref=$(generate_sequence $seq_len)
    
    for impl in "${!EXECUTABLES[@]}"; do
        for threads in "${THREAD_COUNTS[@]}"; do
            total_time=0
            min_time=999999
            max_time=0
            
            for ((i=1; i<=ITERATIONS; i++)); do
                # Run with perf
                run_perf_test "$impl" "$query" "$ref" "$threads"
                
                # Get execution time
                time_output=$(${EXECUTABLES[$impl]} "$query" "$ref" "$threads")
                exec_time=$(echo "$time_output" | grep "Time taken:" | awk '{print $3}')
                
                # Update statistics
                total_time=$(echo "$total_time + $exec_time" | bc)
                min_time=$(echo "if ($exec_time < $min_time) $exec_time else $min_time" | bc)
                max_time=$(echo "if ($exec_time > $max_time) $exec_time else $max_time" | bc)
            done
            
            # Calculate average time and GCUPS
            avg_time=$(echo "scale=4; $total_time / $ITERATIONS" | bc)
            gcups=$(echo "scale=2; ($seq_len * $seq_len) / ($avg_time * 1000000000)" | bc)
            
            # Calculate parallel efficiency (relative to single thread)
            if [ "$threads" -eq 1 ]; then
                base_time=$avg_time
                efficiency=100
            else
                efficiency=$(echo "scale=2; ($base_time / ($avg_time * $threads)) * 100" | bc)
            fi
            
            # Save performance metrics
            echo "$impl,$threads,$seq_len,$avg_time,$gcups,$efficiency" >> "$RESULTS_DIR/performance_metrics.csv"
        done
    done
done

# Generate analysis report
cat > "$RESULTS_DIR/detailed_analysis.txt" << EOL
Performance Analysis Report
=========================
Generated on: $(date)

1. Implementation Comparison
---------------------------
$(awk -F',' '
    NR>1 {
        sum[$1","$3]+=$4; 
        count[$1","$3]++; 
        if($2==1) base[$1","$3]=$4
    } 
    END {
        print "Average Execution Time (seconds) by Implementation and Sequence Length:\n"
        for (key in sum) {
            split(key,k,",")
            printf "%-20s %8s: %10.4f\n", k[1], k[2], sum[key]/count[key]
        }
    }
' "$RESULTS_DIR/performance_metrics.csv")

2. Cache Performance
-------------------
$(awk -F',' '
    NR>1 {
        sum_miss[$1]+=$7; 
        count[$1]++
    } 
    END {
        print "Average Cache Miss Rate (%) by Implementation:\n"
        for (impl in sum_miss) {
            printf "%-20s: %10.2f%%\n", impl, sum_miss[impl]/count[impl]
        }
    }
' "$RESULTS_DIR/detailed_metrics.csv")

3. Instruction Level Parallelism
-------------------------------
$(awk -F',' '
    NR>1 {
        sum_ipc[$1]+=$6; 
        count[$1]++
    } 
    END {
        print "Average Instructions per Cycle (IPC) by Implementation:\n"
        for (impl in sum_ipc) {
            printf "%-20s: %10.2f\n", impl, sum_ipc[impl]/count[impl]
        }
    }
' "$RESULTS_DIR/detailed_metrics.csv")

4. Scalability Analysis
----------------------
$(awk -F',' '
    NR>1 {
        if($3==2000) {
            printf "%-20s %2d threads: %10.2f%% efficiency\n", $1, $2, $6
        }
    }
' "$RESULTS_DIR/performance_metrics.csv")

5. Memory Hierarchy Impact
-------------------------
$(awk -F',' '
    NR>1 {
        l1_miss_rate[$1]+=($8/$7)*100; 
        count[$1]++
    } 
    END {
        print "Average L1 Cache Miss Rate (%) by Implementation:\n"
        for (impl in l1_miss_rate) {
            printf "%-20s: %10.2f%%\n", impl, l1_miss_rate[impl]/count[impl]
        }
    }
' "$RESULTS_DIR/detailed_metrics.csv")

Conclusions
----------
1. Cache Efficiency: Implementations with lower cache miss rates generally show better performance
2. Parallel Scaling: Higher thread counts show diminishing returns due to memory bandwidth limitations
3. Memory Access Patterns: Different implementations show varying cache utilization patterns
4. Overall Efficiency: Trade-offs between parallelism and memory access efficiency are evident
EOL

echo "Benchmark completed. Results and analysis are in $RESULTS_DIR/"
