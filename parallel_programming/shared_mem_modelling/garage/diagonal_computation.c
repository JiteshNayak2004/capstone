#include <stdio.h>

void compute_diagonal_segment(int diagonal_index,int start,int end) {

    int count=0;
    for(int i=diagonal_index;i>=0;i--) {
	for(int j=0;j<=diagonal_index;j++) {
	    if(((i+j)==diagonal_index)&&(i<=m-1)&&(j<=n-1)){
		if((count>=start)&&(count<end)) {
		    int match = get_dp_value(i-1, j-1) + score(seq1[i-1], seq2[j-1]);
		    int delete = get_dp_value(i-1, j) + GAP_SCORE;
		    int insert = get_dp_value(i, j-1) + GAP_SCORE;

		    int max_score = match > delete ? (match > insert ? match : insert) : (delete > insert ? delete : insert);
		    max_score = max_score > 0 ? max_score : 0; // Ensure non-negative scores

		    set_dp_value(i, j, max_score);

		    //compute logic
		}
		else if(count==end)
		    return;
		    else 
		count=count+1;
	    }
	}
    }
}
int main() {

    return 0;
}

