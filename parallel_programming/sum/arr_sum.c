#include <stdint.h>
#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

// Number of elements in the array
#define NUM_ELEMENTS 100

// Array of elements
int32_t array[NUM_ELEMENTS];

// Function to initialize the array with values 1 to 100
void initialize_array() {
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        array[i] = i + 1;
    }
}

// Function to compute the sum of a segment of the array from start to end
int32_t compute_sum(uint32_t core_id, uint32_t num_cores) {
    uint32_t elements_per_core = NUM_ELEMENTS / num_cores;
    uint32_t start = core_id * elements_per_core;
    uint32_t end;

    if(core_id==num_cores-1){
        end=NUM_ELEMENTS;
    }
    else{
        end=(core_id+1)*elements_per_core;
    }

    int32_t local_sum = 0;
    for (uint32_t i = start; i < end; i++) {
        local_sum += array[i];
    }
    return local_sum;
}

int main() {
    uint32_t core_id = mempool_get_core_id();
    uint32_t num_cores =16;

    // Initialize synchronization variables
    mempool_barrier_init(core_id);

    // Initialize the array by core 0
    if (core_id == 0) {
        initialize_array();
    }

    mempool_timer_t start_cycles,end_cycles,cycle_count;


    // Synchronize to ensure array is initialized
    mempool_barrier(num_cores);

    // Each core computes the sum of its segment
    int32_t local_sum = compute_sum(core_id, num_cores);

    // Array to store local sums of each core
    static int32_t sums[10];
    sums[core_id] = local_sum;

    // Synchronize to ensure all cores have computed their local sums
    mempool_barrier(num_cores);

    //starting timer
    
    start_cycles=mempool_get_timer();

    // Core 0 aggregates the sums from all cores
    if (core_id == 0) {
        int32_t total_sum = 0;
        for (uint32_t i = 0; i < num_cores; i++) {
            total_sum += sums[i];
        }
        end_cycles=mempool_get_timer();
        cycle_count=end_cycles-start_cycles;

        printf("Total Sum: %d\n", total_sum); // Should print 5050 for an array with values 1 to 100
        printf("no of cycles %d\n",cycle_count);
    }

    // Synchronize before ending the program
    mempool_barrier(num_cores);

    return 0;
}

