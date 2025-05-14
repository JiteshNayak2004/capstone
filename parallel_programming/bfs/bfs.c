// Copyright 2021 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Author: Matheus Cavalcante, ETH Zurich

#include <stdint.h>
#include <string.h>

#include "encoding.h"
#include "printf.h"
#include "runtime.h"
#include "synchronization.h"
//#include "graph512.h"
//#include "libgomp.h"
#include "bio-diseasome.h"
//#include "road-euro.h"

// Define constants for core control
#define source 0 // Starting node for BFS
#define cc 8 // Number of cores core count
#define cid cc-1 // ID of the last core
#define cc1 cc 
#define cid1 cc1-1

// Global variables for BFS
uint32_t count = 0; // To track updates in the bottom-up step
uint32_t count1 = 0; // To track active nodes in the top-down step

// Bottom-Up Step function
void BUstep(uint32_t core_id, uint32_t k) {
    uint32_t id, b1, b2, uw;
    // ver is declared in the bio-diseasome.h library
    // The unit of work (uw) would be 100 / 4 = 25.
    //For core 0 (core_id = 0), b1 would be 0 * 25 = 0 and b2 would be (0 + 1) * 25 = 25.
    //For core 1 (core_id = 1), b1 would be 1 * 25 = 25 and b2 would be (1 + 1) * 25 = 50.
    //For core 2 (core_id = 2), b1 would be 2 * 25 = 50 and b2 would be (2 + 1) * 25 = 75.
    //For core 3 (core_id = 3), b1 would be 3 * 25 = 75 and b2 would be 100 (since it is the last core, it processes up to the total number of vertices).

    uw = ver / cc; // Divide the work among cores <unit of work> 
    b1 = core_id * uw;
    if (core_id == (cc - 1))
        b2 = ver; // Last core takes remaining nodes
    else
        b2 = (core_id + 1) * uw;

    // Iterate over nodes assigned to the core
    for (uint32_t tid = b1; tid < b2; tid++) {
        if (h_cost[tid] < 0) { // If the node hasn't been visited
            // Check its neighbors
            for (uint32_t i = h_graph_nodes[tid]; i < h_graph_nodes[tid + 1]; i++) {
                id = h_graph_edges[i]; // Neighbor node ID
                if (h_graph_active[id]) { // If neighbor is active
                    h_cost[tid] = k; // Update cost
                    h_updating_graph_active[tid] = 1; // Mark as active
                    count = 1; // Indicate an update
                    break; // Move to the next node
                }
            }
        }
    }
}

// Copy and reset active nodes after Bottom-Up step
void copy_bu(uint32_t core_id) {
    uint32_t b1, b2, uw;
    uw = ver / cc; // Divide the work among cores
    b1 = core_id * uw;
    if (core_id == (cc - 1))
        b2 = ver; // Last core takes remaining nodes
    else
        b2 = (core_id + 1) * uw;

    // Copy and reset active nodes
    for (uint32_t tid = b1; tid < b2; tid++) {
        h_graph_active[tid] = h_updating_graph_active[tid];
        h_updating_graph_active[tid] = 0;
    }
}

// Top-Down Step function
void TDstep(uint32_t core_id, uint32_t k) {
    uint32_t id, tid, b1, b2, uw;

    if (count1 < cc1) {
        // Each core processes one active node if count1 < cc1
        if (core_id < count1) {
            b1 = core_id;
            b2 = core_id + 1;
        }
    } else {
        // Divide the work among cores
        uw = count1 / cc1;
        b1 = core_id * uw;
        if (core_id == cid1)
            b2 = count1; // Last core takes remaining nodes
        else
            b2 = (core_id + 1) * uw;
    }
    mempool_barrier(cc);

    // Iterate over active nodes assigned to the core
    for (uint32_t j = b1; j < b2; j++) {
        tid = h_graph_active[j];
        // Check its neighbors
        for (uint32_t i = h_graph_nodes[tid]; i < h_graph_nodes[tid + 1]; i++) {
            id = h_graph_edges[i]; // Neighbor node ID
            if (h_cost[id] < 0) { // If neighbor hasn't been visited
                h_cost[id] = k; // Update cost
                // Critical section to update active nodes safely
                #pragma omp critical
                {
                    h_updating_graph_active[count] = id;
                    count++;
                }
            }
        }
    }
}

