#!/usr/bin/env python3

import os
import time
import subprocess
import numpy as np
from pathlib import Path
import sys

# Add project root to Python path
project_root = Path(__file__).parent.parent
sys.path.append(str(project_root))
from misc.test_data import generate_random_sequence

def run_implementation(impl_path, query, reference, num_threads=4):
    """Run a Smith-Waterman implementation and measure execution time"""
    start_time = time.time()
    
    try:
        # Ensure query and reference are passed as temporary files if needed
        query_file = Path(f"{project_root}/tmp_query.txt")
        ref_file = Path(f"{project_root}/tmp_ref.txt")
        
        # Write the sequences to temporary files
        with open(query_file, 'w') as f:
            f.write(query)
        
        with open(ref_file, 'w') as f:
            f.write(reference)
        
        # Run the implementation with file paths as arguments
        result = subprocess.run(
            [impl_path, str(query_file), str(ref_file), str(num_threads)],
            capture_output=True,
            check=True
        )
        
        end_time = time.time()
        print(f"Output: {result.stdout.decode()}")
        return end_time - start_time
    except subprocess.CalledProcessError as e:
        print(f"Error running {impl_path}:")
        print(f"Exit code: {e.returncode}")
        print(f"Stdout: {e.stdout.decode()}")
        print(f"Stderr: {e.stderr.decode()}")
        return None
    finally:
        # Cleanup temporary files
        if 'query_file' in locals():
            os.remove(query_file)
        if 'ref_file' in locals():
            os.remove(ref_file)

def calculate_gcups(query_len, ref_len, time_seconds):
    """Calculate GCUPS (Giga Cell Updates Per Second)"""
    cells = query_len * ref_len
    return (cells / time_seconds) / 1e9

def benchmark_implementation(name, executable_path, query, reference, num_runs=5):
    """Benchmark a single implementation multiple times"""
    print(f"\nBenchmarking {name}...")
    print(f"Query length: {len(query)}")
    print(f"Reference length: {len(reference)}")
    
    times = []
    for i in range(num_runs):
        print(f"\nRun {i+1}/{num_runs}")
        execution_time = run_implementation(executable_path, query, reference)
        if execution_time is not None:
            times.append(execution_time)
            print(f"Time: {execution_time:.4f} seconds")
    
    if not times:
        return None
    
    avg_time = np.mean(times)
    gcups = calculate_gcups(len(query), len(reference), avg_time)
    std_dev = np.std(times)
    
    print(f"\nResults for {name}:")
    print(f"Average time: {avg_time:.4f} seconds")
    print(f"GCUPS: {gcups:.4f}")
    print(f"Standard deviation: {std_dev:.4f}")
    
    return {
        'name': name,
        'avg_time': avg_time,
        'gcups': gcups,
        'std_dev': std_dev
    }

def main():
    # Generate shorter sequences due to MAX_SEQ_LENGTH limit
    print("Generating random test sequences...")
    query = generate_random_sequence(500)  # Shorter query sequence
    ref = generate_random_sequence(2000)   # Shorter reference sequence
    
    print(f"\nTest sequences:")
    print(f"Query: {query[:50]}... (length: {len(query)})")
    print(f"Reference: {ref[:50]}... (length: {len(ref)})")
    
    # Define implementations to benchmark
    implementations = [
        ('Serial', str(project_root / 'benchmark/benchmark_iteration_1/serial/sw_serial_omp')),
        ('RowOMP', str(project_root / 'benchmark/benchmark_iteration_1/rowomp/rowparallel_omp')),
        ('AntiOMP', str(project_root / 'benchmark/benchmark_iteration_1/antiomp/sw_anti_omp'))
    ]
    
    # Run benchmarks
    results = []
    for name, path in implementations:
        result = benchmark_implementation(name, path, query, ref)
        if result:
            results.append(result)
    
    # Print final results
    print("\nFinal Results:")
    print("-" * 80)
    print(f"{'Implementation':<15} {'Avg Time (s)':<15} {'GCUPS':<15} {'Std Dev (s)':<15}")
    print("-" * 80)
    
    for result in sorted(results, key=lambda x: x['gcups'], reverse=True):
        print(f"{result['name']:<15} {result['avg_time']:<15.4f} {result['gcups']:<15.4f} {result['std_dev']:<15.4f}")

if __name__ == '__main__':
    main()
