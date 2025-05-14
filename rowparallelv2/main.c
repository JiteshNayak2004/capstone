#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "synchronization.h"

// Disable vectorization for the entire file
#pragma GCC optimize("no-tree-vectorize")
// Enable aggressive optimizations
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

// Configuration parameters
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -2
#define MAX_SEQ_LENGTH 100
#define BLOCK_SIZE 16  // Optimized for L1 SPM banks
#define NUM_BANKS 16   // Number of L1 memory banks

// Memory-aligned data structures in L1 SPM
// Align to 4 bytes (32-bit) for optimal memory access
__attribute__((aligned(4), section(".l1"))) static int16_t dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH];
__attribute__((aligned(4), section(".l1"))) static int16_t hPrime[MAX_SEQ_LENGTH];
__attribute__((aligned(4), section(".l1"))) static int16_t gapArray[MAX_SEQ_LENGTH];
__attribute__((aligned(4), section(".l1"))) static int16_t prefixOut[MAX_SEQ_LENGTH];
__attribute__((aligned(4), section(".l1"))) static int16_t Lij[MAX_SEQ_LENGTH];

// Double buffering arrays for overlapping computation
__attribute__((aligned(4), section(".l1"))) static int16_t buffer1[BLOCK_SIZE][MAX_SEQ_LENGTH];
__attribute__((aligned(4), section(".l1"))) static int16_t buffer2[BLOCK_SIZE][MAX_SEQ_LENGTH];

// Lookup table for gap scores
__attribute__((aligned(4), section(".l1"))) static int16_t gap_score_table[MAX_SEQ_LENGTH];

// Sequences and lengths
static const char* seq1;
static const char* seq2;
static int seq1_len;
static int seq2_len;

// Performance tracking aligned to avoid bank conflicts
__attribute__((aligned(4), section(".l1"))) static uint32_t compute_times[256] = {0};
__attribute__((aligned(4), section(".l1"))) static int32_t global_compute_time = 0;

// Optimized max function
static inline int16_t __attribute__((always_inline)) fast_max(int16_t a, int16_t b) {
    return a > b ? a : b;
}

// Optimized score computation
static inline int16_t __attribute__((always_inline)) compute_score(char a, char b) {
    return (a == b) ? MATCH_SCORE : MISMATCH_SCORE;
}

// Initialize gap score lookup table
static void init_gap_scores() {
    for (int i = 0; i < MAX_SEQ_LENGTH; i++) {
        gap_score_table[i] = (int16_t)(i * GAP_SCORE);
    }
}

// Optimized H intermediate computation
static inline int16_t __attribute__((always_inline)) compute_h_intermediate(const int16_t* row_prev, int j, char a, char b) {
    int16_t match_score = compute_score(a, b);
    int16_t score1 = row_prev[j] + GAP_SCORE;
    int16_t score2 = row_prev[j-1] + match_score;
    return fast_max(fast_max(score1, score2), 0);
}

// Optimized prefix max computation with manual unrolling
static void compute_prefix_max(const int16_t *input, int16_t *output, int length) {
    output[0] = input[0];

    // Manual unroll by 4 to maximize instruction pipeline usage
    int i;
    for (i = 1; i < length - 3; i += 4) {
        output[i] = fast_max(output[i-1], input[i]);
        output[i+1] = fast_max(output[i], input[i+1]);
        output[i+2] = fast_max(output[i+1], input[i+2]);
        output[i+3] = fast_max(output[i+2], input[i+3]);
    }

    // Handle remaining elements
    for (; i < length; i++) {
        output[i] = fast_max(output[i-1], input[i]);
    }
}

