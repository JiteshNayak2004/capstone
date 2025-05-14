#include <stdint.h>
#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"

#define MAX_SEQ_LENGTH 100
#define MATCH_SCORE 2
#define MISMATCH_SCORE -1
#define GAP_SCORE -1

#define CORES_PER_TILE 4
#define TILES_PER_GROUP 16
#define GROUPS_PER_CLUSTER 4
#define SPM_SIZE 1024 // 1 KiB

// Custom core count (modify this to change the number of active cores)
#define cc 4

// Sequences
char seq1[MAX_SEQ_LENGTH];
char seq2[MAX_SEQ_LENGTH];
int seq1_len, seq2_len;

// DP matrix
uint16_t dp[MAX_SEQ_LENGTH + 1][MAX_SEQ_LENGTH + 1];

// Timing variables
uint32_t compute_time = 0;
uint32_t sync_time = 0;
uint32_t communication_time = 0;

// Communication counters
uint32_t intra_tile_comm = 0;
uint32_t inter_tile_same_group_comm = 0;
uint32_t inter_group_comm = 0;

// Function to compute score
int score(char a, char b) {
  if (a == b) return MATCH_SCORE;
  return MISMATCH_SCORE;
}

// Function to analyze communication type
void analyze_communication(int i, int j) {
  uint32_t address = (uint32_t)&dp[i][j];
  uint32_t tile_index = address / (SPM_SIZE * CORES_PER_TILE);
  uint32_t group_index = tile_index / TILES_PER_GROUP;
  uint32_t core_tile_index = mempool_get_core_id() / CORES_PER_TILE;
  uint32_t core_group_index = core_tile_index / TILES_PER_GROUP;

  if (tile_index == core_tile_index) {
    intra_tile_comm++;
  } else if (group_index == core_group_index) {
    inter_tile_same_group_comm++;
  } else {
    inter_group_comm++;
  }
}

// Function to get value from DP matrix with communication analysis
int get_dp_value(int i, int j) {
  uint32_t start_time = mempool_get_timer();
  int value = dp[i][j];
  uint32_t end_time = mempool_get_timer();
  communication_time += end_time - start_time;

  analyze_communication(i, j);

  return value;
}

// Function to set value in DP matrix with communication analysis
void set_dp_value(int i, int j, int value) {
  uint32_t start_time = mempool_get_timer();
  dp[i][j] = value;
  uint32_t end_time = mempool_get_timer();
  communication_time += end_time - start_time;

  analyze_communication(i, j);
}

// Function to compute a segment of a diagonal
void compute_diagonal_segment(int diagonal, int start, int end) {
  for (int k = start; k < end && k <= diagonal; k++) {
    int i = k;
    int j = diagonal - k;

    if (i <= seq1_len && j <= seq2_len) {
      int match = get_dp_value(i-1, j-1) + score(seq1[i-1], seq2[j-1]);
      int delete = get_dp_value(i-1, j) + GAP_SCORE;
      int insert = get_dp_value(i, j-1) + GAP_SCORE;

      int max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
      max_score = max_score > 0 ? max_score : 0; // Ensure non-negative scores

      set_dp_value(i, j, max_score);
    }
  }
}

// Main Smith-Waterman function
void smith_waterman(uint32_t core_id) {
  int total_diagonals = seq1_len + seq2_len;

  for (int diagonal = 1; diagonal <= total_diagonals; diagonal++) {
    mempool_barrier(cc);
    int diagonal_length = (diagonal <= seq1_len + 1) ? diagonal : (seq1_len + 1 + seq2_len + 1 - diagonal);
    int elements_per_core = (diagonal_length + cc - 1) / cc;
    int start = core_id * elements_per_core;
    int end = (core_id == cc - 1) ? diagonal_length : (core_id + 1) * elements_per_core;

    uint32_t start_time = mempool_get_timer();
    compute_diagonal_segment(diagonal, start, end);
    uint32_t end_time = mempool_get_timer();
    compute_time += end_time - start_time;

    start_time = mempool_get_timer();
    mempool_barrier(cc);
    end_time = mempool_get_timer();
    sync_time += end_time - start_time;
  }
}

int main() {
  uint32_t core_id = mempool_get_core_id();

  // Initialize synchronization

  mempool_barrier_init(core_id);

  // extra cores loooopeeed
  if(core_id>cc) {
    while(1);}



  // Initialize sequences (only by core 0)
  if (core_id == 0) {
    // Example sequences
    const char* s1 = "ACGTACGT";
    const char* s2 = "AGTCAGTC";

    seq1_len = 0;
    while (s1[seq1_len] != '\0' && seq1_len < MAX_SEQ_LENGTH) {
      seq1[seq1_len] = s1[seq1_len];
      seq1_len++;
    }

    seq2_len = 0;
    while (s2[seq2_len] != '\0' && seq2_len < MAX_SEQ_LENGTH) {
      seq2[seq2_len] = s2[seq2_len];
      seq2_len++;
    }
  }

  // Ensure all cores have the sequence information
  mempool_barrier(cc);

  uint32_t start_time = mempool_get_timer();

  // Run Smith-Waterman algorithm
  smith_waterman(core_id);

  uint32_t end_time = mempool_get_timer();
  uint32_t total_time = end_time - start_time;

  // Aggregate communication stats across all cores
  uint32_t total_intra_tile = 0, total_inter_tile_same_group = 0, total_inter_group = 0;
  mempool_barrier(cc);
  for (int i = 0; i < cc; i++) {
    if (core_id == i) {
      total_intra_tile += intra_tile_comm;
      total_inter_tile_same_group += inter_tile_same_group_comm;
      total_inter_group += inter_group_comm;
    }
    mempool_barrier(cc);
  }

  // Print results (only core 0)
  if (core_id == 0) {
    int max_score = 0;
    for (int i = 0; i <= seq1_len; i++) {
      for (int j = 0; j <= seq2_len; j++) {
        if (dp[i][j] > max_score) {
          max_score = dp[i][j];
        }
      }
    }

    printf("Maximum alignment score: %d\n", max_score);
    printf("Performance Metrics:\n");
    printf("Total time: %d cycles\n", total_time);
    printf("Computation time: %d cycles\n", compute_time);
    printf("Synchronization time: %d cycles\n", sync_time);
    printf("Communication time: %d cycles\n", communication_time);

    printf("\nCommunication Statistics:\n");
    printf("1. Communications inside the same tile: %d\n", total_intra_tile);
    printf("2. Communications between two tiles of the same group: %d\n", total_inter_tile_same_group);
    printf("3. Communications between two tiles in different groups: %d\n", total_inter_group);
  }

  return 0;
}
