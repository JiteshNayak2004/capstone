#include <stdio.h>
#include <stdint.h>
#include <math.h>


#define MAX_SEQ_LENGTH 5
#define GAP_SCORE -1

// Global variables
char seq1[MAX_SEQ_LENGTH + 1] = "ABC"; // Sequence 1
char seq2[MAX_SEQ_LENGTH + 1] = "ABD"; // Sequence 2
int seq1_len = 3; // Length of seq1
int seq2_len = 3; // Length of seq2
int cc = 2; // Number of cores (for testing purposes)
int dp[MAX_SEQ_LENGTH + 1][MAX_SEQ_LENGTH + 1]; // DP matrix

// Function to initialize the DP matrix
void initialize_dp_matrix() {
    for (int i = 0; i <= seq1_len; i++) {
        dp[i][0] = i * GAP_SCORE; // Fill first column
    }
    for (int j = 0; j <= seq2_len; j++) {
        dp[0][j] = j * GAP_SCORE; // Fill first row
    }
}
int minimum(int x, int y)
{
  return (x < y) ? x : y;
}

int maximum(int x, int y)
{
  return (x > y) ? x : y;
}
int diagonal_length_cal(int diagonal_index) {
	int num_diagonal_elements;
	int min_dim=minimum(seq1_len,seq2_len);
	int max_dim=maximum(seq1_len,seq2_len);
	if (diagonal_index <= min_dim) {
	num_diagonal_elements = diagonal_index+1;}
	else if  (diagonal_index <= max_dim){
	num_diagonal_elements= min_dim;}
	else{
	num_diagonal_elements= seq1_len + seq2_len+1 - diagonal_index;}

	return num_diagonal_elements;
}

// Smith-Waterman function
// iterates through all diagonals and divides the diagonal_elements equally
void smith_waterman(uint32_t core_id) {
    // cuz it's m+1+n+1-1
    int total_diagonals = seq1_len + seq2_len;

    for (int diagonal_index = 0; diagonal_index <= total_diagonals; diagonal_index++) {
	// Calculate the diagonal length

	int num_diagonal_elements;
	num_diagonal_elements=diagonal_length_cal(diagonal_index);

	int active_cores=minimum(num_diagonal_elements,cc);
	int elements_per_core = (int)ceil((double)num_diagonal_elements / active_cores);
	printf("elements per core are %d   \n",elements_per_core);
	int new_elements_per_core = (num_diagonal_elements + active_cores - 1) / active_cores;
	printf("elements per core in the new method are %d   \n",elements_per_core);
	int start = core_id * elements_per_core;
	int end = (core_id == cc - 1) ? num_diagonal_elements : (core_id + 1) * elements_per_core;

	if(start==end) {
	    //printf("no work for core_id==%d for this diagonal\n",core_id);
	}

	// Process the diagonal segment (for demonstration, just print the indices)
	//printf("Core %d, Diagonal %d: Processing from element %d to element %d\n", core_id, diagonal_index, start, end);
    }
}

// Test function for different core IDs
void test_smith_waterman() {
	initialize_dp_matrix();

    for (uint32_t core_id = 0; core_id < cc; core_id++) {
        //printf("Testing with core ID: %d\n", core_id);
        smith_waterman(core_id);
    }
}

int main() {
    test_smith_waterman();
    return 0;
}

