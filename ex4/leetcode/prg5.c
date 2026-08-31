#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Structure to store graph edges
typedef struct {
    int to;
    int cost; // 1 if original edge (needs reversal), 0 if artificial reverse edge
} Edge;

int minReorder(int n, int** connections, int connectionsSize, int* connectionsColSize) {
    // 1. Count degrees to allocate exact space for the adjacency list
    int* degree = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < connectionsSize; i++) {
        degree[connections[i][0]]++;
        degree[connections[i][1]]++;
    }

    // 2. Build the Adjacency List
    Edge** graph = (Edge**)malloc(n * sizeof(Edge*));
    for (int i = 0; i < n; i++) {
        graph[i] = (Edge*)malloc(degree[i] * sizeof(Edge));
    }

    // Reuse the degree array as a pointer offset index map during insertion
    int* head = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < connectionsSize; i++) {
        int u = connections[i][0];
        int v = connections[i][1];

        // Original edge u -> v (cost = 1 because it moves away from 0)
        graph[u][head[u]].to = v;
        graph[u][head[u]].cost = 1;
        head[u]++;

        // Virtual reverse edge v -> u (cost = 0 because it moves toward 0)
        graph[v][head[v]].to = u;
        graph[v][head[v]].cost = 0;
        head[v]++;
    }

    // 3. BFS Traversal setup
    int* queue = (int*)malloc(n * sizeof(int));
    bool* visited = (bool*)calloc(n, sizeof(bool));
    int front = 0, rear = 0;
    int totalReversals = 0;

    // Start BFS from City 0
    queue[rear++] = 0;
    visited[0] = true;

    while (front < rear) {
        int curr = queue[front++];

        for (int i = 0; i < degree[curr]; i++) {
            Edge neighbor = graph[curr][i];
            if (!visited[neighbor.to]) {
                visited[neighbor.to] = true;
                totalReversals += neighbor.cost; // Add 1 if it is an outgoing original edge
                queue[rear++] = neighbor.to;
            }
        }
    }

    // Free dynamically allocated memory
    for (int i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);
    free(degree);
    free(head);
    free(queue);
    free(visited);

    return totalReversals;
}