// Copy and reset active nodes after Top-Down step
void copy_td(uint32_t core_id) {
    uint32_t b1, b2, uw;

    if (count < cc1) {
        // Each core processes one active node if count < cc1
        if (core_id < count) {
            b1 = core_id;
            b2 = core_id + 1;
            for (uint32_t c = b1; c < b2; c++) {
                h_graph_active[c] = h_updating_graph_active[c];
            }
        }
    } else {
        // Divide the work among cores
        uw = count / cc1;
        b1 = core_id * uw;
        if (core_id == cid1)
            b2 = count; // Last core takes remaining nodes
        else
            b2 = (core_id + 1) * uw;
        // Copy active nodes
        for (uint32_t c = b1; c < b2; c++) {
            h_graph_active[c] = h_updating_graph_active[c];
        }
    }
}

// Reset active nodes
void reset_h_graph_active(uint32_t core_id) {
    uint32_t b1, b2, uw;
    uw = ver / cc; // Divide the work among cores
    b1 = core_id * uw;
    if (core_id == cid)
        b2 = ver; // Last core takes remaining nodes
    else
        b2 = (core_id + 1) * uw;

    // Reset active nodes
    for (uint32_t c = b1; c < b2; c++) {
        h_graph_active[c] = 0;
    }
}

// Reset updating active nodes
void reset_h_updating_graph_active(uint32_t core_id) {
    uint32_t b1, b2, uw;
    uw = ver / cc; // Divide the work among cores
    b1 = core_id * uw;
    if (core_id == cid)
        b2 = ver; // Last core takes remaining nodes
    else
        b2 = (core_id + 1) * uw;

    // Reset updating active nodes
    for (uint32_t c = b1; c < b2; c++) {
        h_updating_graph_active[c] = 0;
    }
}

// Convert updating active nodes to active nodes
void conver() {
    uint32_t inter;
    for (uint32_t c = 0; c < count1; c++) {
        inter = h_updating_graph_active[c];
        h_graph_active[inter] = 1;
    }
}

// Main function
int main() {
    uint32_t core_id = mempool_get_core_id(); // Get core ID
    uint32_t num_cores = mempool_get_core_count(); // Get number of cores
    mempool_barrier_init(core_id); // Initialize barrier synchronization

    // Initialize graph data by specific cores
    if (core_id == 12) {
        for (uint32_t i = 0; i <= ver; i++) {
            h_graph_nodes[i] = h_graph_nodesdup[i];
        }
    }

    if (core_id == 4) {
        for (uint32_t i = 0; i < edg; i++) {
            h_graph_edges[i] = h_graph_edgesdup[i];
        }
    }

    if (core_id == 0) {
        for (uint32_t i = 0; i < ver; i++) {
            h_cost[i] = -1; // Initialize costs
        }
    }

    uint32_t k = 1; // BFS level
    uint32_t swi = 1; // Switch flag

    mempool_barrier(num_cores); // Synchronize cores

    // Initialize BFS starting node
    if (core_id == 0) {
        count1 = 1;
        h_graph_active[0] = source;
        h_cost[source] = 0;
        swi = 0;
    }

    mempool_barrier(cc); // Synchronize cores

    // Start timer
    mempool_timer_t cycles = mempool_get_timer();

    // BFS algorithm loop
    do {
        mempool_barrier(cc); // Synchronize cores

        if (k < 3) {
            TDstep(core_id, k); // Top-Down step
            mempool_barrier(cc); // Synchronize cores
            copy_td(core_id); // Copy and reset active nodes
        }

        if (k == 3) {
            reset_h_graph_active(core_id); // Reset active nodes
            mempool_barrier(cc); // Synchronize cores
            if (core_id == 0)
                conver(); // Convert active nodes
            mempool_barrier(cc); // Synchronize cores
            reset_h_updating_graph_active(core_id); // Reset updating active nodes
        }

        if (k >= 3) {
            mempool_barrier(cc); // Synchronize cores
            BUstep(core_id, k); // Bottom-Up step
            pool_barrier(cc); // Synchronize cores
            copy_bu(core_id); // Copy and reset active nodes
        }

        mempool_barrier(cc); // Synchronize cores

        // Update switch flag based on active nodes
        if (core_id == 0) {
            swi = 0;
            for (uint32_t tid = 0; tid < ver; tid++) {
                if (h_graph_active[tid] == 1) {
                    swi = 1;
                    break;
                }
            }
        }

        mempool_barrier(cc); // Synchronize cores

        k++; // Increment BFS level

    } while (swi); // Continue while there are active nodes

    // Stop timer
    cycles = mempool_get_timer() - cycles;

    // Print results from core 0
    if (core_id == 0) {
        printf("Iter, %d, cycles %d\n", k, cycles);
    }

    mempool_barrier(num_cores); // Synchronize cores

    return 0; // Exit program
}

