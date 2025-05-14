#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -2

const char *seq1 = "ACGTT";
const char *seq2 = "ACG";

void prefixMax(int *inputArray, int *outputArray, int length) {
    outputArray[0] = inputArray[0];
    for (int i = 1; i < length; i++) {
        outputArray[i] = (outputArray[i - 1] > inputArray[i]) ? outputArray[i - 1] : inputArray[i];
    }
}

int HIntermediate(int *rowIPrev, int jPos, int iPos) {
    int matchMismatch = (seq1[jPos - 1] == seq2[iPos - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
    int score1 = rowIPrev[jPos] + GAP_SCORE;
    int score2 = rowIPrev[jPos - 1] + matchMismatch;
    return (score1 > score2 ? score1 : score2) > 0 ? (score1 > score2 ? score1 : score2) : 0;
}

int HFinal(int finalInput, int jPos) {
    int gapScoreCalc = jPos * GAP_SCORE;
    return (finalInput > gapScoreCalc) ? finalInput : gapScoreCalc;
}

int main() {
    int rows = strlen(seq2) + 1;
    int cols = strlen(seq1) + 1;

    // Initialize matrix
    double serial_start = omp_get_wtime();
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)calloc(cols, sizeof(int));
    }
    double serial_end = omp_get_wtime();

    // Temporary arrays
    int *hPrime = (int *)calloc(cols - 1, sizeof(int));
    int *gapArray = (int *)calloc(cols - 1, sizeof(int));
    int *prefixOut = (int *)calloc(cols - 1, sizeof(int));
    int *Lij = (int *)calloc(cols - 1, sizeof(int));

    double parallel_start = omp_get_wtime();
    for (int i = 1; i < rows; i++) {
        // Parallelizable: hPrime computation
        #pragma omp parallel for
        for (int j = 1; j < cols; j++) {
            hPrime[j - 1] = HIntermediate(matrix[i - 1], j, i);
        }

        // Parallelizable: Adjust hPrime and compute gapArray
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            hPrime[j] += ((cols - j - 2) * GAP_SCORE);
            gapArray[j] = -((cols - j - 2) * GAP_SCORE);
        }

        // Serial: Prefix max computation
        prefixMax(hPrime, prefixOut, cols - 1);

        // Parallelizable: Compute Lij
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            Lij[j] = prefixOut[j] + gapArray[j];
        }

        // Parallelizable: Compute final matrix values
        #pragma omp parallel for
        for (int j = 0; j < cols - 1; j++) {
            matrix[i][j + 1] = HFinal(Lij[j], j);
        }
    }
    double parallel_end = omp_get_wtime();

    double print_start = omp_get_wtime();
    // Print final matrix (Serial)
    printf("Final Matrix:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    double print_end = omp_get_wtime();

    // Cleanup (Serial)
    double cleanup_start = omp_get_wtime();
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(hPrime);
    free(gapArray);
    free(prefixOut);
    free(Lij);
    double cleanup_end = omp_get_wtime();

    // Report times
    printf("Setup time: %f seconds\n", serial_end - serial_start);
    printf("Parallel computation time: %f seconds\n", parallel_end - parallel_start);
    printf("Printing time: %f seconds\n", print_end - print_start);
    printf("Cleanup time: %f seconds\n", cleanup_end - cleanup_start);

    return 0;
}

