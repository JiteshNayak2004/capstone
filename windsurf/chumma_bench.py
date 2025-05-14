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
import re

# Binary paths - fill these in manually
SSW_BINARY = "./external/ssw/ssw_test"  # Fill in the path to SSW binary
SWIPE_BINARY = "./external/swipe/swipe"   # Fill in the path to SWIPE binary

def generate_random_sequence(length):
    """Generate a random DNA sequence of given length"""
    return ''.join(random.choice('ACGT') for _ in range(length))

class SmithWatermanBenchmark:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.database_dir = self.project_root / 'database'
        self.create_database_dir()
    
    def create_database_dir(self):
        """Create a directory to store sequence files"""
        self.database_dir.mkdir(exist_ok=True)
    
    def _write_fasta_sequence(self, sequence, name="seq"):
        """Write a sequence in FASTA format and return the file path"""
        path = self.database_dir / f"{name}.fasta"
        with open(path, 'w') as f:
            f.write(f">{name}\n{sequence}\n")
        return str(path)

    def extract_execution_time(self, output, implementation):
        """Extract execution time from implementation output"""
        try:
            if implementation == "Serial":
                match = re.search(r"Execution time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "RowOMP":
                match = re.search(r"Execution time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "AntiOMP":
                match = re.search(r"Execution time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "RowOMP_v2":
                match = re.search(r"Execution time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "AntiOMP_v2":
                match = re.search(r"Execution time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "SSW":
                match = re.search(r"SSW alignment time: ([\d.]+) seconds", output)
                return float(match.group(1)) if match else None
            elif implementation == "SWIPE":
                match = re.search(r"Speed:\s+([\d.]+) GCUPS", output)
                if match:
                    gcups = float(match.group(1))
                    # Calculate time based on GCUPS
                    return (self.query_len * self.ref_len) / (gcups * 1e9)
                return None
        except Exception as e:
            print(f"Error extracting execution time for {implementation}: {str(e)}")
            return None

    def calculate_gcups(self, query_len, ref_len, time_taken):
        """Calculate GCUPS (Billion Cell Updates Per Second)"""
        cells = query_len * ref_len
        return (cells / time_taken) / 1e9 if time_taken > 0 else 0

    def run_implementation(self, name, impl_path, query_file, ref_file, num_threads=2):
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
                # Create BLAST database
                makeblastdb_cmd = [
                    "makeblastdb",
                    "-in", ref_file,
                    "-dbtype", "nucl",
                    "-parse_seqids",
                    "-blastdb_version", "4"
                ]
                subprocess.run(makeblastdb_cmd, check=True, capture_output=True)
                
                cmd = [
                    impl_path,
                    "-d", ref_file,
                    "-i", query_file,
                    "-p", "0",
                    "-r", "1",
                    "-q", "-3",
                    "-G", "5",
                    "-E", "2",
                    "-a", str(num_threads)
                ]
            else:
                cmd = [impl_path, query_file, ref_file, str(num_threads)]
            
            # Run the implementation
            result = subprocess.run(
                cmd,
                capture_output=True,
                check=True,
                text=True,
                timeout=300
            )
            
            # Extract execution time from output
            execution_time = self.extract_execution_time(result.stdout, name)
            if execution_time is None:
                print(f"Warning: Could not extract execution time for {name}")
                return None
            
            print(f"Output from {name}:")
            print(result.stdout)
            
            # Calculate GCUPS
            gcups = self.calculate_gcups(self.query_len, self.ref_len, execution_time)
            
            return {
                'time': execution_time,
                'gcups': gcups,
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
    
    def run_benchmark(self, query_length, ref_length, num_runs=1, thread_counts=None):
        """Run benchmark for all implementations"""
        if thread_counts is None:
            thread_counts = [2]
        
        # Store sequence lengths as instance variables for GCUPS calculation
        self.query_len = query_length
        self.ref_len = ref_length
        
        print(f"\nBenchmark Set")
        print(f"Query length: {query_length}")
        print(f"Reference length: {ref_length}")
        
        # Generate test sequences
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
            'SSW': ('external', SSW_BINARY),
            'SWIPE': ('external', SWIPE_BINARY)
        }
        
        results = {}
        
        for num_threads in thread_counts:
            print(f"\nRunning benchmarks with {num_threads} threads...")
            for name, (category, path) in implementations.items():
                print(f"\nBenchmarking {name}...")
                
                times = []
                gcups = []
                
                for i in range(num_runs):
                    print(f"Run {i+1}/{num_runs}")
                    result = self.run_implementation(name, str(path), query_file, ref_file, num_threads)
                    
                    if result:
                        times.append(result['time'])
                        gcups.append(result['gcups'])
                        print(f"Time: {result['time']:.6f} seconds")
                        print(f"GCUPS: {result['gcups']:.4f}\n")
                    else:
                        print(f"Failed to run {name}\n")
                        break
                
                if times:
                    results.setdefault(num_threads, {})[name] = {
                        'category': category,
                        'avg_time': np.mean(times),
                        'avg_gcups': np.mean(gcups),
                        'max_gcups': np.max(gcups),
                        'min_gcups': np.min(gcups),
                        'std_gcups': np.std(gcups)
                    }
        
        # Print results table
        if results:
            print("\nBenchmark Results:")
            print("-" * 120)
            headers = ['Threads', 'Implementation', 'Category', 'Avg Time(s)', 'Avg GCUPS', 'Max GCUPS', 'Min GCUPS', 'Std Dev']
            print(f"{headers[0]:<10} {headers[1]:<15} {headers[2]:<20} {headers[3]:<15} {headers[4]:<12} {headers[5]:<12} {headers[6]:<12} {headers[7]:<12}")
            print("-" * 120)
            
            for num_threads, thread_results in results.items():
                sorted_results = sorted(thread_results.items(), key=lambda x: x[1]['avg_gcups'], reverse=True)
                for name, data in sorted_results:
                    print(f"{num_threads:<10} {name:<15} {data['category']:<20} "
                          f"{data['avg_time']:<15.6f} {data['avg_gcups']:<12.4f} "
                          f"{data['max_gcups']:<12.4f} {data['min_gcups']:<12.4f} "
                          f"{data['std_gcups']:<12.4f}")
        
        return results

def main():
    project_root = Path(__file__).parent
    benchmark = SmithWatermanBenchmark(project_root)
    
    try:
        # Define thread counts to test
        thread_counts = [2]
        
        # Run benchmark with moderate-sized sequences
        benchmark.run_benchmark(
            query_length=400, 
            ref_length=10000, 
            num_runs=3, 
            thread_counts=thread_counts
        )
    finally:
        pass

if __name__ == "__main__":
    main()
