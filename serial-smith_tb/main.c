#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

// Define the maximum possible lengths for the sequences
//#define MAX_SEQ1_LEN 40
//#define MAX_SEQ2_LEN 20
#define cc 1

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

// Global DP matrix
int16_t dp_matrix[20 + 1][40 + 1] = {0};

// Function to calculate the Smith-Waterman DP matrix
void smith_waterman(const char* seq1, const char* seq2, int* max_i, int* max_j) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Initialize DP matrix
    for (int i = 0; i <= len1; i++) {
        dp_matrix[i][0] = 0; // Initialize leftmost column to 0
    }
    for (int j = 0; j <= len2; j++) {
        dp_matrix[0][j] = 0; // Initialize topmost row to 0
    }

    int16_t max_score = 0; // Reset max_score
    *max_i = 0;            // Reset position of max score
    *max_j = 0;

    // Fill DP matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int16_t match = dp_matrix[i - 1][j - 1] +
                            (seq1[i - 1] == seq2[j - 1] ? MATCH_SCORE : MISMATCH_SCORE);
            int16_t delete = dp_matrix[i - 1][j] + GAP_PENALTY;
            int16_t insert = dp_matrix[i][j - 1] + GAP_PENALTY;

            dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));

            // Track the maximum score and its position
            if (dp_matrix[i][j] > max_score) {
                max_score = dp_matrix[i][j];
                *max_i = i;
                *max_j = j;
            }
        }
    }
}

// Function to perform traceback and construct the aligned sequences
void traceback(const char* seq1, const char* seq2, int max_i, int max_j) {
    int i = max_i;
    int j = max_j;

    char aligned_seq1[61] = {0};
    char aligned_seq2[61] = {0};
    int index = 60;

    aligned_seq1[index] = '\0';
    aligned_seq2[index] = '\0';

    while (i > 0 && j > 0 && dp_matrix[i][j] > 0) {
        if (dp_matrix[i][j] == dp_matrix[i - 1][j - 1] +
                                (seq1[i - 1] == seq2[j - 1] ? MATCH_SCORE : MISMATCH_SCORE)) {
            aligned_seq1[--index] = seq1[--i];
            aligned_seq2[index] = seq2[--j];
        } else if (dp_matrix[i][j] == dp_matrix[i - 1][j] + GAP_PENALTY) {
            aligned_seq1[--index] = seq1[--i];
            aligned_seq2[index] = '-';
        } else {
            aligned_seq1[--index] = '-';
            aligned_seq2[index] = seq2[--j];
        }
    }

    printf("Aligned Sequences:\n");
    printf("Seq1: %s\n", &aligned_seq1[index]);
    printf("Seq2: %s\n", &aligned_seq2[index]);
}

int main() {
    uint32_t core_id = mempool_get_core_id();
    mempool_barrier_init(core_id);

    if (core_id >= cc) {
        while (1);
    }

    if (core_id == 0) {
        const char* seq1 = "GATGGGCGTGCCAAAGGAGATCGGACGAAATGGATTTCGA";
        const char* seq2 = "TGCGGGCCAAGATAGGCGGT";
        int max_i, max_j;

        uint32_t start_cycles = mempool_get_timer();
        smith_waterman(seq1, seq2, &max_i, &max_j);
        uint32_t end_cycles = mempool_get_timer();

        uint32_t start_traceback = mempool_get_timer();
        traceback(seq1, seq2, max_i, max_j);
        uint32_t end_traceback = mempool_get_timer();

        printf("Start cycles: %d\n", start_cycles);
        printf("End cycles: %d\n", end_cycles);
        printf("Compute time: %d\n", end_cycles - start_cycles);
        printf("Traceback time: %d\n", end_traceback - start_traceback);
    }

    return 0;
}

