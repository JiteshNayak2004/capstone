#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "synchronization.h"

// Configuration parameters
//#define MAX_SEQ_LENGTH 40
#define MATCH_SCORE 2       // Score for a match
#define MISMATCH_SCORE -1   // Penalty for a mismatch
#define GAP_SCORE -2        // Penalty for a gap
#define CORE_COUNT 1        // Number of cores for parallelization
#define MAX_SEQ_LENGTH_1 100
#define MAX_SEQ_LENGTH_2 40

// Sequences to align
const char seq1[] = "CTTGTCGCTCGTGCACGCTTTCAATTTCAGCTTGTCTTCTCGTTTCGATATTAGGTCTCCCCTACATAATTAGAGGGGAGAACGAGTTCGTAGCAGCATA";
const char seq2[] = "ACAAAAAGGCAGTGGGATTTTAGTGAGTAAAGGGGGACCC";
int seq1_len = sizeof(seq1) - 1;
int seq2_len = sizeof(seq2) - 1;

// DP matrix and temporary arrays
int16_t dp[MAX_SEQ_LENGTH_1+1][MAX_SEQ_LENGTH_2+1] __attribute__((section(".l1")))={0};
int16_t hPrime[MAX_SEQ_LENGTH_1+1] __attribute__((section(".l1")))={0};
int16_t gapArray[MAX_SEQ_LENGTH_1+1] __attribute__((section(".l1")))={0};
int16_t prefixOut[MAX_SEQ_LENGTH_1+1] __attribute__((section(".l1")))={0};
int16_t Lij[MAX_SEQ_LENGTH_1+1] __attribute__((section(".l1")))={0};

// Utility functions
int16_t max(int16_t a, int16_t b) {
  return (a > b) ? a : b;
}

