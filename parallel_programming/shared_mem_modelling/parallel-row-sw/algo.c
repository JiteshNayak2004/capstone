#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#define MATCH_SCORE 2       // Score for a match
#define MISMATCH_SCORE -1   // Penalty for a mismatch
#define GAP_SCORE -2        // Penalty for a gap

// Define number of threads for parallelization
#define THREAD_COUNT 4

// Returns the maximum of two integers
int max(int a, int b) {
    return a > b ? a : b;
}

// Computes H intermediate: max(rowIPrev[j] + GAP_SCORE, rowIPrev[j-1] + match/mismatch, 0)
int HIntermediate(int *rowIPrev, int j, int i, const char *seq1, const char *seq2) {
    int matchMismatch = (seq1[j - 1] == seq2[i - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
    int score1 = rowIPrev[j] + GAP_SCORE;
    int score2 = rowIPrev[j - 1] + matchMismatch;
    return max(max(score1, score2), 0);
}

// Computes final H value: max(finalInput, j * GAP_SCORE)
int HFinal(int finalInput, int j) {
    return max(finalInput, j * GAP_SCORE);
}

// Computes prefix max of inputArray
void prefixMax(int *inputArray, int *outputArray, int length) {
    outputArray[0] = inputArray[0];
    for (int i = 1; i < length; i++) {
        outputArray[i] = max(outputArray[i - 1], inputArray[i]);
    }
}

int main() {
    const char *seq1 = "ACGTT";
    const char *seq2 = "ACG";

    int rows = strlen(seq2) + 1;
    int cols = strlen(seq1) + 1;

    // Allocate the scoring matrix
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)calloc(cols, sizeof(int));
    }

    // Temporary arrays for intermediate computations
    int *hPrime = (int *)calloc(cols - 1, sizeof(int));
    int *gapArray = (int *)calloc(cols - 1, sizeof(int));
    int *prefixOut = (int *)calloc(cols - 1, sizeof(int));
    int *Lij = (int *)calloc(cols - 1, sizeof(int));

    // Set number of threads for OpenMP
    omp_set_num_threads(THREAD_COUNT);

    double overall_start = omp_get_wtime();

    for (int i = 1; i < rows; i++) {

        // Compute H intermediate values
        #pragma omp parallel for
        for (int j = 1; j < cols; j++) {
            hPrime[j - 1] = HIntermediate(matrix[i - 1], j, i, seq1, seq2);
        }

        // Compute adjusted H intermediate values and gapArray
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            hPrime[j] += ((cols - j - 2) * GAP_SCORE);
            gapArray[j] = -((cols - j - 2) * GAP_SCORE);
        }

        // Compute prefix maximums
        prefixMax(hPrime, prefixOut, cols - 1);

        // Compute Lij values
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            Lij[j] = prefixOut[j] + gapArray[j];
        }

        // Compute final H values for row i
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            matrix[i][j + 1] = HFinal(Lij[j], j);
        }

    }

    double overall_end = omp_get_wtime();
    printf("Overall time taken: %f seconds\n", overall_end - overall_start);

    // Print the final matrix
    printf("Final Matrix:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    // Free allocated memory
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(hPrime);
    free(gapArray);
    free(prefixOut);
    free(Lij);

    return 0;
}

