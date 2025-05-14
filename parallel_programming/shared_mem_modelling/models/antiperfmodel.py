import math
import pdb
from typing import List, Tuple


class SmithWatermanPerformanceSimulator:
    def __init__(self,
                 seq1_length: int,
                 seq2_length: int,
                 cycles_per_cell: int = 200,
                 max_cores: int = 8):
        """
        Initialize the simulator with sequence lengths and performance parameters.

        :param seq1_length: Length of first sequence
        :param seq2_length: Length of second sequence
        :param cycles_per_cell: Computational cycles required per DP matrix cell
        :param max_cores: Maximum number of cores available
        """
        self.seq1_length = seq1_length
        self.seq2_length = seq2_length
        self.cycles_per_cell = cycles_per_cell
        self.max_cores = max_cores

        # Total diagonals in the matrix
        self.total_diagonals = seq1_length + seq2_length - 1

    def diagonal_length_calculator(self, diagonal_index: int) -> int:
        """
        Calculate the number of elements in a specific diagonal.

        :param diagonal_index: Index of the diagonal
        :return: Number of elements in the diagonal
        """
        min_dim = min(self.seq1_length, self.seq2_length)
        max_dim = max(self.seq1_length, self.seq2_length)

        if diagonal_index < min_dim:
            return diagonal_index + 1
        elif diagonal_index < max_dim:
            return min_dim
        else:
            return self.seq1_length + self.seq2_length - 1 - diagonal_index

    def simulate_diagonal_computation(self, core_count: int) -> List[Tuple[int, int]]:
        """
        Simulate diagonal computation across multiple cores.

        :param core_count: Number of cores to use
        :return: List of (diagonal_index, total_compute_cycles)
        """
        diagonal_performance = []

        for diagonal_index in range(self.total_diagonals):
            # Calculate diagonal length
            num_diagonal_elements = self.diagonal_length_calculator(
                diagonal_index)

            # Determine active cores for this diagonal
            active_cores = min(num_diagonal_elements, core_count)

            # Calculate elements per core
            elements_per_core = math.ceil(num_diagonal_elements / active_cores)

            # Compute cycles for this diagonal
            per_core_cycles = elements_per_core * self.cycles_per_cell
            max_individual_compute = per_core_cycles

            diagonal_performance.append((
                diagonal_index,
                max_individual_compute,
            ))

        return diagonal_performance

    def analyze_total_performance(self, core_count: int) -> Tuple[int, int]:
        """
        Analyze total performance for a given core count.

        :param core_count: Number of cores to use
        :return: (total_compute_cycles, total_sync_cycles, speedup_factor)
        """
        diagonal_results = self.simulate_diagonal_computation(core_count)

        total_compute_cycles = sum(result[1] for result in diagonal_results)

        # Basic speedup calculation (simplified)
        single_core_cycles = (
            self.seq1_length * self.seq2_length * self.cycles_per_cell)
        speedup_factor = single_core_cycles / (total_compute_cycles)

        return (
            total_compute_cycles,
            round(speedup_factor, 3)
        )

    def performance_comparison(self) -> None:
        """
        Compare performance across different core counts.
        """
        print("Performance Analysis for Sequences:")
        print(f"Sequence 1 Length: {self.seq1_length}")
        print(f"Sequence 2 Length: {self.seq2_length}")
        print(f"Cycles per Cell: {self.cycles_per_cell}\n")

        print("Core Count | Compute Cycles | Sync Cycles | Speedup Factor")
        print("-" * 55)

        for cores in range(1, self.max_cores + 1):
            compute, speedup = self.analyze_total_performance(cores)
            print(f"{cores:10d} | {compute:13d} | {speedup:14.2f}")


# Example Usage
if __name__ == "__main__":
    simulator = SmithWatermanPerformanceSimulator(
        seq1_length=1400,   # Example sequence length
        seq2_length=300,    # Another example sequence length
        cycles_per_cell=30,
        max_cores=64

    )

    simulator.performance_comparison()