// Compute H intermediate: max(rowIPrev[j] + GAP_SCORE, rowIPrev[j-1] + match/mismatch, 0)
int16_t HIntermediate(int16_t *rowIPrev, int j, int i) {
  int16_t matchMismatch = (seq1[j - 1] == seq2[i - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
  int16_t score1 = rowIPrev[j] + GAP_SCORE;
  int16_t score2 = rowIPrev[j - 1] + matchMismatch;
  int16_t score3 = 0;
  return max(max(score1, score2), score3);
}

// Compute final H value: max(finalInput, j * GAP_SCORE)
int16_t HFinal(int16_t finalInput, int j) {
  return max(finalInput, j * GAP_SCORE);
}

// Compute prefix max of inputArray
void prefixMax(int16_t *inputArray, int16_t *outputArray, int length) {
  outputArray[0] = inputArray[0];
  for (int i = 1; i < length; i++) {
    outputArray[i] = max(outputArray[i - 1], inputArray[i]);
  }
}

// Segment computation for a portion of a row
void compute_row_segment(int i, int start_col, int end_col) {
  // PART 1 h'ij computation
  // Compute H intermediate values for the segment
  for (int j = start_col; j < end_col; j++) {
    hPrime[j - 1] = HIntermediate(dp[i - 1], j, i);
  }

  // PART 2 lij computation
  // Create modified h'ij
  // Find prefix max scan of h'ij
  // Create a gap errors array
  // Add output of prefix max scan and gap errors; this is lij

  // Preparing input to prefix max
  for (int j = start_col - 1; j < end_col - 1; j++) {
    hPrime[j] += ((seq1_len - j - 2) * GAP_SCORE);
    gapArray[j] = -((seq1_len - j - 2) * GAP_SCORE);
  }

  // Compute prefix maximums of hij
  prefixMax(hPrime + (start_col - 1), prefixOut + (start_col - 1), end_col - start_col);

  // Compute Lij values
  for (int j = start_col - 1; j < end_col - 1; j++) {
    Lij[j] = prefixOut[j] + gapArray[j];
  }

  // PART 3
  // Compute final H values for row segment
  for (int j = start_col - 1; j < end_col - 1; j++) {
    dp[i][j + 1] = HFinal(Lij[j], j);
  }
}

// Traceback to reconstruct the alignment
void traceback(int16_t dp[MAX_SEQ_LENGTH_1+1][MAX_SEQ_LENGTH_2+1], int seq1_len, int seq2_len) {
  int i = seq2_len, j = seq1_len;
  char aligned_seq1[MAX_SEQ_LENGTH_1 + MAX_SEQ_LENGTH_2 + 1];
  char aligned_seq2[MAX_SEQ_LENGTH_1 + MAX_SEQ_LENGTH_2 + 1];
  int index = 0;

  while (i > 0 && j > 0) {
    if (dp[i][j] == 0) {
      break; // End of traceback
    }

    if (seq1[j - 1] == seq2[i - 1] && dp[i][j] == dp[i - 1][j - 1] + MATCH_SCORE) {
      aligned_seq1[index] = seq1[j - 1];
      aligned_seq2[index] = seq2[i - 1];
      i--;
      j--;
    } else if (dp[i][j] == dp[i][j - 1] + GAP_SCORE) {
      aligned_seq1[index] = seq1[j - 1];
      aligned_seq2[index] = '-';
      j--;
    } else {
      aligned_seq1[index] = '-';
      aligned_seq2[index] = seq2[i - 1];
      i--;
    }
    index++;
  }

  // Reverse the alignment strings
  aligned_seq1[index] = '\0';
  aligned_seq2[index] = '\0';
  for (int k = 0; k < index / 2; k++) {
    char temp = aligned_seq1[k];
    aligned_seq1[k] = aligned_seq1[index - k - 1];
    aligned_seq1[index - k - 1] = temp;

    temp = aligned_seq2[k];
    aligned_seq2[k] = aligned_seq2[index - k - 1];
    aligned_seq2[index - k - 1] = temp;
  }

  printf("Aligned Sequence 1: %s\n", aligned_seq1);
  printf("Aligned Sequence 2: %s\n", aligned_seq2);
}

// Smith-Waterman row-parallel processing
void smith_waterman(uint32_t core_id) {
  // Determine work distribution for rows
  int total_rows = seq2_len;
  int elements_per_core = (seq1_len + CORE_COUNT - 1) / CORE_COUNT;

  // Distribute row computation among cores
  int start_col = core_id * elements_per_core+1;
  int end_col = (core_id == CORE_COUNT - 1) ?
    seq1_len + 1 :
    (core_id + 1) * elements_per_core + 1;

  mempool_barrier(CORE_COUNT);

  for (int i = 1; i <= total_rows; i++) {
    mempool_barrier(CORE_COUNT);
    compute_row_segment(i, start_col, end_col);
    mempool_barrier(CORE_COUNT);
  }
  mempool_barrier(CORE_COUNT);
}

int main() {
  uint32_t core_id = mempool_get_core_id();

  // Initialize synchronization
  mempool_barrier_init(core_id);

  // Park excess cores
  if (core_id >= CORE_COUNT) {
    while(1);
  }

  // Initialize DP matrix by core 0
  if (core_id == 0) {
    printf("Sequence 1: %s (Length: %d)\n", seq1, seq1_len);
    printf("Sequence 2: %s (Length: %d)\n", seq2, seq2_len);
  }

  // Synchronize before computation
  mempool_barrier(CORE_COUNT);

  // Measure total kernel execution time
  uint32_t start_cycles = mempool_get_timer();
  smith_waterman(core_id);
  uint32_t end_cycles = mempool_get_timer();

  // Final synchronization and reporting
  mempool_barrier(CORE_COUNT);

  if (core_id == 0) {
    traceback(dp, seq1_len, seq2_len);
  }

  printf("start and end cycles are %d and %d\n", start_cycles, end_cycles);
  wake_up(core_id+1);
  mempool_barrier(CORE_COUNT);
  return 0;
}

