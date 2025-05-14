#include <stdint.h>
#include "encoding.h"
#include <stdio.h>
#include <stdbool.h>
#include "runtime.h"
#include "synchronization.h"

// Define the number of cores to use
#define NUM_CORES 16

// Function to split the reference string (simplified for this example)
void split_reference_string(const char* reference_string, int pattern_length, char** segments, int num_cores) {
    int ref_length = strlen(reference_string);
    int segment_length = ref_length / num_cores + pattern_length - 1;

    for (int i = 0; i < num_cores; i++) {
        int start = i * (ref_length / num_cores);
        int length = (i == num_cores - 1) ? (ref_length - start) : segment_length;
        
        strncpy(segments[i], reference_string + start, length);
        segments[i][length] = '\0';
    }
}

// Global variables
char* reference_text;
char* pattern;
int k;
char* segments[NUM_CORES];
int results[NUM_CORES];

int main() {
    uint32_t core_id = mempool_get_core_id();
    
    // Initialize synchronization
    mempool_barrier_init(core_id);
    
    // Core 0 initializes the data
    if (core_id == 0) {
        reference_text = "AAAGAAAAGAATTTTCAACCCAGAATTTCATATCCAGCCAAACTACGCTTCATAAGTGAAGGAGAAATAAAATCCTTTACAGACAAGCAAATGCTGAGAG";
        pattern = "AAAGAAAAGAATTTTCAACCCAGAATTTCATATCCAGCCAAACAAAGCTTCATAAGTGAAGGAGAAATAAATCCTTTACAGAGAAGCAAATGCTGAGAGA";
        k = 6;

        // Allocate memory for segments
        for (int i = 0; i < NUM_CORES; i++) {
            segments[i] = malloc(strlen(reference_text) + 1);
        }

        // Split the reference string
        split_reference_string(reference_text, strlen(pattern), segments, NUM_CORES);
        
        printf("Data initialized and split\n");
    }
    
    // Synchronize to ensure all data is initialized
    mempool_barrier(NUM_CORES);
    
    // Each core processes its segment
    if (core_id < NUM_CORES) {
        results[core_id] = genasm_filter(segments[core_id], pattern, k);
    }
    
    // Synchronize after processing
    mempool_barrier(NUM_CORES);
    
    // Core 0 combines the results
    if (core_id == 0) {
        int final_result = 0;
        for (int i = 0; i < NUM_CORES; i++) {
            if (results[i] == 1) {
                final_result = 1;
                break;
            }
        }
        printf("Final result: %d\n", final_result);

        // Clean up
        for (int i = 0; i < NUM_CORES; i++) {
            free(segments[i]);
        }
    }
    
    // Synchronize before ending
    mempool_barrier(NUM_CORES);
    
    return 0;
}
