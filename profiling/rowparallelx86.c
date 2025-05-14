#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>

// Configuration parameters
#define MAX_SEQ_LENGTH 100
#define MATCH_SCORE 2       // Score for a match
#define MISMATCH_SCORE -1   // Penalty for a mismatch
#define GAP_SCORE -2        // Penalty for a gap
#define CORE_COUNT 2        // Number of threads for parallelization

// Sequences to align
const char seq1[] = "TTTTAGATAGGCCGCAGGTATAATACATCGCTGCAGCCCAGGGCCGCTCTCCCCGTATGATAGCATGGGG";
const char seq2[] = "TCAGTATGGCGATTTCTCGGCTCGTCATGT";
int seq1_len = sizeof(seq1) - 1;
int seq2_len = sizeof(seq2) - 1;

// DP matrix and temporary arrays
int16_t dp[MAX_SEQ_LENGTH + 1][MAX_SEQ_LENGTH + 1];
int16_t hPrime[MAX_SEQ_LENGTH];
int16_t gapArray[MAX_SEQ_LENGTH];
int16_t prefixOut[MAX_SEQ_LENGTH];
int16_t Lij[MAX_SEQ_LENGTH];

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
  for (int j = start_col; j < end_col; j++) {
    hPrime[j - 1] = HIntermediate(dp[i - 1], j, i);
  }

  // PART 2 lij computation
  for (int j = start_col - 1; j < end_col - 1; j++) {
    hPrime[j] += ((seq1_len - j - 2) * GAP_SCORE);
    gapArray[j] = -((seq1_len - j - 2) * GAP_SCORE);
  }

  prefixMax(hPrime + (start_col - 1), prefixOut + (start_col - 1), end_col - start_col);

  for (int j = start_col - 1; j < end_col - 1; j++) {
    Lij[j] = prefixOut[j] + gapArray[j];
  }

  // PART 3
  for (int j = start_col - 1; j < end_col - 1; j++) {
    dp[i][j + 1] = HFinal(Lij[j], j);
  }
}

// Smith-Waterman row-parallel processing
void smith_waterman() {
  #pragma omp parallel num_threads(CORE_COUNT)
  {
    int core_id = omp_get_thread_num();

    int elements_per_core = seq1_len / CORE_COUNT;
    int start_col = core_id * elements_per_core + 1;
    int end_col = (core_id == CORE_COUNT - 1) ? seq1_len + 1 : (core_id + 1) * elements_per_core + 1;

    for (int i = 1; i <= seq2_len; i++) {
      #pragma omp barrier

      compute_row_segment(i, start_col, end_col);

      #pragma omp barrier

      #pragma omp single
      {
        uint32_t row_compute_time = 0; // Placeholder for timing calculations
        uint32_t row_sync_time = 0;    // Placeholder for timing calculations

        printf("Row %d - Compute time: %u, Sync time: %u \n", i, row_compute_time, row_sync_time);
      }
    }
  }
}

int main() {
  memset(dp, 0, sizeof(dp));
  printf("Sequence 1: %s (Length: %d)\n", seq1, seq1_len);
  printf("Sequence 2: %s (Length: %d)\n", seq2, seq2_len);

  double start_time = omp_get_wtime();
  smith_waterman();
  double end_time = omp_get_wtime();

  printf("Total execution time: %f seconds\n", end_time - start_time);

  // Optional: print final DP matrix
  // for (int i = 0; i <= seq2_len; i++) {
  //   for (int j = 0; j <= seq1_len; j++) {
  //     printf("%d ", dp[i][j]);
  //   }
  //   printf("\n");
  // }

  return 0;
}

