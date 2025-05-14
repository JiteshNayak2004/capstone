#!/usr/bin/env python3

import os
import time
import subprocess
import numpy as np
from pathlib import Path
import sys
import shutil
import tempfile
import random

# Binary paths - fill these in manually
SSW_BINARY = "./external/ssw/ssw_test"  # Fill in the path to SSW binary
SWIPE_BINARY = "./external/swipe/swipe"   # Fill in the path to SWIPE binary

def generate_random_sequence(length):
    """Generate a random DNA sequence of given length"""
    return ''.join(random.choice('ACGT') for _ in range(length))

class SmithWatermanBenchmark:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.temp_dir = None
        self.create_temp_dir()
    
    def create_temp_dir(self):
        """Create a temporary directory for sequence files"""
        self.temp_dir = tempfile.mkdtemp(prefix='sw_benchmark_')
    
    def cleanup(self):
        """Clean up temporary files"""
        if self.temp_dir and os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)
    
    def _write_fasta_sequence(self, sequence, name="seq"):
        """Write a sequence in FASTA format and return the file path"""
        fd, path = tempfile.mkstemp(prefix='seq_', suffix='.fasta', dir=self.temp_dir, text=True)
        with os.fdopen(fd, 'w') as f:
            f.write(f">{name}\n{sequence}\n")
        return path

    def calculate_gcups(self, query_len, ref_len, time_taken):
        """Calculate GCUPS (Billion Cell Updates Per Second)"""
        cells = query_len * ref_len
        return (cells / time_taken) / 1e9 if time_taken > 0 else 0

    def run_implementation(self, name, impl_path, query_file, ref_file, num_threads=1):
        """Run a single implementation and measure execution time"""
        try:
            # Ensure executable exists and is executable
            if not os.path.exists(impl_path):
                print(f"Error: Implementation not found at {impl_path}")
                return None
            
            if not os.access(impl_path, os.X_OK):
                os.chmod(impl_path, 0o755)
            
            print(f"Running {name} with:")
            print(f"  Query file: {query_file}")
            print(f"  Reference file: {ref_file}")
            print(f"  Threads: {num_threads}")
            
            # Prepare command based on implementation
            if name == "SSW":
                cmd = [impl_path, ref_file, query_file]
            elif name == "SWIPE":
                # Create BLAST database for SWIPE with version 4
                makeblastdb_cmd = [
                    "makeblastdb",
                    "-in", ref_file,
                    "-dbtype", "nucl",
                    "-parse_seqids",
                    "-blastdb_version", "4"
                ]
                try:
                    subprocess.run(makeblastdb_cmd, check=True, capture_output=True)
                except subprocess.CalledProcessError as e:
                    print(f"Error creating BLAST database: {e.stderr.decode()}")
                    raise
                
                # SWIPE command with DNA scoring parameters
                cmd = [
                    impl_path,
                    "-d", ref_file,    # database file (requires BLAST format)
                    "-i", query_file,  # query file (FASTA format)
                    "-p", "0",         # blastn mode (nucleotide vs nucleotide)
                    "-r", "1",         # match reward
                    "-q", "-3",        # mismatch penalty
                    "-G", "5",         # gap open penalty
                    "-E", "2",         # gap extension penalty
                    "-a", str(num_threads)  # number of threads
                ]
            else:  # Internal implementations
                cmd = [impl_path, query_file, ref_file, str(num_threads)]
            
            # Run the implementation with a timeout
            start_time = time.time()
            result = subprocess.run(
                cmd,
                capture_output=True,
                check=True,
                text=True,
                timeout=300  # 5-minute timeout for larger sequences
            )
            end_time = time.time()
            
            execution_time = end_time - start_time
            print(f"Output from {name}:")
            print(result.stdout)
            
            # Get sequence lengths
            with open(query_file) as f:
                query_len = len(''.join(line.strip() for line in f.readlines()[1:]))
            with open(ref_file) as f:
                ref_len = len(''.join(line.strip() for line in f.readlines()[1:]))
            
            return {
                'time': execution_time,
                'gcups': self.calculate_gcups(query_len, ref_len, execution_time),
                'output': result.stdout
            }
            
        except subprocess.TimeoutExpired:
            print(f"Error: {name} timed out after 300 seconds")
            return None
        except subprocess.CalledProcessError as e:
            print(f"Error running {name}:")
            print(f"Exit code: {e.returncode}")
            print(f"Stdout: {e.stdout}")
            print(f"Stderr: {e.stderr}")
            return None
        except Exception as e:
            print(f"Unexpected error running {name}: {e}")
            return None
    
    def run_benchmark(self, query_length, ref_length, num_runs=3, num_threads=4):
        """Run benchmark for all implementations"""
        print(f"\nBenchmark Set 1")
        print(f"Query length: {query_length}")
        print(f"Reference length: {ref_length}")
        
        # Generate test sequences and write to FASTA files
        query = generate_random_sequence(query_length)
        reference = generate_random_sequence(ref_length)
        
        query_file = self._write_fasta_sequence(query, "query")
        ref_file = self._write_fasta_sequence(reference, "reference")
        
        # Define implementations to benchmark
        implementations = {
            'Serial': ('benchmark_iteration1', self.project_root / 'benchmark/benchmark_iteration_1/serial/sw_serial_omp'),
            'RowOMP': ('benchmark_iteration1', self.project_root / 'benchmark/benchmark_iteration_1/rowomp/rowparallel_omp'),
            'AntiOMP': ('benchmark_iteration1', self.project_root / 'benchmark/benchmark_iteration_1/antiomp/sw_anti_omp'),
            'RowOMP_v2': ('benchmark_iteration2', self.project_root / 'benchmark/benchmark_iteration2/rowomp/rowparallel_omp_v2'),
            'AntiOMP_v2': ('benchmark_iteration2', self.project_root / 'benchmark/benchmark_iteration2/antiomp/sw_anti_omp_v2'),
            'SSW': ('external', './external/ssw/ssw_test'),
            'SWIPE': ('external', './external/swipe/swipe')
        }
        
        results = {}
        
        # Run benchmarks for each implementation
        for name, (category, path) in implementations.items():
            print(f"\nBenchmarking {name}...")
            print(f"Query length: {query_length}")
            print(f"Reference length: {ref_length}\n")
            
            times = []
            gcups = []
            
            for i in range(num_runs):
                print(f"Run {i+1}/{num_runs}")
                result = self.run_implementation(name, str(path), query_file, ref_file, num_threads)
                
                if result:
                    times.append(result['time'])
                    gcups.append(result['gcups'])
                    print(f"Time: {result['time']:.4f} seconds")
                    print(f"GCUPS: {result['gcups']:.4f}\n")
                else:
                    print(f"Failed to run {name}\n")
                    break
            
            if times:
                results[name] = {
                    'category': category,
                    'avg_time': np.mean(times),
                    'avg_gcups': np.mean(gcups),
                    'max_gcups': np.max(gcups),
                    'std_time': np.std(times),
                    'std_gcups': np.std(gcups)
                }
        
        # Print results table
        if results:
            print("\nBenchmark Results:")
            print("-" * 100)
            print(f"{'Implementation':<15} {'Category':<20} {'Avg Time(s)':<12} {'Avg GCUPS':<12} "
                  f"{'Max GCUPS':<12} {'Std Time':<12} {'Std GCUPS':<12}")
            print("-" * 100)
            
            # Sort by average GCUPS (descending)
            sorted_results = sorted(results.items(), key=lambda x: x[1]['avg_gcups'], reverse=True)
            
            for name, data in sorted_results:
                print(f"{name:<15} {data['category']:<20} {data['avg_time']:<12.4f} {data['avg_gcups']:<12.4f} "
                      f"{data['max_gcups']:<12.4f} {data['std_time']:<12.4f} {data['std_gcups']:<12.4f}")
        
        return results

def main():
    project_root = Path(__file__).parent
    benchmark = SmithWatermanBenchmark(project_root)
    
    try:
        # Run benchmark with moderate-sized sequences
        benchmark.run_benchmark(query_length=40, ref_length=100, num_runs=3, num_threads=2)
    finally:
        benchmark.cleanup()

if __name__ == "__main__":
    main()
