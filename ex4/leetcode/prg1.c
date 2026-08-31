#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_STOP 1000000

// Structure to hold a dynamic list of buses for each stop
typedef struct {
    int *buses;
    int size;
    int capacity;
} StopToBuses;

int numBusesToDestination(int** routes, int routesSize, int* routesColSize, int source, int target) {
    // Edge case: already at destination
    if (source == target) return 0;

    // Find the maximum stop index present in the routes to allocate exact memory
    int max_stop_val = 0;
    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            if (routes[i][j] > max_stop_val) {
                max_stop_val = routes[i][j];
            }
        }
    }
    
    // If source or target is out of bounds of the actual stops available
    if (source > max_stop_val || target > max_stop_val) return -1;

    // Create the inverted index: Stop -> List of Bus Routes
    StopToBuses* stopGraph = (StopToBuses*)calloc(max_stop_val + 1, sizeof(StopToBuses));
    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            int stop = routes[i][j];
            if (stopGraph[stop].size == stopGraph[stop].capacity) {
                stopGraph[stop].capacity = stopGraph[stop].capacity == 0 ? 4 : stopGraph[stop].capacity * 2;
                stopGraph[stop].buses = (int*)realloc(stopGraph[stop].buses, stopGraph[stop].capacity * sizeof(int));
            }
            stopGraph[stop].buses[stopGraph[stop].size++] = i;
        }
    }

    // BFS setup
    int* queue = (int*)malloc(routesSize * sizeof(int));
    bool* visitedBuses = (bool*)calloc(routesSize, sizeof(bool));
    bool* visitedStops = (bool*)calloc(max_stop_val + 1, sizeof(bool));
    
    int head = 0, tail = 0;
    int busCount = 0;

    // Initialize BFS with all buses that pass through the source stop
    visitedStops[source] = true;
    for (int i = 0; i < stopGraph[source].size; i++) {
        int bus = stopGraph[source].buses[i];
        queue[tail++] = bus;
        visitedBuses[bus] = true;
    }

    // Process BFS layer by layer
    while (head < tail) {
        int levelSize = tail - head;
        busCount++; // Increment bus count for this connection layer

        for (int i = 0; i < levelSize; i++) {
            int currentBus = queue[head++];

            // Check all stops this bus can reach
            for (int j = 0; j < routesColSize[currentBus]; j++) {
                int stop = routes[currentBus][j];

                if (stop == target) {
                    // Free allocated memory before returning
                    for (int k = 0; k <= max_stop_val; k++) {
                        if (stopGraph[k].buses) free(stopGraph[k].buses);
                    }
                    free(stopGraph);
                    free(queue);
                    free(visitedBuses);
                    free(visitedStops);
                    return busCount;
                }

                if (visitedStops[stop]) continue;
                visitedStops[stop] = true;

                // Enqueue all unvisited buses that pass through this stop
                for (int k = 0; k < stopGraph[stop].size; k++) {
                    int nextBus = stopGraph[stop].buses[k];
                    if (!visitedBuses[nextBus]) {
                        visitedBuses[nextBus] = true;
                        queue[tail++] = nextBus;
                    }
                }
            }
        }
    }

    // Free memory if target is unreachable
    for (int k = 0; k <= max_stop_val; k++) {
        if (stopGraph[k].buses) free(stopGraph[k].buses);
    }
    free(stopGraph);
    free(queue);
    free(visitedBuses);
    free(visitedStops);

    return -1;
}
