#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

// Configuration parameters
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1
#define MAX_SEQ_LENGTH 30000  // Increased from 1000 to 30000

// Directions for traceback
#define TRACE_NONE 0
#define TRACE_DIAG 1
#define TRACE_UP 2
#define TRACE_LEFT 3

// Global DP and Traceback matrices
__attribute__((aligned(64))) int16_t dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH] = {0};
__attribute__((aligned(64))) uint8_t traceback[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH] = {0};

// Inline utility functions
static inline int16_t score(char a, char b) {
    return (a == b) ? MATCH_SCORE : MISMATCH_SCORE;
}
void strrev(char* str) {
    if (!str || !*str) return; // Handle null or empty strings

    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

static inline int16_t minimum(int16_t x, int16_t y) {
    return (x < y) ? x : y;
}

static inline int16_t maximum(int16_t x, int16_t y) {
    return (x > y) ? x : y;
}

static inline int16_t max3_with_trace(int16_t a, int16_t b, int16_t c, uint8_t *trace) {
    if (a >= b && a >= c) {
        *trace = TRACE_DIAG;
        return a;
    } else if (b >= c) {
        *trace = TRACE_UP;
        return b;
    } else {
        *trace = TRACE_LEFT;
        return c;
    }
}

// Calculate diagonal length
static inline int diagonal_length(int diagonal_index, int seq1_len, int seq2_len) {
    int min_dim = minimum(seq1_len, seq2_len);
    int max_dim = maximum(seq1_len, seq2_len);

    if (diagonal_index < min_dim) {
        return diagonal_index + 1;
    } else if (diagonal_index < max_dim) {
        return min_dim;
    } else {
        return seq1_len + seq2_len - 1 - diagonal_index;
    }
}

// Compute a segment of cells in a diagonal
static void compute_diagonal_segment(int diagonal_index, int start, int end, 
                                   const char* seq1, const char* seq2, 
                                   int seq1_len, int seq2_len) {
    // Calculate initial coordinates
    int initial_i, initial_j;
    if (diagonal_index < seq2_len) {
        initial_i = diagonal_index;
        initial_j = 0;
    } else {
        initial_i = seq2_len - 1;
        initial_j = diagonal_index - seq2_len + 1;
    }

    // Move to segment start
    initial_i -= start;
    initial_j += start;

    // Process segment
    for (int count = start; count < end; count++) {
        if (initial_i >= 0 && initial_j < seq1_len) {
            int16_t match, delete, insert;
            uint8_t trace;

            // Handle boundary conditions and compute scores
            if (initial_i == 0 && initial_j == 0) {
                match = score(seq1[initial_j], seq2[initial_i]);
                delete = GAP_PENALTY;
                insert = GAP_PENALTY;
                trace = TRACE_NONE;
            } else if (initial_i == 0) {
                match = score(seq1[initial_j], seq2[initial_i]);
                delete = GAP_PENALTY;
                insert = dp[initial_i][initial_j - 1] + GAP_PENALTY;
                trace = TRACE_LEFT;
            } else if (initial_j == 0) {
                match = score(seq1[initial_j], seq2[initial_i]);
                delete = dp[initial_i - 1][initial_j] + GAP_PENALTY;
                insert = GAP_PENALTY;
                trace = TRACE_UP;
            } else {
                match = dp[initial_i - 1][initial_j - 1] + score(seq1[initial_j], seq2[initial_i]);
                delete = dp[initial_i - 1][initial_j] + GAP_PENALTY;
                insert = dp[initial_i][initial_j - 1] + GAP_PENALTY;
                dp[initial_i][initial_j] = maximum(0, max3_with_trace(match, delete, insert, &trace));
            }

            dp[initial_i][initial_j] = maximum(0, max3_with_trace(match, delete, insert, &trace));
            traceback[initial_i][initial_j] = trace;
        }
        initial_i--;
        initial_j++;
    }
}

// Perform traceback to reconstruct the alignment
void traceback_alignment(const char* seq1, const char* seq2, int max_i, int max_j) {
    char alignment1[MAX_SEQ_LENGTH] = {0};
    char alignment2[MAX_SEQ_LENGTH] = {0};
    int index1 = 0, index2 = 0;

    int i = max_i, j = max_j;
    while (i >= 0 && j >= 0 && dp[i][j] > 0) {
        if (traceback[i][j] == TRACE_DIAG) {
            alignment1[index1++] = seq1[j];
            alignment2[index2++] = seq2[i];
            i--;
            j--;
        } else if (traceback[i][j] == TRACE_UP) {
            alignment1[index1++] = '-';
            alignment2[index2++] = seq2[i];
            i--;
        } else if (traceback[i][j] == TRACE_LEFT) {
            alignment1[index1++] = seq1[j];
            alignment2[index2++] = '-';
            j--;
        }
    }

    // Reverse alignments for final output
    alignment1[index1] = '\0';
    alignment2[index2] = '\0';
    strrev(alignment1);
    strrev(alignment2);

    printf("\nOptimal Alignment:\n");
    printf("Seq1: %s\n", alignment1);
    printf("Seq2: %s\n", alignment2);
}

// Main Smith-Waterman algorithm
void smith_waterman(const char* seq1, const char* seq2, int num_threads) {
    int seq1_len = strlen(seq1);
    int seq2_len = strlen(seq2);

    if (seq1_len > MAX_SEQ_LENGTH || seq2_len > MAX_SEQ_LENGTH) {
        fprintf(stderr, "Error: Sequence length exceeds maximum allowed length\n");
        exit(1);
    }

    int total_diagonals = seq1_len + seq2_len - 1;

    // Set number of threads
    omp_set_num_threads(num_threads);

    double start_time = omp_get_wtime();

    // Process each diagonal
    for (int diagonal = 0; diagonal < total_diagonals; diagonal++) {
        int diag_len = diagonal_length(diagonal, seq1_len, seq2_len);
        int active_threads = minimum(diag_len, num_threads);

        // Parallelize the diagonal computation
        #pragma omp parallel num_threads(active_threads)
        {
            int thread_id = omp_get_thread_num();
            int elements_per_thread = (diag_len + active_threads - 1) / active_threads;
            int start = thread_id * elements_per_thread;
            int end = minimum(start + elements_per_thread, diag_len);

            if (start < diag_len) {
                compute_diagonal_segment(diagonal, start, end, seq1, seq2, seq1_len, seq2_len);
            }
        }
        // Implicit barrier at end of parallel region
    }

    // Find maximum score and position
    int16_t max_score = 0;
    int max_i = 0, max_j = 0;

    #pragma omp parallel for reduction(max:max_score) collapse(2)
    for (int i = 0; i < seq2_len; i++) {
        for (int j = 0; j < seq1_len; j++) {
            if (dp[i][j] > max_score) {
                #pragma omp critical
                {
                    if (dp[i][j] > max_score) {
                        max_score = dp[i][j];
                        max_i = i;
                        max_j = j;
                    }
                }
            }
        }
    }

    // Perform traceback
    traceback_alignment(seq1, seq2, max_i, max_j);
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    printf("Max alignment score: %d at position (%d, %d)\n", max_score, max_i, max_j);
    printf("Execution time: %.6f seconds\n", time_taken);

}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <sequence1> <sequence2> <num_threads>\n", argv[0]);
        return 1;
    }

    const char* seq1 = argv[1];
    const char* seq2 = argv[2];
    int num_threads = atoi(argv[3]);

    if (num_threads < 1) {
        fprintf(stderr, "Error: Number of threads must be positive\n");
        return 1;
    }

    printf("Running Anti-Diagonal Smith-Waterman with Traceback\n");
    printf("Sequence 1: %s (Length: %zu)\n", seq1, strlen(seq1));
    printf("Sequence 2: %s (Length: %zu)\n", seq2, strlen(seq2));
    printf("Number of threads: %d\n", num_threads);

    smith_waterman(seq1, seq2, num_threads);

    return 0;
}

