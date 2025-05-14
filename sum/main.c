#include <stdint.h>
#include "encoding.h"
#include <stdio.h>
#include <stdbool.h>
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

//core count <modify core count here not in main
#define cc 16

// Number of elements in the array
#define NUM_ELEMENTS 1600

// Array of elements
int32_t array[NUM_ELEMENTS];

// Function to initialize the array with values 1 to 200
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

  if(core_id>=num_cores-1){
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

  //numberofcores has the corecount for the 3 variant mem,min,tera useless
  uint32_t numberofcores =mempool_get_core_count();

  // cc as index as no of sums=no of cores that sum their parts
  static int32_t sums[cc];


  // Initialize synchronization variables
  mempool_barrier_init(core_id);

  // so in mempool num_cores are fixed for a config you
  // must manually sleep cores you don't require
  if (core_id >=cc) {
    while(1);}




  // Initialize the array by core 0
  if (core_id == 0) {
    initialize_array();
    printf("array has been initialized\n");

  }


  // Synchronize to ensure array is initialized
  mempool_barrier(cc);

  // this if stmt can be removed and code would work similarly
  // but used for more readibility and understanding

  uint32_t start_cycles,end_cycles;
  uint32_t cycle_count;
  start_cycles=mempool_get_timer();

  // Each core computes the sum of its segment
  int32_t local_sum = compute_sum(core_id, cc);

  end_cycles=mempool_get_timer();
  cycle_count=end_cycles-start_cycles;

  uint32_t a,b,c;
  a=500000;
  b=600000;
  uint32_t roofstart,roofend,roofperf;
  mempool_start_benchmark();
  c=a*b+b*b*b;
  a=20;

  mempool_stop_benchmark();

  roofperf=1;


  // store local sums of each core
  sums[core_id] = local_sum;

  if(core_id==0){
    printf("%d is the local sum for core %d\n",local_sum,core_id);
    printf(" the value c is %d time it took for one add is %d\n",c,roofperf); }



  // Synchronize to ensure all cores have computed their local sums
  mempool_barrier(cc);

  // Core 0 aggregates the sums from all cores

  if (core_id == 0) {

    uint32_t total_sum = 0;
    for (uint32_t i = 0; i < cc; i++) {
      total_sum += sums[i];
    }

  }

  // Synchronize
  mempool_barrier(cc);
  if(core_id!=0) {
    mempool_wfi(); // wait for interrupt basically sleeps all core until
                   // explicit wake from interrupt is done
  }

  printf("the time taken by the section of code by core %d is %d\n",core_id,cycle_count);
  wake_up(core_id+1);


  mempool_barrier(cc);
  return 0;
}

