#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <stdlib.h>

// Configuration parameters
// Scoring scheme for DNA sequences (matching SSW)
#define MATCH_SCORE 2
#define MISMATCH_SCORE -2  // SSW uses 2 as penalty
#define GAP_OPEN -3       // SSW uses 3 as penalty
#define GAP_EXTENSION -1  // SSW uses 1 as penalty
#define MAX_SEQ_LENGTH_1 30000
#define MAX_SEQ_LENGTH_2 30000

// DP matrix
int16_t dp[MAX_SEQ_LENGTH_1+1][MAX_SEQ_LENGTH_2+1] = {0};

// Inline utility function for better performance
static inline int16_t max(int16_t a, int16_t b) {
    return (a > b) ? a : b;
}

// Main Smith-Waterman algorithm implementation
void smith_waterman(const char* seq1, const char* seq2, int num_threads) {
    int seq1_len = strlen(seq1);
    int seq2_len = strlen(seq2);
    
    if (seq1_len > MAX_SEQ_LENGTH_1 || seq2_len > MAX_SEQ_LENGTH_2) {
        fprintf(stderr, "Error: Sequence length exceeds maximum allowed length\n");
        exit(1);
    }
    
    double start_time = omp_get_wtime();
    
    // Set number of threads
    omp_set_num_threads(num_threads);
    
    // Fill the DP matrix
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int i = 1; i <= seq2_len; i++) {
            for (int j = 1; j <= seq1_len; j++) {
                int16_t match_score = (seq1[j - 1] == seq2[i - 1]) ? MATCH_SCORE : MISMATCH_SCORE;
                dp[i][j] = max(max(dp[i-1][j] + GAP_OPEN + GAP_EXTENSION, dp[i][j-1] + GAP_OPEN + GAP_EXTENSION), dp[i-1][j-1] + match_score);
                dp[i][j] = max(dp[i][j], 0); // Ensure no negative values
            }
        }
    }
    
    // Find maximum score
    int16_t max_score = 0;
    int max_i = 0, max_j = 0;
    #pragma omp parallel for reduction(max:max_score) collapse(2)
    for (int i = 0; i <= seq2_len; i++) {
        for (int j = 0; j <= seq1_len; j++) {
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
    
    // Traceback to find optimal alignment
    char aligned_seq1[MAX_SEQ_LENGTH_1 + MAX_SEQ_LENGTH_2] = {0};
    char aligned_seq2[MAX_SEQ_LENGTH_1 + MAX_SEQ_LENGTH_2] = {0};
    int alignment_length = 0;
    
    int i = max_i;
    int j = max_j;
    while (i > 0 && j > 0 && dp[i][j] > 0) {
        int16_t current_score = dp[i][j];
        int16_t diagonal_score = dp[i-1][j-1];
        int16_t up_score = dp[i-1][j];
        int16_t left_score = dp[i][j-1];
        
        if (current_score == diagonal_score + ((seq1[j - 1] == seq2[i - 1]) ? MATCH_SCORE : MISMATCH_SCORE)) {
            aligned_seq1[alignment_length] = seq1[j - 1];
            aligned_seq2[alignment_length] = seq2[i - 1];
            i--; j--;
        } else if (current_score == up_score + GAP_OPEN + GAP_EXTENSION) {
            aligned_seq1[alignment_length] = '-';
            aligned_seq2[alignment_length] = seq2[i - 1];
            i--;
        } else if (current_score == left_score + GAP_OPEN + GAP_EXTENSION) {
            aligned_seq1[alignment_length] = seq1[j - 1];
            aligned_seq2[alignment_length] = '-';
            j--;
        }
        alignment_length++;
    }
    
    // Reverse the alignment strings
    for (int k = 0; k < alignment_length / 2; k++) {
        char temp = aligned_seq1[k];
        aligned_seq1[k] = aligned_seq1[alignment_length - k - 1];
        aligned_seq1[alignment_length - k - 1] = temp;
        
        temp = aligned_seq2[k];
        aligned_seq2[k] = aligned_seq2[alignment_length - k - 1];
        aligned_seq2[alignment_length - k - 1] = temp;
    }
    
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;
    uint64_t cells = (uint64_t)seq1_len * (uint64_t)seq2_len;
    double gcups = (cells / time_taken) / 1e9;
    
    printf("Row-Parallel Smith-Waterman Results:\n");
    printf("Max alignment score: %d at position (%d, %d)\n", max_score, max_i, max_j);
    printf("Optimal alignment:\n%s\n%s\n", aligned_seq1, aligned_seq2);
    printf("Execution time: %.6f seconds\n", time_taken);
    printf("Performance: %.2f GCUPS (%.2f billion cell updates per second)\n", gcups, gcups);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <sequence1_file> <sequence2_file> <num_threads>\n", argv[0]);
        return 1;
    }
    
    // Read sequences from files
    FILE *f1 = fopen(argv[1], "r");
    FILE *f2 = fopen(argv[2], "r");
    if (!f1 || !f2) {
        fprintf(stderr, "Error: Could not open input files\n");
        return 1;
    }
    
    char seq1[MAX_SEQ_LENGTH_1 + 1];
    char seq2[MAX_SEQ_LENGTH_2 + 1];
    
    if (!fgets(seq1, MAX_SEQ_LENGTH_1, f1) || !fgets(seq2, MAX_SEQ_LENGTH_2, f2)) {
        fprintf(stderr, "Error: Could not read sequences\n");
        return 1;
    }
    
    fclose(f1);
    fclose(f2);
    
    // Remove newlines if present
    seq1[strcspn(seq1, "\n")] = 0;
    seq2[strcspn(seq2, "\n")] = 0;
    
    int num_threads = atoi(argv[3]);
    if (num_threads < 1) {
        fprintf(stderr, "Error: Number of threads must be positive\n");
        return 1;
    }
    
    smith_waterman(seq1, seq2, num_threads);
    
    return 0;
}
