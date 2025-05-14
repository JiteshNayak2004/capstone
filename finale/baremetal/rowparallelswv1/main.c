#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "synchronization.h"

// Configuration parameters
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -2
#define MAX_SEQ_LENGTH 1024
#define VECTOR_SIZE 4  // Process 4 elements at once using SIMD
#define UNROLL_FACTOR 4  // Manual loop unrolling factor

// All data structures in L1
static int16_t __attribute__((aligned(8))) dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t __attribute__((aligned(8))) hPrime[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t __attribute__((aligned(8))) gapArray[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t __attribute__((aligned(8))) prefixOut[MAX_SEQ_LENGTH] __attribute__((section(".l1")));
static int16_t __attribute__((aligned(8))) Lij[MAX_SEQ_LENGTH] __attribute__((section(".l1")));

// Sequences and lengths
static const char* seq1;
static const char* seq2;
static int seq1_len;
static int seq2_len;

// Performance tracking
static uint32_t compute_start_times[256] __attribute__((section(".l1"))) = {0};
static uint32_t compute_end_times[256] __attribute__((section(".l1"))) = {0};
static int32_t global_compute_time __attribute__((section(".l1"))) = 0;
static int32_t global_sync_time __attribute__((section(".l1"))) = 0;

// SIMD optimized max function using XpulpV2 instructions
static inline int16_t v2s_max(int16_t a, int16_t b) {
    int16_t result;
    asm volatile ("pmax %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
    return result;
}

// Vectorized score computation for 4 elements
static inline void compute_score_vector(const char* a, const char* b, int16_t* scores) {
    for (int i = 0; i < VECTOR_SIZE; i++) {
        scores[i] = (a[i] == b[i]) ? MATCH_SCORE : MISMATCH_SCORE;
    }
}

// Optimized H intermediate computation for 4 elements
static inline void HIntermediate_vector(int16_t* rowIPrev, int j, int i, int16_t* results) {
    // Load 4 elements at once using v2s load
    int16_t scores[VECTOR_SIZE];
    compute_score_vector(&seq1[j-1], &seq2[i-1], scores);

    // Process 4 elements in parallel
    #pragma GCC unroll 4
    for (int k = 0; k < VECTOR_SIZE; k++) {
        int16_t score1 = rowIPrev[j+k] + GAP_SCORE;
        int16_t score2 = rowIPrev[j+k-1] + scores[k];
        results[k] = v2s_max(v2s_max(score1, score2), 0);
    }
}

// Optimized prefix max computation
static void prefixMax_optimized(int16_t *inputArray, int16_t *outputArray, int length) {
    outputArray[0] = inputArray[0];

    // Unrolled loop for better instruction pipeline utilization
    for (int i = 1; i < length; i += UNROLL_FACTOR) {
        #pragma GCC unroll 4
        for (int j = 0; j < UNROLL_FACTOR && (i+j) < length; j++) {
            outputArray[i+j] = v2s_max(outputArray[i+j-1], inputArray[i+j]);
        }
    }
}

// Optimized row segment computation
static void compute_row_segment(int i, int start_col, int end_col) {
    // Vector processing of h'ij
    for (int j = start_col; j < end_col; j += VECTOR_SIZE) {
        if (j + VECTOR_SIZE <= end_col) {
            // Process 4 elements at once
            HIntermediate_vector(dp[i-1], j, i, &hPrime[j-1]);
        } else {
            // Handle remaining elements
            for (int k = j; k < end_col; k++) {
                int16_t matchMismatch = (seq1[k-1] == seq2[i-1]) ? MATCH_SCORE : MISMATCH_SCORE;
                int16_t score1 = dp[i-1][k] + GAP_SCORE;
                int16_t score2 = dp[i-1][k-1] + matchMismatch;
                hPrime[k-1] = v2s_max(v2s_max(score1, score2), 0);
            }
        }
    }

    // Optimize gap calculations using vector operations
    #pragma GCC unroll 4
    for (int j = start_col - 1; j < end_col - 1; j += UNROLL_FACTOR) {
        for (int k = 0; k < UNROLL_FACTOR && (j+k) < end_col-1; k++) {
            int gap_factor = (seq1_len - (j+k) - 2);
            hPrime[j+k] += gap_factor * GAP_SCORE;
            gapArray[j+k] = -(gap_factor * GAP_SCORE);
        }
    }

    // Optimized prefix max computation
    prefixMax_optimized(hPrime + (start_col - 1), prefixOut + (start_col - 1), end_col - start_col);

    // Vectorized Lij computation
    #pragma GCC unroll 4
    for (int j = start_col - 1; j < end_col - 1; j += UNROLL_FACTOR) {
        for (int k = 0; k < UNROLL_FACTOR && (j+k) < end_col-1; k++) {
            Lij[j+k] = prefixOut[j+k] + gapArray[j+k];
        }
    }

    // Final H values computation with vector operations
    #pragma GCC unroll 4
    for (int j = start_col - 1; j < end_col - 1; j += UNROLL_FACTOR) {
        for (int k = 0; k < UNROLL_FACTOR && (j+k) < end_col-1; k++) {
            dp[i][j+k+1] = v2s_max(Lij[j+k], (j+k) * GAP_SCORE);
        }
    }
}

// Optimized parallel Smith-Waterman implementation
void smith_waterman(uint32_t core_id) {
    uint32_t num_cores = mempool_get_core_count();

    // Initialize first row and column (only core 0)
    if (core_id == 0) {
        memset(dp[0], 0, (seq1_len + 1) * sizeof(int16_t));
        for (int i = 0; i <= seq2_len; i++) {
            dp[i][0] = 0;
        }
    }

    mempool_barrier(num_cores);

    // Optimized work distribution
    int rows_per_core = (seq2_len + num_cores - 1) / num_cores;
    int start_row = core_id * rows_per_core + 1;
    int end_row = (core_id == num_cores - 1) ? seq2_len + 1 : (core_id + 1) * rows_per_core + 1;

    // Process assigned rows
    for (int i = start_row; i < end_row; i++) {
        compute_start_times[core_id] = mempool_get_timer();

        compute_row_segment(i, 1, seq1_len + 1);

        compute_end_times[core_id] = mempool_get_timer();
        mempool_barrier(num_cores);

        // Performance tracking (core 0 only)
        if (core_id == 0) {
            uint32_t earliest_start = UINT32_MAX;
            uint32_t latest_end = 0;

            for (int j = 0; j < num_cores; j++) {
                earliest_start = (compute_start_times[j] < earliest_start) ?
                    compute_start_times[j] : earliest_start;
                latest_end = (compute_end_times[j] > latest_end) ?
                    compute_end_times[j] : latest_end;
            }

            global_compute_time += latest_end - earliest_start;
        }
    }
}

// Initialize sequences
static void init_smith_waterman(const char* sequence1, const char* sequence2) {
    seq1 = sequence1;
    seq2 = sequence2;
    seq1_len = strlen(seq1);
    seq2_len = strlen(seq2);
}

// Optimized backtracking
static void backtrack(char* aligned1, char* aligned2) {
    int i = seq2_len;
    int j = seq1_len;
    int align_pos = 0;

    while (i > 0 && j > 0) {
        int score = dp[i][j];
        int diag_score = dp[i-1][j-1] + ((seq1[j-1] == seq2[i-1]) ? MATCH_SCORE : MISMATCH_SCORE);
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

    mempool_barrier_init(core_id);

    if (core_id == 0) {
        const char* sequence1 = "CTTGTCGCTCGTGCACGCTTTCAATTTCAGC";
        const char* sequence2 = "ACAAAAAGGCAGTGGGATTTTAGTGAGT";
        init_smith_waterman(sequence1, sequence2);
    }

    mempool_barrier(mempool_get_core_count());
    smith_waterman(core_id);

    if (core_id == 0) {
        char aligned1[MAX_SEQ_LENGTH * 2];
        char aligned2[MAX_SEQ_LENGTH * 2];
        backtrack(aligned1, aligned2);

        printf("Aligned sequences:\n%s\n%s\n", aligned1, aligned2);
        printf("Performance metrics:\nCompute time: %d\n", global_compute_time);
    }

    return 0;
}
