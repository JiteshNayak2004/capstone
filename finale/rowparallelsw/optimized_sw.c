#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "synchronization.h"

// Configuration parameters
#define MATCH_SCORE 2       // Score for a match
#define MISMATCH_SCORE -1   // Penalty for a mismatch
#define GAP_SCORE -2        // Penalty for a gap
#define MAX_SEQ_LENGTH 1024  // Maximum sequence length
#define TILE_SIZE 256       // Size of work unit per tile

// All data structures in L1
static int16_t dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t hPrime[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t gapArray[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t prefixOut[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t Lij[MAX_SEQ_LENGTH] __attribute__((section(".l1")));

// Sequences and their lengths
static const char* seq1;
static const char* seq2;
static int seq1_len;
static int seq2_len;

// Performance tracking
static uint32_t compute_start_times[256] __attribute__((section(".l1"))) = {0};
static uint32_t compute_end_times[256] __attribute__((section(".l1"))) = {0};
static int32_t global_compute_time __attribute__((section(".l1"))) = 0;
static int32_t global_sync_time __attribute__((section(".l1"))) = 0;

// Utility functions
static inline int16_t max(int16_t a, int16_t b) {
    return (a > b) ? a : b;
}

// Compute score based on match/mismatch
static inline int16_t compute_score(char a, char b) {
    return (a == b) ? MATCH_SCORE : MISMATCH_SCORE;
}

// Compute H intermediate: max(rowIPrev[j] + GAP_SCORE, rowIPrev[j-1] + match/mismatch, 0)
static inline int16_t HIntermediate(int16_t *rowIPrev, int j, int i) {
    int16_t matchMismatch = compute_score(seq1[j - 1], seq2[i - 1]);
    int16_t score1 = rowIPrev[j] + GAP_SCORE;
    int16_t score2 = rowIPrev[j - 1] + matchMismatch;
    int16_t score3 = 0;
    return max(max(score1, score2), score3);
}

// Compute final H value: max(finalInput, j * GAP_SCORE)
static inline int16_t HFinal(int16_t finalInput, int j) {
    return max(finalInput, j * GAP_SCORE);
}

// Compute prefix max of inputArray
static void prefixMax(int16_t *inputArray, int16_t *outputArray, int length) {
    outputArray[0] = inputArray[0];
    for (int i = 1; i < length; i++) {
        outputArray[i] = max(outputArray[i - 1], inputArray[i]);
    }
}

// Compute row segment for the Smith-Waterman algorithm
static void compute_row_segment(int i, int start_col, int end_col) {
    // PART 1: h'ij computation
    for (int j = start_col; j < end_col; j++) {
        hPrime[j - 1] = HIntermediate(dp[i - 1], j, i);
    }

    // PART 2: lij computation
    // Prepare input for prefix max
    for (int j = start_col - 1; j < end_col - 1; j++) {
        hPrime[j] += ((seq1_len - j - 2) * GAP_SCORE);
        gapArray[j] = -((seq1_len - j - 2) * GAP_SCORE);
    }

    // Compute prefix maximums
    prefixMax(hPrime + (start_col - 1), prefixOut + (start_col - 1), end_col - start_col);

    // Compute Lij values
    for (int j = start_col - 1; j < end_col - 1; j++) {
        Lij[j] = prefixOut[j] + gapArray[j];
    }

    // PART 3: Final H values computation
    for (int j = start_col - 1; j < end_col - 1; j++) {
        dp[i][j + 1] = HFinal(Lij[j], j);
    }
}

// Main parallel Smith-Waterman implementation
void smith_waterman(uint32_t core_id) {
    uint32_t num_cores = mempool_get_core_count();
    uint32_t active_cores = (seq1_len < num_cores) ? seq1_len : num_cores;
    
    // Stall unused cores
    if (core_id >= active_cores) {
        return;
    }
    
    // Initialize first row and column
    if (core_id == 0) {
        for (int j = 0; j <= seq1_len; j++) {
            dp[0][j] = 0;
        }
        for (int i = 0; i <= seq2_len; i++) {
            dp[i][0] = 0;
        }
    }

    mempool_barrier(active_cores);

    // Calculate work distribution
    int elements_per_core = (seq1_len + active_cores - 1) / active_cores;
    int start_col = core_id * elements_per_core + 1;
    int end_col = (core_id == active_cores - 1) ? seq1_len + 1 : (core_id + 1) * elements_per_core + 1;

    // Process rows
    for (int i = 1; i <= seq2_len; i++) {
        compute_start_times[core_id] = mempool_get_timer();
        
        compute_row_segment(i, start_col, end_col);
        
        compute_end_times[core_id] = mempool_get_timer();
        mempool_barrier(active_cores);

        // Performance tracking by core 0
        if (core_id == 0) {
            uint32_t earliest_start = UINT32_MAX;
            uint32_t latest_end = 0;

            for (int j = 0; j < active_cores; j++) {
                earliest_start = (compute_start_times[j] < earliest_start) ? 
                    compute_start_times[j] : earliest_start;
                latest_end = (compute_end_times[j] > latest_end) ? 
                    compute_end_times[j] : latest_end;
            }

            uint32_t row_compute_time = latest_end - earliest_start;
            uint32_t max_individual_compute = 0;

            for (int j = 0; j < active_cores; j++) {
                uint32_t core_compute_time = compute_end_times[j] - compute_start_times[j];
                max_individual_compute = (core_compute_time > max_individual_compute) ? 
                    core_compute_time : max_individual_compute;
            }

            uint32_t row_sync_time = row_compute_time - max_individual_compute;
            global_compute_time += max_individual_compute;
            global_sync_time += row_sync_time;
        }
    }
}

// Initialize the algorithm
static void init_smith_waterman(const char* sequence1, const char* sequence2) {
    seq1 = sequence1;
    seq2 = sequence2;
    seq1_len = strlen(seq1);
    seq2_len = strlen(seq2);
}

// Backtracking to find the alignment
static void backtrack(char* aligned1, char* aligned2) {
    int i = seq2_len;
    int j = seq1_len;
    int align_pos = 0;
    
    while (i > 0 && j > 0) {
        int score = dp[i][j];
        int diag_score = dp[i-1][j-1] + compute_score(seq1[j-1], seq2[i-1]);
        int up_score = dp[i-1][j] + GAP_SCORE;
        int left_score = dp[i][j-1] + GAP_SCORE;

        if (score == diag_score) {
            aligned1[align_pos] = seq1[j-1];
            aligned2[align_pos] = seq2[i-1];
            i--; j--;
        } else if (score == up_score) {
            aligned1[align_pos] = '-';
            aligned2[align_pos] = seq2[i-1];
            i--;
        } else {
            aligned1[align_pos] = seq1[j-1];
            aligned2[align_pos] = '-';
            j--;
        }
        align_pos++;
    }

    // Reverse the strings
    for (int k = 0; k < align_pos/2; k++) {
        char temp = aligned1[k];
        aligned1[k] = aligned1[align_pos-1-k];
        aligned1[align_pos-1-k] = temp;

        temp = aligned2[k];
        aligned2[k] = aligned2[align_pos-1-k];
        aligned2[align_pos-1-k] = temp;
    }
    aligned1[align_pos] = '\0';
    aligned2[align_pos] = '\0';
}

int main() {
    uint32_t core_id = mempool_get_core_id();
    
    // Initialize synchronization
    mempool_barrier_init(core_id);

    // Example sequences (only core 0 initializes)
    if (core_id == 0) {
        const char* sequence1 = "CTTGTCGCTCGTGCACGCTTTCAATTTCAGC";
        const char* sequence2 = "ACAAAAAGGCAGTGGGATTTTAGTGAGT";
        init_smith_waterman(sequence1, sequence2);
    }

    mempool_barrier(mempool_get_core_count());

    // Run Smith-Waterman algorithm
    smith_waterman(core_id);

    // Only core 0 handles results and backtracking
    if (core_id == 0) {
        char aligned1[MAX_SEQ_LENGTH * 2];
        char aligned2[MAX_SEQ_LENGTH * 2];
        backtrack(aligned1, aligned2);
        
        printf("Aligned sequences:\n%s\n%s\n", aligned1, aligned2);
        printf("Performance metrics:\nCompute time: %d\nSync time: %d\n", 
               global_compute_time, global_sync_time);
    }

    return 0;
}
