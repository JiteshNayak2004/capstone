#include <stdint.h>
#include "encoding.h"
#include <stdio.h>
#include <stdbool.h>
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"
#include <string.h>

// DEFENITIONS

#define MAX_SEQ_LENGTH 50
#define MATCH_SCORE 1
#define MISMATCH_SCORE -2
#define GAP_SCORE -1

// CORE_COUNT
#define cc 4

// TIMERS
int synchronization_time=0;
int compute_time=0;


// compute does not include indices calculation duration
uint32_t temp_compute[cc]={0};

// SEQUENCE DATA
const char seq1[]="ACGT";
const char seq2[]="TGCA";
int seq1_len=strlen(seq1);
int seq2_len=strlen(seq2);

// DP matrix of size m*n not (m+1)(n+1)

int16_t dp[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH]__attribute__((section(".l1")));
// Function to compute score
int16_t score(char a, char b) {
  if (a == b) return MATCH_SCORE;
  return MISMATCH_SCORE;
}

// function to find min
int16_t minimum(int16_t x, int16_t y)
{
  return (x < y) ? x : y;
}
// function to find max
int16_t maximum(int16_t x, int16_t y)
{
  return (x > y) ? x : y;
}

//computes length of diagonal given index
int diagonal_length_cal(int diagonal_index) {
  int num_diagonal_elements;
  int min_dim=minimum(seq1_len,seq2_len);
  int max_dim=maximum(seq1_len,seq2_len);
  if (diagonal_index < min_dim) {
    num_diagonal_elements = diagonal_index+1;}
  else if  (diagonal_index < max_dim){
    num_diagonal_elements= min_dim;}
  else{
    num_diagonal_elements= seq1_len + seq2_len - 1 - diagonal_index;}

  return num_diagonal_elements;
}

// Function to compute a segment of a diagonal
void compute_diagonal_segment(int diagonal_index,int start,int end) {

  int count=0;
  for(int i=diagonal_index;i>=0;i--) {
    for(int j=0;j<=diagonal_index;j++) {
      if(((i+j)==diagonal_index)&&(i<=seq1_len-1)&&(j<=seq2_len-1)){
        //printf("element [%d][%d] is valid\n",i,j);

        if((count>=start)&&(count<end)) {
          //compute logic

          //printf("start is %d and end is %d\n",start,end);
          if((i==0)&&(j==0)){
            int16_t match  =  score(seq1[i], seq2[j]);
            int16_t delete =  GAP_SCORE;
            int16_t insert =  GAP_SCORE;

            int16_t max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
            max_score = max_score > 0 ? max_score : 0;
            //printf("the max score when i=0 and j=0 is %d \n",max_score);
            dp[i][j]=max_score;
          }
          else if(i==0) {
            int16_t match = score(seq1[i], seq2[j]);
            int16_t delete = GAP_SCORE;
            int16_t insert = dp[i][j-1] + GAP_SCORE;

            int16_t max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
            max_score = max_score > 0 ? max_score : 0; // Ensure non-negative scores
            //printf("the max score when i=0  %d \n",max_score);
            dp[i][j]=max_score;
          }
          else if(j==0){
            int16_t match = score(seq1[i], seq2[j]);
            int16_t delete = dp[i-1][j] + GAP_SCORE;
            int16_t insert = GAP_SCORE;

            int16_t max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
            max_score = max_score > 0 ? max_score : 0; // Ensure non-negative scores

            //printf("the max score when j=0  %d \n",max_score);
            dp[i][j]=max_score;
          }
          else {
            int16_t match = dp[i-1][j-1]+ score(seq1[i], seq2[j]);
            int16_t delete = dp[i-1][j] + GAP_SCORE;
            int16_t insert = dp[i][j-1] + GAP_SCORE;

            int16_t max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
            max_score = max_score > 0 ? max_score : 0; // Ensure non-negative scores

            //printf("score when i=%d j=%d is %d\n",i,j,max_score);
            dp[i][j]=max_score;
          }

        }
        else if(count==end) {
          return;}
        //incrementing counter irrespective
        count=count+1;
      }
    }
  }
}

