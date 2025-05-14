#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

// Define the maximum possible lengths for the sequences
#define MAX_SEQ_LENGTH 3000

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1

// Inline max functions for better performance
static inline int16_t max2(int16_t a, int16_t b) {
    return (a > b) ? a : b;
}

static inline int16_t max3(int16_t a, int16_t b, int16_t c) {
    return max2(max2(a, b), c);
}

// Global DP matrix with cache line padding to prevent false sharing
__attribute__((aligned(64))) int16_t dp_matrix[MAX_SEQ_LENGTH + 1][MAX_SEQ_LENGTH + 1] = {0};

// Function to calculate the Smith-Waterman DP matrix
void smith_waterman(const char* seq1, const char* seq2, int num_threads) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);
    
    if (len1 > MAX_SEQ_LENGTH || len2 > MAX_SEQ_LENGTH) {
        fprintf(stderr, "Error: Sequence length exceeds maximum allowed length\n");
        exit(1);
    }
    
    // Initialize variables to track maximum score
    int16_t max_score = 0;
    int16_t max_i = 0;
    int16_t max_j = 0;
    
    // Set number of threads
    omp_set_num_threads(num_threads);
    
    double start_time = omp_get_wtime();
    
    // Initialize first row and column (already 0 from static initialization)
    
    // Fill DP matrix using OpenMP
    #pragma omp parallel shared(dp_matrix, seq1, seq2, len1, len2) 
    {
        int16_t local_max_score = 0;
        int16_t local_max_i = 0;
        int16_t local_max_j = 0;
        
        // Parallelize the outer loop for better load balancing
        #pragma omp for schedule(guided)
        for (int i = 1; i <= len2; i++) {
            for (int j = 1; j <= len1; j++) {
                // Calculate scores
                int16_t match = dp_matrix[i-1][j-1] + 
                    (seq2[i-1] == seq1[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
                int16_t delete = dp_matrix[i-1][j] + GAP_PENALTY;
                int16_t insert = dp_matrix[i][j-1] + GAP_PENALTY;
                
                // Update matrix
                dp_matrix[i][j] = max3(0, match, max2(delete, insert));
                
                // Update local maximum
                if (dp_matrix[i][j] > local_max_score) {
                    local_max_score = dp_matrix[i][j];
                    local_max_i = i;
                    local_max_j = j;
                }
            }
        }
        
        // Critical section to update global maximum
        #pragma omp critical
        {
            if (local_max_score > max_score) {
                max_score = local_max_score;
                max_i = local_max_i;
                max_j = local_max_j;
            }
        }
    }
    
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    uint64_t cells = (uint64_t)len1 * (uint64_t)len2;
    
    // Print results
    printf("Max score: %d at position (%d, %d)\n", max_score, max_i, max_j);
    printf("Execution time: %.6f seconds\n", time_taken);
}

#ifdef DEBUG
void print_dp_matrix(const char* seq1, const char* seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);
    
    printf("    ");
    for (int j = 0; j < len1; j++) {
        printf("  %c", seq1[j]);
    }
    printf("\n");
    
    for (int i = 0; i <= len2; i++) {
        if (i == 0) {
            printf("   ");
        } else {
            printf("  %c  ", seq2[i-1]);
        }
        
        for (int j = 0; j <= len1; j++) {
            printf(" %3d ", dp_matrix[i][j]);
        }
        printf("\n");
    }
}
#endif

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
    
    printf("Running Serial Smith-Waterman with OpenMP\n");
    printf("Sequence 1: %s (Length: %zu)\n", seq1, strlen(seq1));
    printf("Sequence 2: %s (Length: %zu)\n", seq2, strlen(seq2));
    printf("Number of threads: %d\n", num_threads);
    
    smith_waterman(seq1, seq2, num_threads);
    
    #ifdef DEBUG
    printf("\nDynamic Programming (DP) Matrix:\n");
    print_dp_matrix(seq1, seq2);
    #endif
    
    return 0;
}
