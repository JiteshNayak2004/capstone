#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <immintrin.h>

// Configuration parameters
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1
#define MAX_SEQ_LENGTH 3000
#define CACHE_LINE 64
#define VECTOR_WIDTH 16  // Process 16 elements at once with AVX2
#define MIN_PARALLEL_SIZE 64  // Minimum diagonal length for parallel processing

// Helper macros
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Cache-aligned DP matrix with padding to prevent false sharing
typedef struct {
    int16_t value;
    char padding[CACHE_LINE - sizeof(int16_t)];
} __attribute__((aligned(CACHE_LINE))) padded_cell;

static padded_cell dp[MAX_SEQ_LENGTH+1][MAX_SEQ_LENGTH+1];

// SIMD optimized score calculation for a diagonal segment
static void process_diagonal_segment_simd(int diagonal_idx, int start, int end,
                                        const char* seq1, const char* seq2,
                                        int seq1_len, int seq2_len) {
    // Calculate starting positions
    int i = (diagonal_idx < seq1_len) ? 0 : (diagonal_idx - seq1_len + 1);
    int j = diagonal_idx - i;
    
    // Skip if out of bounds
    if (i >= seq2_len || j < 0) return;
    
    // Adjust bounds for valid matrix positions
    start = MAX(start, 0);
    end = MIN(end, MIN(seq2_len - i, j + 1));
    
    // Process 16 elements at a time using SIMD
    int vector_count = (end - start) / VECTOR_WIDTH;
    
    for (int v = 0; v < vector_count; v++) {
        int offset = v * VECTOR_WIDTH;
        int curr_i = i + start + offset;
        int curr_j = j - start - offset;
        
        // Skip if we're at the boundary
        if (curr_i <= 0 || curr_j <= 0) continue;
        
        // Prefetch next elements
        if (curr_i + VECTOR_WIDTH < seq2_len && curr_j - VECTOR_WIDTH > 0) {
            _mm_prefetch(&dp[curr_i + VECTOR_WIDTH][curr_j - VECTOR_WIDTH].value, _MM_HINT_T0);
        }
        
        // Load match/mismatch scores
        __m256i match_scores = _mm256_set1_epi16(MATCH_SCORE);
        __m256i mismatch_scores = _mm256_set1_epi16(MISMATCH_SCORE);
        __m256i gap_scores = _mm256_set1_epi16(GAP_PENALTY);
        
        // Calculate match mask
        __m256i match_mask = _mm256_set1_epi16(0);
        for (int k = 0; k < VECTOR_WIDTH; k++) {
            if (curr_i + k < seq2_len && curr_j - k > 0 && 
                seq1[curr_j - k - 1] == seq2[curr_i + k - 1]) {
                match_mask = _mm256_insert_epi16(match_mask, 1, k);
            }
        }
        
        // Calculate scores
        __m256i scores = _mm256_blendv_epi8(mismatch_scores, match_scores, match_mask);
        
        // Load diagonal, vertical, and horizontal values
        __m256i diag_vals = _mm256_setzero_si256();
        __m256i vert_vals = _mm256_setzero_si256();
        __m256i horiz_vals = _mm256_setzero_si256();
        
        for (int k = 0; k < VECTOR_WIDTH; k++) {
            if (curr_i + k < seq2_len && curr_j - k > 0) {
                diag_vals = _mm256_insert_epi16(diag_vals, 
                    dp[curr_i + k - 1][curr_j - k - 1].value, k);
                vert_vals = _mm256_insert_epi16(vert_vals,
                    dp[curr_i + k - 1][curr_j - k].value, k);
                horiz_vals = _mm256_insert_epi16(horiz_vals,
                    dp[curr_i + k][curr_j - k - 1].value, k);
            }
        }
        
        // Calculate maximum values
        __m256i diag_scores = _mm256_add_epi16(diag_vals, scores);
        __m256i vert_scores = _mm256_add_epi16(vert_vals, gap_scores);
        __m256i horiz_scores = _mm256_add_epi16(horiz_vals, gap_scores);
        
        __m256i max_vals = _mm256_max_epi16(
            _mm256_max_epi16(diag_scores, vert_scores),
            _mm256_max_epi16(horiz_scores, _mm256_setzero_si256())
        );
        
        // Store results
        for (int k = 0; k < VECTOR_WIDTH; k++) {
            if (curr_i + k < seq2_len && curr_j - k > 0) {
                dp[curr_i + k][curr_j - k].value = ((int16_t*)&max_vals)[k];
            }
        }
    }
    
    // Handle remaining elements
    int remaining_start = start + vector_count * VECTOR_WIDTH;
    for (int k = remaining_start; k < end; k++) {
        int curr_i = i + k;
        int curr_j = j - k;
        
        if (curr_i <= 0 || curr_j <= 0 || curr_i >= seq2_len || curr_j >= seq1_len) continue;
        
        int match = (seq1[curr_j-1] == seq2[curr_i-1]) ? MATCH_SCORE : MISMATCH_SCORE;
        
        dp[curr_i][curr_j].value = MAX(0,
            MAX(dp[curr_i-1][curr_j-1].value + match,
                MAX(dp[curr_i-1][curr_j].value + GAP_PENALTY,
                    dp[curr_i][curr_j-1].value + GAP_PENALTY)));
    }
}

