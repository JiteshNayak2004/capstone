#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "runtime.h"
#include "synchronization.h"

#define MAX_SEQ_LENGTH 100
#define NUM_CORES 2
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -1

//dp matrix
uint16_t matrix[100][100] __attribute__((section(".l1"))); 

// Function to compute Smith-Waterman score
int32_t compute_score(char a, char b) {
    if (a == b) return MATCH_SCORE;
    return MISMATCH_SCORE;
}

// Function to compute a portion of the diagonal
void compute_diagonal_portion(int diag, int start, int end, const char* seq1, const char* seq2) {
    for (int k = start; k < end && k <= diag; k++) {
        int i = k;
        int j = diag - k;

        if (i < MAX_SEQ_LENGTH && j < MAX_SEQ_LENGTH) {
            int32_t diagonal = get_spm_value(calculate_address(i-1, j-1)) + compute_score(seq1[i-1], seq2[j-1]);
            int32_t left = get_spm_value(calculate_address(i, j-1)) + GAP_SCORE;
            int32_t up = get_spm_value(calculate_address(i-1, j)) + GAP_SCORE;

            int32_t max_score = (diagonal > left) ? diagonal : left;
            max_score = (max_score > up) ? max_score : up;
            max_score = (max_score > 0) ? max_score : 0;

            set_spm_value(calculate_address(i, j), max_score);
        }
    }
}

int main() {
    uint32_t core_id = mempool_get_core_id();

    // Example sequences
    const char* seq1 = "ACGTACGT";
    const char* seq2 = "ACGTACGT";
    int seq1_len = 8;
    int seq2_len = 8;

    // Initialize synchronization
    mempool_barrier_init(core_id);

    // Main computation loop
    for (int diag = 0; diag < seq1_len + seq2_len - 1; diag++) {
        int diag_length = (diag < seq1_len) ? diag + 1 : seq1_len + seq2_len - diag - 1;
        int elements_per_core = (diag_length + NUM_CORES - 1) / NUM_CORES;
        int start = core_id * elements_per_core;
        int end = (core_id == NUM_CORES - 1) ? diag_length : (core_id + 1) * elements_per_core;

        compute_diagonal_portion(diag, start, end, seq1, seq2);

        // Synchronize after each diagonal computation
        mempool_barrier(NUM_CORES);
    }

    // Print the result (only core 0)
    if (core_id == 0) {
        int32_t max_score = 0;
        for (int i = 0; i < seq1_len; i++) {
            for (int j = 0; j < seq2_len; j++) {
                int32_t score = get_spm_value(calculate_address(i, j));
                if (score > max_score) {
                    max_score = score;
                }
            }
        }
        printf("Maximum alignment score: %d\n", max_score);
    }

    return 0;
}
