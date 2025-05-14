#include <iostream>
#include <algorithm>  // For std::min
#include <vector>     // For std::vector

// Function to calculate the number of elements in each wavefront
std::vector<int> getWavefrontSizes(int m, int n) {
    std::vector<int> wavefront_sizes;
    int num_wavefronts = m + n - 1;

    for (int k = 1; k <= num_wavefronts; ++k) {
        if (k <= std::min(m, n)) {
            wavefront_sizes.push_back(k);
        } else {
            wavefront_sizes.push_back((m + n - 1) - (k - std::min(m, n)));
        }
    }
    
    return wavefront_sizes;
}

// Function to calculate the parallel execution time given the number of cores and wavefront sizes
double calculateParallelTime(const std::vector<int>& wavefront_sizes, int num_cores) {
    double total_parallel_time = 0.0;

    for (int size : wavefront_sizes) {
        int cores_used = std::min(size, num_cores);
        total_parallel_time += static_cast<double>(size) / cores_used;
    }

    return total_parallel_time;
}

// Function to calculate theoretical speedup
double calculateSpeedup(int m, int n, int num_cores) {
    // Total number of elements in the matrix
    int total_elements = m * n;

    // Sequential time (time to process all elements sequentially)
    double sequential_time = static_cast<double>(total_elements);

    // Get wavefront sizes
    std::vector<int> wavefront_sizes = getWavefrontSizes(m, n);

    // Calculate parallel execution time
    double parallel_time = calculateParallelTime(wavefront_sizes, num_cores);

    // Calculate speedup
    double speedup = sequential_time / parallel_time;

    return speedup;
}

int main() {
    int m, n, num_cores;

    std::cout << "Enter matrix dimensions (m n): ";
    std::cin >> m >> n;

    std::cout << "Enter number of cores: ";
    std::cin >> num_cores;

    double speedup = calculateSpeedup(m, n, num_cores);

    std::cout << "Theoretical Speedup: " << speedup << std::endl;

    return 0;
}

