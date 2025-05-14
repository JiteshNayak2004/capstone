#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

// Scoring scheme
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

// Function to calculate the Smith-Waterman DP matrix
void smith_waterman(const char* seq1, const char* seq2) {
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Allocate DP matrix
    int** dp_matrix = (int**) malloc((len1 + 1) * sizeof(int*));
    for (int i = 0; i <= len1; i++) {
        dp_matrix[i] = (int*) malloc((len2 + 1) * sizeof(int));
    }

    // Initialize DP matrix
    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            dp_matrix[i][j] = 0;
        }
    }

    // Variables to track maximum score and position
    int max_score = 0;
    int max_i = 0;
    int max_j = 0;

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

    // Backtracking step
    printf("\n--- Smith-Waterman Results ---\n");
    printf("Max Score: %d\n", max_score);

    int i = max_i;
    int j = max_j;
    char aligned_seq1[len1 + len2];
    char aligned_seq2[len1 + len2];
    int pos = 0;

    while (i > 0 && j > 0 && dp_matrix[i][j] != 0) {
        if (dp_matrix[i][j] == dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE)) {
            // Match or Mismatch
            aligned_seq1[pos] = seq1[i-1];
            aligned_seq2[pos] = seq2[j-1];
            i--;
            j--;
        } else if (dp_matrix[i][j] == dp_matrix[i-1][j] + GAP_PENALTY) {
            // Deletion
            aligned_seq1[pos] = seq1[i-1];
            aligned_seq2[pos] = '-';
            i--;
        } else { // Insertion
            aligned_seq1[pos] = '-';
            aligned_seq2[pos] = seq2[j-1];
            j--;
        }
        pos++;
    }

    // Reverse the aligned sequences
    aligned_seq1[pos] = '\0';
    aligned_seq2[pos] = '\0';
    for (int k = 0; k < pos / 2; k++) {
        char temp = aligned_seq1[k];
        aligned_seq1[k] = aligned_seq1[pos - k - 1];
        aligned_seq1[pos - k - 1] = temp;

        temp = aligned_seq2[k];
        aligned_seq2[k] = aligned_seq2[pos - k - 1];
        aligned_seq2[pos - k - 1] = temp;
    }

    // Print the aligned sequences
    printf("Aligned Query:     %s\n", aligned_seq2);
    printf("Aligned Reference: %s\n", aligned_seq1);

    // Calculate alignment positions in the reference
    int start_position = i;
    int end_position = max_i;

    printf("Alignment index Start: %d\n", start_position);
    printf("Alignment index End:   %d\n", end_position);

    // Free allocated memory
    for (int i = 0; i <= len1; i++) {
        free(dp_matrix[i]);
    }
    free(dp_matrix);
}

int main() {

    const char* seq1 = "GCATAAAGATTTGAGTAAAGTCGGTAAGGGATTGTCGATCAATCAAACGGCCAGAGGGGAGATCGCGGATATATCCGAGCTTGGCTATTACCGGAGGCCTGCAAGTGGCGCTAAGTACCACGAGATCTAGGGCGTAACCACAAGGGCGATCAAGACACCCCCGCCGGGTATCACATAGGCGTCAGCCGACCAGTGCAACAGCGGCGCCTAATGTTGTTCGTACGTACAATGGTAGAAACGAAATCGCATAGCGATTGCGCCCATTTTGAACCGGCTAAGTCCATAACGAGTAACAAGGGTAAAAAGGCTTCGCTCACATGATCAAGGAAATGAAAACCATTACAGGATATACATGTGACCTCCCCTACGGGGCATCCTGCGTGTTGAACACTAGAAATGGCGTAGTTCTGGTGCTATGTCTTCCTTAGTCATTCTTGTGGTCCTTGAGCTATCTGGACGAATCCGGGACGCCGCAAGTTAGGAGCCTAAGTTGTTGCATA";
    const char* seq2 = "ACGAATCCGGGACGCCGCAAGTTAGGAGCCTAAGTTGTTGCATA";
    smith_waterman(seq1, seq2);

    return 0;
}

