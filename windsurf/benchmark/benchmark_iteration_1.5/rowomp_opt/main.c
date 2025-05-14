#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <immintrin.h>

// Configuration parameters
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -1
#define MAX_SEQ_LENGTH 3000
#define BLOCK_SIZE 16
#define CACHE_LINE 64

// Helper macros
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Cache-aligned matrix cell
typedef struct {
    int16_t value;
    char padding[CACHE_LINE - sizeof(int16_t)];
} __attribute__((aligned(CACHE_LINE))) padded_cell;

// Global DP matrix
static padded_cell dp[MAX_SEQ_LENGTH+1][MAX_SEQ_LENGTH+1];

// SIMD optimized block computation
static void compute_block(int start_i, int start_j, int block_height, int block_width,
                         const char* seq1, const char* seq2, int seq1_len, int seq2_len) {
    // Ensure we don't go out of bounds
    block_height = MIN(block_height, seq2_len - start_i + 1);
    block_width = MIN(block_width, seq1_len - start_j + 1);
    
    if (block_height <= 0 || block_width <= 0) return;
    
    // Load score constants
    __m256i match_scores = _mm256_set1_epi16(MATCH_SCORE);
    __m256i mismatch_scores = _mm256_set1_epi16(MISMATCH_SCORE);
    __m256i gap_scores = _mm256_set1_epi16(GAP_PENALTY);
    __m256i zero = _mm256_setzero_si256();
    
    // Process the block row by row
    for (int i = 0; i < block_height; i++) {
        int curr_i = start_i + i;
        if (curr_i <= 0) continue;
        
        // Process each row in SIMD chunks
        for (int j = 0; j < block_width; j += BLOCK_SIZE) {
            int curr_j = start_j + j;
            if (curr_j <= 0) continue;
            
            int vector_width = MIN(BLOCK_SIZE, block_width - j);
            
            // Prefetch next row's data
            if (i + 1 < block_height) {
                _mm_prefetch(&dp[curr_i + 1][curr_j].value, _MM_HINT_T0);
            }
            
            // Load diagonal, vertical, and horizontal values
            __m256i diag_vals = _mm256_setzero_si256();
            __m256i vert_vals = _mm256_setzero_si256();
            __m256i horiz_vals = _mm256_setzero_si256();
            
            // Calculate match/mismatch mask
            __m256i match_mask = _mm256_setzero_si256();
            
            for (int k = 0; k < vector_width; k++) {
                if (curr_j + k <= seq1_len && curr_i <= seq2_len) {
                    // Load values
                    diag_vals = _mm256_insert_epi16(diag_vals,
                        dp[curr_i-1][curr_j+k-1].value, k);
                    vert_vals = _mm256_insert_epi16(vert_vals,
                        dp[curr_i-1][curr_j+k].value, k);
                    horiz_vals = _mm256_insert_epi16(horiz_vals,
                        dp[curr_i][curr_j+k-1].value, k);
                    
                    // Check for match
                    if (seq1[curr_j+k-1] == seq2[curr_i-1]) {
                        match_mask = _mm256_insert_epi16(match_mask, -1, k);
                    }
                }
            }
            
            // Calculate scores
            __m256i match_mismatch = _mm256_blendv_epi8(mismatch_scores, match_scores, match_mask);
            __m256i diag_scores = _mm256_add_epi16(diag_vals, match_mismatch);
            __m256i vert_scores = _mm256_add_epi16(vert_vals, gap_scores);
            __m256i horiz_scores = _mm256_add_epi16(horiz_vals, gap_scores);
            
            // Find maximum
            __m256i max_scores = _mm256_max_epi16(
                _mm256_max_epi16(diag_scores, vert_scores),
                _mm256_max_epi16(horiz_scores, zero)
            );
            
            // Store results
            for (int k = 0; k < vector_width; k++) {
                if (curr_j + k <= seq1_len && curr_i <= seq2_len) {
                    dp[curr_i][curr_j+k].value = ((int16_t*)&max_scores)[k];
                }
            }
        }
    }
}

// Main Smith-Waterman algorithm implementation
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
    
    // Process matrix in blocks
    int block_size = BLOCK_SIZE * 4;  // Process 4 SIMD vectors at a time
    
    // Calculate number of blocks
    int num_blocks_i = (seq2_len + block_size - 1) / block_size;
    int num_blocks_j = (seq1_len + block_size - 1) / block_size;
    
    // Process blocks in parallel
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int block_i = 0; block_i < num_blocks_i; block_i++) {
        for (int block_j = 0; block_j < num_blocks_j; block_j++) {
            int start_i = block_i * block_size;
            int start_j = block_j * block_size;
            
            compute_block(start_i, start_j, block_size, block_size,
                         seq1, seq2, seq1_len, seq2_len);
        }
    }
    
    // Find maximum score
    int16_t max_score = 0;
    int max_i = 0, max_j = 0;
    
    #pragma omp parallel for collapse(2) reduction(max:max_score)
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
