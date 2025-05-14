#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "synchronization.h"

// Configuration parameters
#define MAX_SEQ_LENGTH 100
#define MATCH_SCORE 2       // Score for a match
#define MISMATCH_SCORE -1   // Penalty for a mismatch
#define GAP_SCORE -2        // Penalty for a gap
#define CORE_COUNT 1        // Number of cores for parallelization

// Sequences to align
const char seq1[] = "ACGTTGACGT";
const char seq2[] = "ACG";
int seq1_len = sizeof(seq1) - 1;
int seq2_len = sizeof(seq2) - 1;

// DP matrix and temporary arrays
int16_t dp[MAX_SEQ_LENGTH+1][MAX_SEQ_LENGTH+1] __attribute__((section(".l1")));
int16_t hPrime[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
int16_t gapArray[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
int16_t prefixOut[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
int16_t Lij[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
int16_t local_max_score[CORE_COUNT] __attribute__((section(".l1"))) = {0};
int16_t local_max_row[CORE_COUNT] __attribute__((section(".l1"))) = {0};
int16_t local_max_column[CORE_COUNT] __attribute__((section(".l1"))) = {0};

  // Global variables
  int global_max_score = 0;
  int max_row_index = 0;
  int max_column_index = 0;

  // Performance tracking
  int global_compute_time = 0;
  int global_sync_time = 0;
  uint32_t compute_start_times[CORE_COUNT] = {0};
  uint32_t compute_end_times[CORE_COUNT] = {0};

  // Utility functions
  int16_t max(int16_t a, int16_t b) {
    return (a > b) ? a : b;
  }

  int16_t HIntermediate(int16_t *rowIPrev, int j, int i) {
    int16_t matchMismatch = (seq1[j - 1] == seq2[i - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
    int16_t score1 = rowIPrev[j] + GAP_SCORE;
    int16_t score2 = rowIPrev[j - 1] + matchMismatch;
    return max(max(score1, score2), 0);
  }

  int16_t HFinal(int16_t finalInput, int j) {
    return max(finalInput, j * GAP_SCORE);
  }

  void prefixMax(int16_t *inputArray, int16_t *outputArray, int length) {
    outputArray[0] = inputArray[0];
    for (int i = 1; i < length; i++) {
      outputArray[i] = max(outputArray[i - 1], inputArray[i]);
    }
  }

  void compute_row_segment(int core_id, int i, int start_col, int end_col) {
    for (int j = start_col; j < end_col; j++) {
      hPrime[j - 1] = HIntermediate(dp[i - 1], j, i);
    }

    for (int j = start_col - 1; j < end_col - 1; j++) {
      hPrime[j] += ((seq1_len - j - 2) * GAP_SCORE);
      gapArray[j] = -((seq1_len - j - 2) * GAP_SCORE);
    }

    prefixMax(hPrime + (start_col - 1), prefixOut + (start_col - 1), end_col - start_col);

    for (int j = start_col - 1; j < end_col - 1; j++) {
      Lij[j] = prefixOut[j] + gapArray[j];
      dp[i][j + 1] = HFinal(Lij[j], j);
    }

    mempool_barrier(CORE_COUNT);

    if (dp[i][j + 1] > local_max_score[core_id]) {
      local_max_score[core_id] = dp[i][j + 1];
      local_max_row[core_id] = i;
      local_max_column[core_id] = j + 1;
    }


  }
  void smith_waterman(uint32_t core_id) {
    int elements_per_core = (seq1_len + CORE_COUNT - 1) / CORE_COUNT;
    int start_col = core_id * elements_per_core + 1;
    int end_col = (core_id + 1) * elements_per_core + 1;
    if (end_col > seq1_len + 1) end_col = seq1_len + 1;

    for (int i = 1; i <= seq2_len; i++) {

      compute_start_times[core_id] = mempool_get_timer();
      compute_row_segment(core_id, i, start_col, end_col);
      compute_end_times[core_id] = mempool_get_timer();

      mempool_barrier(CORE_COUNT);

      if (core_id == 0) {
        global_max_score = 0;
        for (int j = 0; j < CORE_COUNT; j++) {
          if (local_max_score[j] > global_max_score) {
            global_max_score = local_max_score[j];
            max_row_index = local_max_row[j];
            max_column_index = local_max_column[j];
          }
        }

        uint32_t earliest_start = UINT32_MAX;
        uint32_t latest_end = 0;
        for (int j = 0; j < CORE_COUNT; j++) {
          earliest_start = (compute_start_times[j] < earliest_start) ? compute_start_times[j] : earliest_start;
          latest_end = (compute_end_times[j] > latest_end) ? compute_end_times[j] : latest_end;
        }

        uint32_t row_compute_time = latest_end - earliest_start;
        uint32_t max_individual_compute = 0;
        for (int j = 0; j < CORE_COUNT; j++) {
          uint32_t core_compute_time = compute_end_times[j] - compute_start_times[j];
          max_individual_compute = (core_compute_time > max_individual_compute) ? core_compute_time : max_individual_compute;
        }

        uint32_t row_sync_time = row_compute_time - max_individual_compute;
        global_compute_time += max_individual_compute;
        global_sync_time += row_sync_time;

        printf("Row %d - Compute time: %u, Sync time: %u\n", i, max_individual_compute, row_sync_time);
      }

      mempool_barrier(CORE_COUNT);

    }

    if (core_id == 0) {
      printf("Final Matrix:\n");
      for (int i = 0; i <= seq2_len; i++) {
        for (int j = 0; j <= seq1_len; j++) {
          printf("%d ", dp[i][j]);
        }
        printf("\n");
      }

      printf("Total kernel cycles: %u\n", end_cycles - start_cycles);
      printf("Computation cycles: %d\n", global_compute_time);
      printf("Synchronization cycles: %d\n", global_sync_time);
      printf("Max score %d at index i:%d and j:%d\n", global_max_score, max_row_index, max_column_index);
    }




  }

  int main() {
    uint32_t core_id = mempool_get_core_id();

    mempool_barrier_init(core_id);

    if (core_id >= CORE_COUNT) {
      while (1);
    }

    if (core_id == 0) {
      memset(dp, 0, sizeof(dp));
      printf("Sequence 1: %s (Length: %d)\n", seq1, seq1_len);
      printf("Sequence 2: %s (Length: %d)\n", seq2, seq2_len);
    }

    mempool_barrier(CORE_COUNT);

    uint32_t start_cycles = mempool_get_timer();
    smith_waterman(core_id);
    uint32_t end_cycles = mempool_get_timer();

    mempool_barrier(CORE_COUNT);
    if(core_id==0) {

    }
    mempool_barrier(CORE_COUNT);
    return 0;
  }

