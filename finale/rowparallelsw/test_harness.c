#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "runtime.h"
#include "synchronization.h"
#include "test_sequences.h"

// Function declarations for different implementations
extern void smith_waterman(uint32_t core_id);
extern void init_smith_waterman(const char* sequence1, const char* sequence2);

// Performance tracking
static uint32_t total_cycles = 0;
static uint32_t total_runs = 0;

void run_test(const char* query, const char* ref) {
    printf("\nRunning test with sequences:\nQuery (%d): %s\nRef (%d): %s\n", 
           (int)strlen(query), query, (int)strlen(ref), ref);

    uint32_t start_time = mempool_get_timer();
    
    // Initialize the sequences
    init_smith_waterman(query, ref);
    
    // Run on all cores
    uint32_t num_cores = mempool_get_core_count();
    for (uint32_t i = 0; i < num_cores; i++) {
        if (mempool_get_core_id() == i) {
            smith_waterman(i);
        }
    }
    
    uint32_t end_time = mempool_get_timer();
    uint32_t cycles = end_time - start_time;
    
    total_cycles += cycles;
    total_runs++;
    
    printf("Test completed in %u cycles\n", cycles);
}

int main() {
    // Initialize barriers
    mempool_barrier_init(mempool_get_core_id());
    
    // Run main test pair
    run_test(query_seq, ref_seq);
    
    // Run additional test pairs
    for (int i = 0; i < NUM_TEST_PAIRS; i++) {
        run_test(test_pairs[i].query, test_pairs[i].ref);
    }
    
    // Print summary
    if (mempool_get_core_id() == 0) {
        printf("\nTest Summary:\n");
        printf("Total runs: %u\n", total_runs);
        printf("Average cycles per run: %u\n", total_cycles / total_runs);
        printf("Total cycles: %u\n", total_cycles);
    }
    
    return 0;
}
