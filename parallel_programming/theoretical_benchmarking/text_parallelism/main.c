#include <iostream>
#include <cmath> // for ceil()

// Function to calculate speedup
double calculateSpeedup(int m, int n, int p) {
    // Chunk size l = (n / p) + (m - 1)
    double l = static_cast<double>(n) / p + (m - 1);

    // Sequential execution time: T_sequential = n * m
    double T_sequential = static_cast<double>(n) * m;

    // Parallel execution time: T_parallel = (n / p + (m - 1)) * m
    double T_parallel = l * m;

    // Speedup: S = T_sequential / T_parallel
    double speedup = T_sequential / T_parallel;

    return speedup;
}

int main() {
    int m, n, p;

    // Input values for read length, reference length, and number of processors
    std::cout << "Enter the read length (m): ";
    std::cin >> m;
    
    std::cout << "Enter the reference length (n): ";
    std::cin >> n;
    
    std::cout << "Enter the number of processors (p): ";
    std::cin >> p;

    // Calculate speedup
    double speedup = calculateSpeedup(m, n, p);

    // Output the result
    std::cout << "Speedup: " << speedup << std::endl;

    return 0;
}

