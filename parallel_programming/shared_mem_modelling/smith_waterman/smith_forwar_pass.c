#include <stdio.h>
#include <string.h>

// Define the maximum possible lengths for the sequences
#define MAX_SEQ1_LEN 1024
#define MAX_SEQ2_LEN 128

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

int max_score = 0;
int max_i = 0;
int max_j = 0;

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

// Global DP matrix
int dp_matrix[MAX_SEQ1_LEN + 1][MAX_SEQ2_LEN + 1];

// Function to calculate the Smith-Waterman DP matrix
void smith_waterman(const char* seq1, const char* seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Initialize DP matrix
    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            dp_matrix[i][j] = 0;
        }
    }

    // Fill DP matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
            int delete = dp_matrix[i-1][j] + GAP_PENALTY;
            int insert = dp_matrix[i][j-1] + GAP_PENALTY;
            dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));

            // Track the maximum score and its position
            if (dp_matrix[i][j] > max_score) {
                max_score = dp_matrix[i][j];
                max_i = i;
                max_j = j;
            }
        }
    }
}

int main() {
    const char* seq1 = "GCATAAAGATTTGAGTAAAGTCGGTAAGGGATTGTCGATCAATCAAACGGCCAGAGGGGAGATCGCGGATATATCCGAGCTTGGCTATTACCGGAGGCCTGCAAGTGGCGCTAAGTACCACGAGATCTAGGGCGTAACCACAAGGGCGATCAAGACACCCCCGCCGGGTATCACATAGGCGTCAGCCGACCAGTGCAACAGCGGCGCCTAATGTTGTTCGTACGTACAATGGTAGAAACGAAATCGCATAGCGATTGCGCCCATTTTGAACCGGCTAAGTCCATAACGAGTAACAAGGGTAAAAAGGCTTCGCTCACATGATCAAGGAAATGAAAACCATTACAGGATATACATGTGACCTCCCCTACGGGGCATCCTGCGTGTTGAACACTAGAAATGGCGTAGTTCTGGTGCTATGTCTTCCTTAGTCATTCTTGTGGTCCTTGAGCTATCTGGACGAATCCGGGACGCCGCAAGTTAGGAGCCTAAGTTGTTGCATA";
    const char* seq2 = "ACGAATCCGGGACGCCGCAAGTTAGGAGCCTAAGTTGTTGCATA";

    smith_waterman(seq1, seq2);

    printf("Max Score: %d at position (%d, %d)\n", max_score, max_i, max_j);

    return 0;
}