// Main Smith-Waterman algorithm with optimized anti-diagonal parallelization
void smith_waterman(const char* seq1, const char* seq2, int num_threads) {
    int seq1_len = strlen(seq1);
    int seq2_len = strlen(seq2);
    
    if (seq1_len > MAX_SEQ_LENGTH || seq2_len > MAX_SEQ_LENGTH) {
        fprintf(stderr, "Error: Sequence length exceeds maximum allowed length\n");
        exit(1);
    }
    
    // Initialize matrix
    #pragma omp parallel for collapse(2)
    for (int i = 0; i <= seq2_len; i++) {
        for (int j = 0; j <= seq1_len; j++) {
            dp[i][j].value = 0;
        }
    }
    
    double start_time = omp_get_wtime();
    
    // Set number of threads
    omp_set_num_threads(num_threads);
    
    int total_diagonals = seq1_len + seq2_len - 1;
    
    // Process diagonals with dynamic scheduling and adaptive parallelism
    #pragma omp parallel
    {
        #pragma omp for schedule(guided)
        for (int diagonal = 1; diagonal < total_diagonals; diagonal++) {
            int diag_length = MIN(
                MIN(diagonal + 1, total_diagonals - diagonal),
                MIN(seq1_len, seq2_len)
            );
            
            if (diag_length >= MIN_PARALLEL_SIZE) {
                // Divide diagonal into chunks for parallel processing
                int chunk_size = (diag_length + num_threads - 1) / num_threads;
                chunk_size = (chunk_size + VECTOR_WIDTH - 1) & ~(VECTOR_WIDTH - 1); // Align to vector width
                
                #pragma omp parallel for schedule(dynamic)
                for (int chunk_start = 0; chunk_start < diag_length; chunk_start += chunk_size) {
                    int chunk_end = MIN(chunk_start + chunk_size, diag_length);
                    process_diagonal_segment_simd(diagonal, chunk_start, chunk_end,
                                               seq1, seq2, seq1_len + 1, seq2_len + 1);
                }
            } else {
                // Process short diagonals serially
                process_diagonal_segment_simd(diagonal, 0, diag_length,
                                           seq1, seq2, seq1_len + 1, seq2_len + 1);
            }
        }
    }
    
    // Find maximum score
    int16_t max_score = 0;
    int max_i = 0, max_j = 0;
    
    #pragma omp parallel for reduction(max:max_score) collapse(2)
    for (int i = 0; i <= seq2_len; i++) {
        for (int j = 0; j <= seq1_len; j++) {
            if (dp[i][j].value > max_score) {
                #pragma omp critical
                {
                    if (dp[i][j].value > max_score) {
                        max_score = dp[i][j].value;
                        max_i = i;
                        max_j = j;
                    }
                }
            }
        }
    }
    
    double end_time = omp_get_wtime();
    printf("Time taken: %f seconds\n", end_time - start_time);
    printf("Maximum score: %d at position (%d,%d)\n", max_score, max_i, max_j);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <sequence1> <sequence2> <num_threads>\n", argv[0]);
        exit(1);
    }
    
    const char* seq1 = argv[1];
    const char* seq2 = argv[2];
    int num_threads = atoi(argv[3]);
    
    smith_waterman(seq1, seq2, num_threads);
    
    return 0;
}
