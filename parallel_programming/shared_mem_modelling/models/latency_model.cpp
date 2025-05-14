#include <iostream>
#include <vector>
#include <cmath> // Correct library for math functions
using namespace std;

// Function to calculate the number of elements in the i-th diagonal
int func_num_diag_elements(int i, int m, int n) {
    if (i < min(m, n)) {
        return i + 1; // First few diagonals until min(m,n) are incremental
    } else if (i < max(m, n)) {
        return min(m, n); // Middle diagonals are constant size
    } else {
        return m + n - 1 - i; // Last few diagonals decrement
    }
}

// Function to calculate memory latency based on number of elements and cores
int mem_latency_cal(int num_elements, int cores) {
    int active_cores=min(num_elements,cores);
    int max_ele_per_core = ((num_elements +active_cores-1)/ active_cores); // Ensure floating-point division
    int loads=3+(max_ele_per_core-1)*2;
    int stores=max_ele_per_core;
    int num_load_stores = loads+stores;
    std::cout<<"no of loads/stores are"<<num_load_stores<<endl;
    int latency = num_load_stores + 5; // Base latency + overhead
    return latency;
}

// Function to calculate computation latency based on number of elements and cores
int comp_latency_cal(int num_elements, int cores) {
    int active_cores=min(num_elements,cores);
    int max_ele_per_core = ((num_elements +active_cores-1)/ active_cores); // Ensure floating-point division
    int comp_latency = 22 * max_ele_per_core; // 22 cycles per computation
    return comp_latency;
}

int main() {
    // Matrix dimensions
    int m, n;
    int cores;
    cout << "Enter number of rows (m): " << endl;        
    cin >> m;
    cout << "Enter number of columns (n): " << endl;        
    cin >> n;
    cout << "Enter number of cores: " << endl;        
    cin >> cores;


    int total_mem_latency = 0;
    int total_compute_latency = 0;
    vector<pair<int, int>> diag_latency;
    int total_diagonals = n + m - 1; // Total number of diagonals

    // Loop over all diagonals
    for (int i = 0; i < total_diagonals; i++) {
        // Calculate number of elements in i-th diagonal
        int num_diag_elements = func_num_diag_elements(i, m, n);

        // Calculate memory latency and computation latency
        int mem_latency = mem_latency_cal(num_diag_elements, cores);
        int compute_latency = comp_latency_cal(num_diag_elements, cores);

        // Accumulate total latencies
        total_mem_latency += mem_latency;
        total_compute_latency += compute_latency;

        // Store latencies for this diagonal
        diag_latency.push_back({mem_latency, compute_latency});

        // Print out details for this diagonal
        cout << "Diagonal " << i + 1 << " has " << num_diag_elements 
             << " elements, Mem Latency: " << mem_latency 
             << ", Compute Latency: " << compute_latency << endl;
    }

    // Calculate total latency
    int total_latency = total_mem_latency + total_compute_latency;

    // Print total results
    cout << "\nTotal Memory Latency: " << total_mem_latency << endl;
    cout << "Total Compute Latency: " << total_compute_latency << endl;
    cout << "Total Latency: " << total_latency << endl;

    return 0;
}