// Process a block of rows using double buffering
static void process_block(int start_row, int block_size, int core_id, int num_cores) {
    int16_t (*curr_buffer)[MAX_SEQ_LENGTH] = buffer1;
    int16_t (*next_buffer)[MAX_SEQ_LENGTH] = buffer2;

    for (int block_row = 0; block_row < block_size && (start_row + block_row) <= seq2_len; block_row++) {
        int i = start_row + block_row;

        // Compute h'ij values with manual unrolling for instruction pipeline
        int j;
        for (j = 1; j <= seq1_len - 3; j += 4) {
            hPrime[j-1] = compute_h_intermediate(dp[i-1], j, seq1[j-1], seq2[i-1]);
            hPrime[j] = compute_h_intermediate(dp[i-1], j+1, seq1[j], seq2[i-1]);
            hPrime[j+1] = compute_h_intermediate(dp[i-1], j+2, seq1[j+1], seq2[i-1]);
            hPrime[j+2] = compute_h_intermediate(dp[i-1], j+3, seq1[j+2], seq2[i-1]);
        }
        for (; j <= seq1_len; j++) {
            hPrime[j-1] = compute_h_intermediate(dp[i-1], j, seq1[j-1], seq2[i-1]);
        }

        // Optimize gap calculations using lookup table
        // Unroll by 4 to maximize instruction pipeline
        for (j = 0; j < seq1_len - 3; j += 4) {
            int16_t idx1 = (int16_t)(seq1_len - j - 2);
            int16_t idx2 = (int16_t)(seq1_len - (j+1) - 2);
            int16_t idx3 = (int16_t)(seq1_len - (j+2) - 2);
            int16_t idx4 = (int16_t)(seq1_len - (j+3) - 2);

            hPrime[j] += gap_score_table[idx1];
            hPrime[j+1] += gap_score_table[idx2];
            hPrime[j+2] += gap_score_table[idx3];
            hPrime[j+3] += gap_score_table[idx4];

            gapArray[j] = -gap_score_table[idx1];
            gapArray[j+1] = -gap_score_table[idx2];
            gapArray[j+2] = -gap_score_table[idx3];
            gapArray[j+3] = -gap_score_table[idx4];
        }
        for (; j < seq1_len; j++) {
            int16_t idx = (int16_t)(seq1_len - j - 2);
            hPrime[j] += gap_score_table[idx];
            gapArray[j] = -gap_score_table[idx];
        }

        compute_prefix_max(hPrime, prefixOut, seq1_len);

        // Compute Lij and final H values together to reduce memory operations
        // Unroll by 4 for better instruction pipeline utilization
        for (j = 0; j < seq1_len - 3; j += 4) {
            // Load values into registers to minimize memory access
            int16_t prefix1 = prefixOut[j];
            int16_t prefix2 = prefixOut[j+1];
            int16_t prefix3 = prefixOut[j+2];
            int16_t prefix4 = prefixOut[j+3];

            int16_t gap1 = gapArray[j];
            int16_t gap2 = gapArray[j+1];
            int16_t gap3 = gapArray[j+2];
            int16_t gap4 = gapArray[j+3];

            // Compute Lij values
            Lij[j] = prefix1 + gap1;
            Lij[j+1] = prefix2 + gap2;
            Lij[j+2] = prefix3 + gap3;
            Lij[j+3] = prefix4 + gap4;

            // Store final H values
            curr_buffer[block_row][j+1] = fast_max(Lij[j], gap_score_table[j]);
            curr_buffer[block_row][j+2] = fast_max(Lij[j+1], gap_score_table[j+1]);
            curr_buffer[block_row][j+3] = fast_max(Lij[j+2], gap_score_table[j+2]);
            curr_buffer[block_row][j+4] = fast_max(Lij[j+3], gap_score_table[j+3]);
        }
        for (; j < seq1_len; j++) {
            Lij[j] = prefixOut[j] + gapArray[j];
            curr_buffer[block_row][j+1] = fast_max(Lij[j], gap_score_table[j]);
        }
    }

    // Copy block results to main dp matrix
    for (int block_row = 0; block_row < block_size && (start_row + block_row) <= seq2_len; block_row++) {
        memcpy(dp[start_row + block_row], curr_buffer[block_row], (seq1_len + 1) * sizeof(int16_t));
    }
}

// Main Smith-Waterman implementation
void smith_waterman(uint32_t core_id) {
    uint32_t num_cores = mempool_get_core_count();
    uint32_t active_cores = (seq1_len < num_cores) ? seq1_len : num_cores;

    // Stall unused cores
    if (core_id >= active_cores) {
        return;
    }

    // Initialize first row and column (only core 0)
    if (core_id == 0) {
        memset(dp[0], 0, (seq1_len + 1) * sizeof(int16_t));
        for (int i = 0; i <= seq2_len; i++) {
            dp[i][0] = 0;
        }
        init_gap_scores();
    }

    mempool_barrier(active_cores);

    // Calculate work distribution with improved load balancing
    int elements_per_core = (seq1_len + active_cores - 1) / active_cores;
    int start_col = core_id * elements_per_core + 1;
    int end_col = (core_id == active_cores - 1) ? seq1_len + 1 : (core_id + 1) * elements_per_core + 1;

    // Process rows in blocks with timing
    uint32_t start_time = mempool_get_timer();

    for (int i = 1; i <= seq2_len; i++) {
        process_block(i, 1, core_id, active_cores);
        mempool_barrier(active_cores);  // Synchronize after each block
    }

    uint32_t end_time = mempool_get_timer();
    compute_times[core_id] = end_time - start_time;

    // Accumulate compute times
    if (core_id == 0) {
        for (int i = 0; i < active_cores; i++) {
            global_compute_time += compute_times[i];
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
        printf("Compute time: %d\n", global_compute_time);
    }

    return 0;
}

