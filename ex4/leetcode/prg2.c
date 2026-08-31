#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INF 0x3F3F3F3F

int findCheapestPrice(int n, int** flights, int flightsSize, int* flightsColSize, int src, int dst, int k) {
    int dist[n];
    int temp[n];
    
    memset(dist, 0x3F, sizeof(dist));
    dist[src] = 0;
    
    for (int i = 0; i <= k; i++) {
        memcpy(temp, dist, sizeof(dist));
        for (int j = 0; j < flightsSize; j++) {
            int u = flights[j][0];
            int v = flights[j][1];
            int p = flights[j][2];
            
            if (dist[u] != INF && dist[u] + p < temp[v]) {
                temp[v] = dist[u] + p;
            }
        }
        memcpy(dist, temp, sizeof(temp));
    }
    
    return dist[dst] == INF ? -1 : dist[dst];
}