void smith_waterman(uint32_t core_id) {
  // cuz it's m+n-1
  int total_diagonals = seq1_len + seq2_len-1;
  //printf("the total_diagonals are %d \n",total_diagonals);

  for (int diagonal_index = 0; diagonal_index < total_diagonals; diagonal_index++) {



    // Calculate the diagonal length
    int num_diagonal_elements;
    num_diagonal_elements=diagonal_length_cal(diagonal_index);

    if(core_id==0) {
      printf("the number of elements in diagonal %d are %d \n",diagonal_index,num_diagonal_elements);
    }
    int active_cores=minimum(num_diagonal_elements,cc);
    if(core_id==0) {
      printf("the active cores for diagonal %d are %d\n",diagonal_index,active_cores);
    }
    int elements_per_core = (num_diagonal_elements + active_cores - 1) / active_cores;
    if(core_id==0) {
      printf("the elements per cores for diagonal %d are %d \n",diagonal_index,elements_per_core);
    }
    if(core_id<active_cores) {
      int start = core_id * elements_per_core;
      int end = (core_id == active_cores - 1) ? num_diagonal_elements : (core_id + 1) * elements_per_core;

      uint32_t start_local_compute=mempool_get_timer();
      compute_diagonal_segment(diagonal_index, start, end);
      uint32_t end_local_compute=mempool_get_timer();
      temp_compute[core_id]+=(end_local_compute-start_local_compute);

    }

    //barrier after diagonal computation
    mempool_barrier(cc);

    uint32_t max_compute_time=temp_compute[0];
    uint32_t min_compute_time=temp_compute[0];
    if(core_id==0) {
      // max and min compute_time
      for(int i=0;i<active_cores;i++) {
        if(temp_compute[i]>max_compute_time) {
          max_compute_time=temp_compute[i];
        }
        if(temp_compute[i]<min_compute_time) {
          min_compute_time=temp_compute[i];
        }
        printf("the compute-cycles for diagonal %d core %d is %d  \n",diagonal_index,i,temp_compute[i]);
      }
      // compute-time calculation
      compute_time+=max_compute_time;
      printf("compute time for diagonal %d is %d \n",diagonal_index,max_compute_time);

      // synchronization-time calculation
      synchronization_time+=(max_compute_time-min_compute_time);
      printf("synchronization time for diagonal %d is %d \n",diagonal_index,max_compute_time-min_compute_time);

      // resetting-temp_compute
      for(int i=0;i<active_cores;i++) {
        temp_compute[i]=0;
      }

    }
    mempool_barrier(cc);
  }
}


int main() {

  uint32_t core_id = mempool_get_core_id();

  // Initialize synchronization variables
  mempool_barrier_init(core_id);

  // must manually sleep cores you don't require
  if (core_id >=cc) {
    while(1);}

  mempool_barrier(cc);
  // Initialize the array by core 0
  if (core_id == 0) {
    if(core_id!=0) {
      mempool_wfi(core_id);
    }
    printf("array has been initialized\n");
    //initializing the dp matrix
    memset(dp, 1, sizeof(dp));

    for(int i=0;i<3;i++) {
      for(int j=0;j<3;j++) {
        printf("the elem at index i %d j %d is %p \n",i,j,&dp[i][j]);
      }
    }
    printf("Sequence 1: %s (Length: %d)\n", seq1, seq1_len);
    printf("Sequence 2: %s (Length: %d)\n", seq2, seq2_len);

 }

  mempool_barrier(cc);

  // timer here computes complete kernel duration
  uint32_t start_cycles=mempool_get_timer();
  smith_waterman(core_id);
  uint32_t end_cycles=mempool_get_timer();
  uint32_t cycle_count=end_cycles-start_cycles;


  mempool_barrier(cc);
  if(core_id==0) {

    // includes the misc calcs such as start,end indices num_elem/core etc
    printf("the cycles taken for complete smith_waterman kernel is %d \n",cycle_count);
    // includes only diagonal_computation
    printf("the cycles taken for compute is %d \n",compute_time);
    // synchronization ashte
    printf("the cycles taken for synchronization is %d \n",synchronization_time);
  }

  mempool_barrier(cc);
  return 0;
}

