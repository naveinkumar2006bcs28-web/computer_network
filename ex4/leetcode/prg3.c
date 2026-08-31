#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

int networkDelayTime(int** times, int timesSize, int* timesColSize, int n, int k) {
    // Allocate adjacency matrix (using 1-based indexing for nodes)
    int** g = (int**)malloc((n + 1) * sizeof(int*));
    int* dist = (int*)malloc((n + 1) * sizeof(int));
    bool* vis = (bool*)malloc((n + 1) * sizeof(bool));

    int inf = 1 << 29;

    for (int i = 0; i <= n; i++) {
        g[i] = (int*)malloc((n + 1) * sizeof(int));
        for (int j = 0; j <= n; j++) {
            g[i][j] = inf;
        }
        dist[i] = inf;
        vis[i] = false;
    }

    // Build the graph
    for (int i = 0; i < timesSize; i++) {
        int u = times[i][0];
        int v = times[i][1];
        int w = times[i][2];
        g[u][v] = w;
    }

    dist[k] = 0;

    for (int i = 0; i < n; i++) {
        int u = -1;
        // Find the unvisited node with the smallest distance
        for (int j = 1; j <= n; j++) {
            if (!vis[j] && (u == -1 || dist[j] < dist[u])) {
                u = j;
            }
        }

        if (u == -1 || dist[u] == inf) {
            break;
        }

        vis[u] = true;

        // Update distances to neighbors
        for (int v = 1; v <= n; v++) {
            if (g[u][v] != inf && dist[u] + g[u][v] < dist[v]) {
                dist[v] = dist[u] + g[u][v];
            }
        }
    }

    int max_dist = 0;
    for (int i = 1; i <= n; i++) {
        if (dist[i] == inf) {
            // Free allocated memory
            for (int j = 0; j <= n; j++) free(g[j]);
            free(g); free(dist); free(vis);
            return -1;
        }
        if (dist[i] > max_dist) {
            max_dist = dist[i];
        }
    }

    // Free allocated memory
    for (int j = 0; j <= n; j++) free(g[j]);
    free(g); free(dist); free(vis);

    return max_dist;
}
