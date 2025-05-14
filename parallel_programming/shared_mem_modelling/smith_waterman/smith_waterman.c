#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>  // OpenMP header for timing

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

void print_dp_matrix(int** dp_matrix, const char* seq1, const char* seq2, int len1, int len2) {
    printf("\n--- Dynamic Programming Matrix ---\n");
    printf("     ");
    for (int j = 0; j < len2; j++) {
        printf("  %3c ", seq2[j]);
    }
    printf("\n");

    for (int i = 0; i <= len1; i++) {
        printf("%3c  ", (i == 0 ? ' ' : seq1[i - 1]));
        for (int j = 0; j <= len2; j++) {
            printf("%3d ", dp_matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void smith_waterman(const char* seq1, const char* seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Allocate DP matrix
    int** dp_matrix = (int**)malloc((len1 + 1) * sizeof(int*));
    for (int i = 0; i <= len1; i++) {
        dp_matrix[i] = (int*)malloc((len2 + 1) * sizeof(int));
    }

    // Initialize DP matrix
    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            dp_matrix[i][j] = 0;
        }
    }

    // Variables to track maximum score and position
    int max_score = 0;
    int max_i = 0;
    int max_j = 0;

    // Time the matrix filling step
    double start_time = omp_get_wtime();

    // Fill DP matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int match = dp_matrix[i - 1][j - 1] + (seq1[i - 1] == seq2[j - 1] ? MATCH_SCORE : MISMATCH_SCORE);
            int delete = dp_matrix[i - 1][j] + GAP_PENALTY;
            int insert = dp_matrix[i][j - 1] + GAP_PENALTY;
            dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
            
            // Track the maximum score and its position
            if (dp_matrix[i][j] > max_score) {
                max_score = dp_matrix[i][j];
                max_i = i;
                max_j = j;
            }
        }
    }

    double end_time = omp_get_wtime();
    printf("Time taken to fill the DP matrix: %f seconds\n", end_time - start_time);

    // Print the DP matrix
    print_dp_matrix(dp_matrix, seq1, seq2, len1, len2);

    // Remaining code for backtracking, alignment, and cleanup remains the same...

    // Free allocated memory
    for (int i = 0; i <= len1; i++) {
        free(dp_matrix[i]);
    }
    free(dp_matrix);
}

int main() {
    const char* seq1 = "ACGTT";

    const char* seq2 = "ACG";
    smith_waterman(seq1, seq2);
    return 0;
}

