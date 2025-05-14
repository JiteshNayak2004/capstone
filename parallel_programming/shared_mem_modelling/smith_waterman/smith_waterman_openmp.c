#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <stdbool.h>

// DEFINITIONS
#define MAX_SEQ_LENGTH 10
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -2

// Number of threads to use
#define NUM_THREADS 4

// SEQUENCE DATA
const char seq1[] = "CTCCAAAAAAGGGGGGGGGGGGGGGGGGGGGGCCCCCCCCCCCCCCCCCC";
const char seq2[] = "AAACCCCCCCCCCCCCCCCCCCCCCCCCC";

// DP matrix
int16_t dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH];

// Function to compute score
int16_t score(char a, char b) {
    if (a == b) return MATCH_SCORE;
    return MISMATCH_SCORE;
}

// Function to find min
int16_t minimum(int16_t x, int16_t y) {
    return (x < y) ? x : y;
}

// Function to find max
int16_t maximum(int16_t x, int16_t y) {
    return (x > y) ? x : y;
}

// Computes length of diagonal given index
int diagonal_length_cal(int diagonal_index, int seq1_len, int seq2_len) {
    int num_diagonal_elements;
    int min_dim = minimum(seq1_len, seq2_len);
    int max_dim = maximum(seq1_len, seq2_len);
    
    if (diagonal_index < min_dim) {
        num_diagonal_elements = diagonal_index + 1;
    } else if (diagonal_index < max_dim) {
        num_diagonal_elements = min_dim;
    } else {
        num_diagonal_elements = seq1_len + seq2_len - 1 - diagonal_index;
    }
    
    return num_diagonal_elements;
}

// Function to compute a segment of a diagonal
void compute_diagonal_segment(int diagonal_index, int start, int end, 
                            int seq1_len, int seq2_len) {
    int count = 0;
    for(int i = diagonal_index; i >= 0; i--) {
        for(int j = 0; j <= diagonal_index; j++) {
            if(((i + j) == diagonal_index) && (i <= seq1_len-1) && (j <= seq2_len-1)) {
                if((count >= start) && (count < end)) {
                    if((i == 0) && (j == 0)) {
                        int16_t match = score(seq1[i], seq2[j]);
                        int16_t delete = GAP_SCORE;
                        int16_t insert = GAP_SCORE;
                        
                        int16_t max_score = match > delete ? 
                            (match > insert ? match : insert) : 
                            (delete > insert ? delete : insert);
                        max_score = max_score > 0 ? max_score : 0;
                        dp[i][j] = max_score;
                    }
                    else if(i == 0) {
                        int16_t match = score(seq1[i], seq2[j]);
                        int16_t delete = GAP_SCORE;
                        int16_t insert = dp[i][j-1] + GAP_SCORE;
                        
                        int16_t max_score = match > delete ? 
                            (match > insert ? match : insert) : 
                            (delete > insert ? delete : insert);
                        max_score = max_score > 0 ? max_score : 0;
                        dp[i][j] = max_score;
                    }
                    else if(j == 0) {
                        int16_t match = score(seq1[i], seq2[j]);
                        int16_t delete = dp[i-1][j] + GAP_SCORE;
                        int16_t insert = GAP_SCORE;
                        
                        int16_t max_score = match > delete ? 
                            (match > insert ? match : insert) : 
                            (delete > insert ? delete : insert);
                        max_score = max_score > 0 ? max_score : 0;
                        dp[i][j] = max_score;
                    }
                    else {
                        int16_t match = dp[i-1][j-1] + score(seq1[i], seq2[j]);
                        int16_t delete = dp[i-1][j] + GAP_SCORE;
                        int16_t insert = dp[i][j-1] + GAP_SCORE;
                        
                        int16_t max_score = match > delete ? 
                            (match > insert ? match : insert) : 
                            (delete > insert ? delete : insert);
                        max_score = max_score > 0 ? max_score : 0;
                        dp[i][j] = max_score;
                    }
                }
                else if(count == end) {
                    return;
                }
                count = count + 1;
            }
        }
    }
}

void smith_waterman() {
    int seq1_len = strlen(seq1);
    int seq2_len = strlen(seq2);
    int total_diagonals = seq1_len + seq2_len - 1;
    
    double start_time = omp_get_wtime();
    
    // Set the number of threads
    omp_set_num_threads(NUM_THREADS);
    
    // Process diagonals
    for (int diagonal_index = 0; diagonal_index < total_diagonals; diagonal_index++) {
        int num_diagonal_elements = diagonal_length_cal(diagonal_index, seq1_len, seq2_len);
        int active_threads = minimum(num_diagonal_elements, NUM_THREADS);
        int elements_per_thread = (num_diagonal_elements + active_threads - 1) / active_threads;
        
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            if (thread_id < active_threads) {
                int start = thread_id * elements_per_thread;
                int end = (thread_id == active_threads - 1) ? 
                    num_diagonal_elements : (thread_id + 1) * elements_per_thread;
                
                compute_diagonal_segment(diagonal_index, start, end, seq1_len, seq2_len);
            }
        }
        // Implicit barrier at the end of parallel region
    }
    
    double end_time = omp_get_wtime();
    printf("Time taken: %f seconds\n", end_time - start_time);
}

int main() {
    // Initialize the dp matrix
    memset(dp, 0, sizeof(dp));
    
    int seq1_len = strlen(seq1);
    int seq2_len = strlen(seq2);
    
    printf("Sequence 1: %s (Length: %d)\n", seq1, seq1_len);
    printf("Sequence 2: %s (Length: %d)\n", seq2, seq2_len);
    
    // Run Smith-Waterman algorithm
    smith_waterman();
    
    // Print the dp matrix
    // printf("\nFinal DP Matrix:\n");
    // for(int i = 0; i < seq1_len; i++) {
    //     for(int j = 0; j < seq2_len; j++) {
    //         printf("%d\t", dp[i][j]);
    //     }
    //     printf("\n");
    // }
    // 
    return 0;
}
