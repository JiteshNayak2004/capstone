#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <immintrin.h>

// Define the maximum possible lengths for the sequences
#define MAX_SEQ_LENGTH 10000

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1

// Cache line size for padding
#define CACHE_LINE 64
#define VECTOR_SIZE 8  // Number of int16_t elements in a SIMD vector

// Helper macros
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// Inline max functions for better performance
static inline int16_t max2(int16_t a, int16_t b) {
    return (a > b) ? a : b;
}

static inline int16_t max3(int16_t a, int16_t b, int16_t c) {
    return max2(max2(a, b), c);
}

// Structure for thread-local data to prevent false sharing
typedef struct {
    int16_t local_max_score;
    int16_t local_max_i;
    int16_t local_max_j;
    char padding[CACHE_LINE - 3 * sizeof(int16_t)];
} __attribute__((aligned(CACHE_LINE))) ThreadData;

// Global DP matrix with cache line padding
__attribute__((aligned(CACHE_LINE))) int16_t dp_matrix[MAX_SEQ_LENGTH + 1][MAX_SEQ_LENGTH + 1] = {0};

// Structure to store alignment result
typedef struct {
    char* seq1_aligned;
    char* seq2_aligned;
    int alignment_length;
} AlignmentResult;

// Function to perform traceback and get the alignment
AlignmentResult* traceback(const char* seq1, const char* seq2, int max_i, int max_j) {
    AlignmentResult* result = (AlignmentResult*)malloc(sizeof(AlignmentResult));
    int max_possible_length = strlen(seq1) + strlen(seq2);  // Maximum possible alignment length
    
    result->seq1_aligned = (char*)malloc(max_possible_length * sizeof(char));
    result->seq2_aligned = (char*)malloc(max_possible_length * sizeof(char));
    
    int current_i = max_i;
    int current_j = max_j;
    int pos = 0;
    
    while (current_i > 0 && current_j > 0 && dp_matrix[current_i][current_j] > 0) {
        int match = (seq1[current_i-1] == seq2[current_j-1]) ? MATCH_SCORE : MISMATCH_SCORE;
        
        if (dp_matrix[current_i][current_j] == dp_matrix[current_i-1][current_j-1] + match) {
            result->seq1_aligned[pos] = seq1[current_i-1];
            result->seq2_aligned[pos] = seq2[current_j-1];
            current_i--; current_j--;
        } else if (dp_matrix[current_i][current_j] == dp_matrix[current_i-1][current_j] + GAP_PENALTY) {
            result->seq1_aligned[pos] = seq1[current_i-1];
            result->seq2_aligned[pos] = '-';
            current_i--;
        } else {
            result->seq1_aligned[pos] = '-';
            result->seq2_aligned[pos] = seq2[current_j-1];
            current_j--;
        }
        pos++;
    }
    
    // Reverse the strings since we built them backwards
    for (int i = 0; i < pos/2; i++) {
        char temp = result->seq1_aligned[i];
        result->seq1_aligned[i] = result->seq1_aligned[pos-1-i];
        result->seq1_aligned[pos-1-i] = temp;
        
        temp = result->seq2_aligned[i];
        result->seq2_aligned[i] = result->seq2_aligned[pos-1-i];
        result->seq2_aligned[pos-1-i] = temp;
    }
    
    result->seq1_aligned[pos] = '\0';
    result->seq2_aligned[pos] = '\0';
    result->alignment_length = pos;
    
    return result;
}

// Function to calculate the Smith-Waterman DP matrix using anti-diagonal approach with SIMD
void smith_waterman(const char* seq1, const char* seq2, int num_threads) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);
    
    if (len1 > MAX_SEQ_LENGTH || len2 > MAX_SEQ_LENGTH) {
        fprintf(stderr, "Error: Sequence length exceeds maximum allowed length\n");
        exit(1);
    }
    
    // Thread-local data array
    ThreadData* thread_data = (ThreadData*)aligned_alloc(CACHE_LINE, num_threads * sizeof(ThreadData));
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].local_max_score = 0;
        thread_data[i].local_max_i = 0;
        thread_data[i].local_max_j = 0;
    }
    
    // Set number of threads
    omp_set_num_threads(num_threads);
    
    double start_time = omp_get_wtime();
    
    // Total number of anti-diagonals
    int num_antidiags = len1 + len2 - 1;
    
    // Process anti-diagonals
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        for (int antidiag = 1; antidiag <= num_antidiags; antidiag++) {
            int start_i = max2(1, antidiag - len2 + 1);
            int end_i = MIN(len1, antidiag);
            int length = end_i - start_i + 1;
            
            // Parallelize within each anti-diagonal
            #pragma omp for schedule(dynamic, 16)
            for (int chunk = 0; chunk < length; chunk += VECTOR_SIZE) {
                int vec_size = MIN(VECTOR_SIZE, length - chunk);
                int i = start_i + chunk;
                int j = antidiag - i + 1;
                
                for (int v = 0; v < vec_size; v++) {
                    // Calculate similarity score
                    int16_t match = (seq1[i+v-1] == seq2[j+v-1]) ? MATCH_SCORE : MISMATCH_SCORE;
                    
                    // Calculate cell value
                    int16_t score = max3(
                        0,
                        dp_matrix[i+v-1][j+v-1] + match,
                        max2(
                            dp_matrix[i+v-1][j+v] + GAP_PENALTY,
                            dp_matrix[i+v][j+v-1] + GAP_PENALTY
                        )
                    );
                    
                    dp_matrix[i+v][j+v] = score;
                    
                    // Update thread-local maximum
                    if (score > thread_data[thread_id].local_max_score) {
                        thread_data[thread_id].local_max_score = score;
                        thread_data[thread_id].local_max_i = i+v;
                        thread_data[thread_id].local_max_j = j+v;
                    }
                }
            }
            #pragma omp barrier
        }
    }
    
    double dp_end_time = omp_get_wtime();
    double dp_time = dp_end_time - start_time;
    
    // Find global maximum from thread-local maxima
    int16_t max_score = 0;
    int max_i = 0, max_j = 0;
    
    for (int t = 0; t < num_threads; t++) {
        if (thread_data[t].local_max_score > max_score) {
            max_score = thread_data[t].local_max_score;
            max_i = thread_data[t].local_max_i;
            max_j = thread_data[t].local_max_j;
        }
    }
    
    // Perform traceback and time it
    double traceback_start_time = omp_get_wtime();
    AlignmentResult* alignment = traceback(seq1, seq2, max_i, max_j);
    double traceback_end_time = omp_get_wtime();
    double traceback_time = traceback_end_time - traceback_start_time;
    
    // Print timing results
    printf("DP Matrix Computation Time: %f seconds\n", dp_time);
    printf("Traceback Time: %f seconds\n", traceback_time);
    printf("Total Time: %f seconds\n", dp_time + traceback_time);
    printf("Max Score: %d at position (%d, %d)\n", max_score, max_i, max_j);
    printf("Alignment length: %d\n", alignment->alignment_length);
    
    // Free memory
    free(thread_data);
    free(alignment->seq1_aligned);
    free(alignment->seq2_aligned);
    free(alignment);
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
    
    char seq1[MAX_SEQ_LENGTH + 1];
    char seq2[MAX_SEQ_LENGTH + 1];
    
    if (!fgets(seq1, MAX_SEQ_LENGTH, f1) || !fgets(seq2, MAX_SEQ_LENGTH, f2)) {
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
