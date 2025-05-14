#include<stdio.h>
#include <stdint.h>
#include <string.h>
#include "data.h"
#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"
#include<stdbool.h>
#define cc 32
#define cid cc-1
//#define MAX_DEGREE 1000
//uint32_t core_id, uint32_t bound_a, uint32_t bound_b


void OrderedCount(uint32_t core_id, uint32_t bound_a, uint32_t bound_b) {
    uint32_t num_triangles = 0;
    for (uint32_t u = bound_a; u < bound_b; u++) {
        for (uint32_t i = h_graph_nodes[u]; i < h_graph_nodes[u + 1]; i++) {
            uint32_t v = h_graph_edges[i]; // adjacent node to u
            if (v > u) {
                for (uint32_t j = h_graph_nodes[v]; j < h_graph_nodes[v + 1]; j++) {
                    uint32_t w = h_graph_edges[j]; // adjacent node to v
                    if (w > v) {
                        for (uint32_t k = h_graph_nodes[w]; k < h_graph_nodes[w + 1]; k++) {
                            if (h_graph_edges[k] == u) {
                                num_triangles++;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
   // printf("The number of triangles is : %d\n", num_triangles);
}



                
             


      
   



uint32_t main(){
uint32_t core_id = mempool_get_core_id();
uint32_t num_cores = mempool_get_core_count();
// Initialize synchronization variables
mempool_barrier_init(core_id);


if (core_id >=cc) {
   while(1);}
   
   
uint32_t uw,bound_a,bound_b;



uw=edg/cc;
bound_a=core_id*uw;
if(core_id==(cc-1))
bound_b=edg;
else
bound_b=(core_id+1)*uw;

for(uint32_t i=bound_a;i<bound_b;i++)
{
h_graph_edges[i]=h_graph_edgesdup[i];
}


/*
for(uint32_t i=0;i<edg;i++)
{
h_graph_edges[i]=h_graph_edgesdup[i];
}*/
mempool_barrier(cc);


if(core_id==0){
bound_a=0;
bound_b=28;
};
if(core_id==1){
bound_a=28;
bound_b=54;
};
if(core_id==2){
bound_a=54;
bound_b=84;
};
if(core_id==3){
bound_a=84;
bound_b=113;
};
if(core_id==4){
bound_a=113;
bound_b=143;
};
if(core_id==5){
bound_a=143;
bound_b=177;
};
if(core_id==6){
bound_a=177;
bound_b=203;
};
if(core_id==7){
bound_a=203;
bound_b=235;
};
if(core_id==8){
bound_a=235;
bound_b=266;
};
if(core_id==9){
bound_a=266;
bound_b=297;
};
if(core_id==10){
bound_a=297;
bound_b=334;
};
if(core_id==11){
bound_a=334;
bound_b=369;
};
if(core_id==12){
bound_a=369;
bound_b=404;
};
if(core_id==13){
bound_a=404;
bound_b=434;
};
if(core_id==14){
bound_a=434;
bound_b=465;
};
if(core_id==15){
bound_a=465;
bound_b=498;
};
if(core_id==16){
bound_a=498;
bound_b=532;
};
if(core_id==17){
bound_a=532;
bound_b=562;
};
if(core_id==18){
bound_a=562;
bound_b=600;
};
if(core_id==19){
bound_a=600;
bound_b=642;
};
if(core_id==20){
bound_a=642;
bound_b=685;
};
if(core_id==21){
bound_a=685;
bound_b=718;
};
if(core_id==22){
bound_a=718;
bound_b=755;
};
if(core_id==23){
bound_a=755;
bound_b=797;
};
if(core_id==24){
bound_a=797;
bound_b=843;
};
if(core_id==25){
bound_a=843;
bound_b=883;
};
if(core_id==26){
bound_a=883;
bound_b=923;
};
if(core_id==27){
bound_a=923;
bound_b=973;
};
if(core_id==28){
bound_a=973;
bound_b=1021;
};
if(core_id==29){
bound_a=1021;
bound_b=1078;
};
if(core_id==30){
bound_a=1078;
bound_b=1128;
};
if(core_id==31){
bound_a=1128;
bound_b=1174;
};







/*
 for(uint32_t i=0;i<ver;i++){
 h_graph_nodes[i]=h_graph_nodesdup[i];
 }*/
 
 
 
 for(uint32_t i=bound_a;i<bound_b;i++){
 h_graph_nodes[i]=h_graph_nodesdup[i];
 }
 
 
mempool_barrier(cc);


if(core_id==0)
  printf("hey!\n");
mempool_barrier(cc);
/*
if(core_id==1)
printf("now!\n");
mempool_barrier(cc);*/

mempool_timer_t cycles = mempool_get_timer();
OrderedCount(core_id, bound_a,bound_b);
//OrderedCount(core_id);
mempool_barrier(cc);
cycles = mempool_get_timer() - cycles;

if (core_id==0)
printf("The number of cycles is %d\n",cycles);
mempool_barrier(cc);
/*
if (core_id==1)
printf("The number of cycles is %d\n",cycles);
mempool_barrier(cc);

if (core_id==2)
printf("The number of cycles is %d\n",cycles);
mempool_barrier(cc);

if (core_id==3)
printf("The number of cycles is %d\n",cycles);
mempool_barrier(cc);
*/
/*printf("The number of triangles is: %d\n", CountTriangles);
mempool_barrier(cc);*/

return 0;
}

