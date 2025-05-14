#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

// Define the maximum possible lengths for the sequences
//#define MAX_SEQ1_LEN 40
//#define MAX_SEQ2_LEN 20
#define cc 1

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

// Global DP matrix
int16_t dp_matrix[20 + 1][40 + 1]={0};

// Function to calculate the Smith-Waterman DP matrix
void smith_waterman(const char* seq1, const char* seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Initialize DP matrix
    for (int i = 0; i <= len1; i++) {
        dp_matrix[i][0] = 0; // Initialize leftmost column to 0
    }
    for (int j = 0; j <= len2; j++) {
        dp_matrix[0][j] = 0; // Initialize topmost row to 0
    }

    int16_t max_score = 0; // Reset max_score
    int16_t max_i = 0;     // Reset position of max score
    int16_t max_j = 0;
//    int roof_start=mempool_get_timer();
    // Fill DP matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int16_t match = dp_matrix[i - 1][j - 1] +
                        (seq1[i - 1] == seq2[j - 1] ? MATCH_SCORE : MISMATCH_SCORE);
            int16_t delete = dp_matrix[i - 1][j] + GAP_PENALTY;
            int16_t insert = dp_matrix[i][j - 1] + GAP_PENALTY;

            dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));

            // Track the maximum score and its position
            if (dp_matrix[i][j] > max_score) {
                max_score = dp_matrix[i][j];
                max_i = i;
                max_j = j;
            }
        }
    }

//    int roof_end=mempool_get_timer();
    // Print results
//    printf("Max score: %d at position (%d, %d)\n", max_score, max_i, max_j);
//    printf("the roof start is %d and roof end is %d",roof_start,roof_end);

}
//
//// Function to print the DP matrix
//void print_dp_matrix(const char* seq1, const char* seq2) {
//    int len1 = strlen(seq1);
//    int len2 = strlen(seq2);
//
//    // Print top-left corner padding and the column headers (seq2 characters)
//    printf("    "); // Padding for top-left corner
//    for (int j = 0; j < len2; j++) {
//        printf("  %c", seq2[j]); // Column headers (seq2 characters)
//    }
//    printf("\n");
//
//    // Print the DP matrix with row labels (seq1 characters)
//    for (int i = 0; i <= len1; i++) {
//        // Print the row label (seq1 characters) for rows except the first one
//        if (i == 0) {
//            printf("   ");  // Padding for the first row (column labels)
//        } else {
//            printf("  %c  ", seq1[i - 1]); // Row labels (seq1 characters)
//        }
//
//        // Print each value of the DP matrix with padding
//        for (int j = 0; j <= len2; j++) {
//            printf(" %3d ", dp_matrix[i][j]); // Print DP matrix values with padding
//        }
//        printf("\n");
//    }
//}
//

int main() {
  uint32_t core_id = mempool_get_core_id();
  mempool_barrier_init(core_id);

  if (core_id >=cc) {
    while(1);}


  if (core_id == 0) {
    const char* seq1 = "GATGGGCGTGCCAAAGGAGATCGGACGAAATGGATTTCGA";
    const char* seq2 = "TGCGGGCCAAGATAGGCGGT";
    uint32_t start_cycles=mempool_get_timer();
    smith_waterman(seq1, seq2);
    uint32_t end_cycles=mempool_get_timer();
//    uint32_t compute_time=end_cycles-start_cycles;
//    printf("Alignment of dp_matrix: %zu\n", alignof(dp_matrix));
//    printf("\nDynamic Programming (DP) Matrix:\n");
    printf("start cycles is %d and end cycles is %d\n",start_cycles,end_cycles);
//    printf("compute time is %d ",compute_time);
    //print_dp_matrix(seq1, seq2);
  }

  return 0;
}

