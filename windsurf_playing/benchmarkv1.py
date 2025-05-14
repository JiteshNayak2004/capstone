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

def generate_random_sequence(length):
    """Generate a random DNA sequence of given length"""
    return ''.join(random.choice('ACGT') for _ in range(length))

class SmithWatermanBenchmark:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.implementations = {
            'benchmark_iteration1': {
                'Serial': 'benchmark/benchmark_iteration_1/serial/sw_serial_omp',
                'RowOMP': 'benchmark/benchmark_iteration_1/rowomp/rowparallel_omp',
                'AntiOMP': 'benchmark/benchmark_iteration_1/antiomp/sw_anti_omp'
            },
            'benchmark_iteration2': {
                'RowOMP_v2': 'benchmark/benchmark_iteration2/rowomp/rowparallel_omp_v2',
                'AntiOMP_v2': 'benchmark/benchmark_iteration2/antiomp/sw_anti_omp_v2'
            }
        }
        
        # Create temporary directory for sequence files
        self.temp_dir = Path(tempfile.mkdtemp(prefix='sw_benchmark_'))
    
    def __del__(self):
        """Cleanup temporary files"""
        if hasattr(self, 'temp_dir') and self.temp_dir.exists():
            shutil.rmtree(self.temp_dir)

    def _write_temp_sequence(self, sequence):
        """Write sequence to a temporary file and return the path"""
        temp_file = self.temp_dir / f"seq_{hash(sequence)}.txt"
        temp_file.write_text(sequence)
        return str(temp_file)

    def run_implementation(self, impl_path, query, reference, num_threads=4):
        """Run a single implementation and measure execution time"""
        try:
            # Write sequences to temporary files
            query_file = self._write_temp_sequence(query)
            ref_file = self._write_temp_sequence(reference)
            
            # Ensure executable exists and is executable
            impl_path = self.project_root / impl_path
            if not impl_path.exists():
                print(f"Error: Implementation not found at {impl_path}")
                return None
            
            if not os.access(impl_path, os.X_OK):
                impl_path.chmod(0o755)
            
            print(f"Running {impl_path.name} with:")
            print(f"  Query file: {query_file}")
            print(f"  Reference file: {ref_file}")
            print(f"  Threads: {num_threads}")
            
            # Run the implementation with a longer timeout for larger sequences
            start_time = time.time()
            result = subprocess.run(
                [str(impl_path), query_file, ref_file, str(num_threads)],
                capture_output=True,
                check=True,
                text=True,
                timeout=300  # 5-minute timeout for larger sequences
            )
            end_time = time.time()
            
            execution_time = end_time - start_time
            print(f"Output from {impl_path.name}:")
            print(result.stdout)
            
            return {
                'time': execution_time,
                'gcups': self.calculate_gcups(len(query), len(reference), execution_time),
                'output': result.stdout
            }
            
        except subprocess.TimeoutExpired:
            print(f"Error: {impl_path.name} timed out after 300 seconds")
            return None
        except subprocess.CalledProcessError as e:
            print(f"Error running {impl_path}:")
            print(f"Exit code: {e.returncode}")
            print(f"Stdout: {e.stdout}")
            print(f"Stderr: {e.stderr}")
            return None
        except Exception as e:
            print(f"Unexpected error running {impl_path}: {e}")
            return None

    @staticmethod
    def calculate_gcups(query_len, ref_len, time_seconds):
        """Calculate GCUPS (Giga Cell Updates Per Second)"""
        cells = query_len * ref_len
        return (cells / time_seconds) / 1e9

    def benchmark_implementation(self, name, executable_path, query, reference, num_runs=3):
        """Benchmark a single implementation multiple times"""
        print(f"\nBenchmarking {name}...")
        print(f"Query length: {len(query)}")
        print(f"Reference length: {len(reference)}")
        
        results = []
        for i in range(num_runs):
            print(f"\nRun {i+1}/{num_runs}")
            result = self.run_implementation(executable_path, query, reference)
            if result:
                results.append(result)
                print(f"Time: {result['time']:.4f} seconds")
                print(f"GCUPS: {result['gcups']:.4f}")
        
        if not results:
            return None
        
        times = [r['time'] for r in results]
        gcups = [r['gcups'] for r in results]
        
        return {
            'name': name,
            'avg_time': np.mean(times),
            'avg_gcups': np.mean(gcups),
            'std_time': np.std(times),
            'std_gcups': np.std(gcups),
            'min_time': min(times),
            'max_gcups': max(gcups)
        }

    def run_benchmarks(self, query, reference):
        """Run benchmarks for all implementations"""
        results = []
        
        # Benchmark each implementation category
        for category, impls in self.implementations.items():
            print(f"\nRunning {category} implementations:")
            for name, path in impls.items():
                result = self.benchmark_implementation(name, path, query, reference)
                if result:
                    result['category'] = category
                    results.append(result)
        
        return results

    def print_results(self, results):
        """Print benchmark results in a formatted table"""
        if not results:
            print("\nNo successful benchmark results to display.")
            return
            
        print("\nBenchmark Results:")
        print("-" * 100)
        headers = ["Implementation", "Category", "Avg Time(s)", "Avg GCUPS", "Max GCUPS", "Std Time", "Std GCUPS"]
        print(f"{headers[0]:<15} {headers[1]:<20} {headers[2]:<12} {headers[3]:<12} {headers[4]:<12} {headers[5]:<12} {headers[6]:<12}")
        print("-" * 100)
        
        # Sort by average GCUPS (descending)
        for r in sorted(results, key=lambda x: x['avg_gcups'], reverse=True):
            print(f"{r['name']:<15} {r['category']:<20} {r['avg_time']:<12.4f} {r['avg_gcups']:<12.4f} "
                  f"{r['max_gcups']:<12.4f} {r['std_time']:<12.4f} {r['std_gcups']:<12.4f}")

def main():
    # Initialize benchmark system
    project_root = Path(__file__).parent
    benchmark = SmithWatermanBenchmark(project_root)
    
    # Generate test sequences
    print("Generating test sequences...")
    
    # Generate sequences of different sizes for thorough benchmarking
    sequence_pairs = [
        (generate_random_sequence(500), generate_random_sequence(2000)),  # Small
    ]
    
    # Run benchmarks for each sequence pair
    for i, (query, ref) in enumerate(sequence_pairs, 1):
        print(f"\nBenchmark Set {i}")
        print(f"Query length: {len(query)}")
        print(f"Reference length: {len(ref)}")
        
        results = benchmark.run_benchmarks(query, ref)
        benchmark.print_results(results)

if __name__ == '__main__':
    main()
