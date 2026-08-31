#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// Structure for Queue elements in BFS
typedef struct {
    int r;
    int c;
} Point;

// Helper function to check if a path exists with a maximum allowed 'effort'
bool canReachDestination(int** heights, int rows, int cols, int maxEffort) {
    // Directions for moving Up, Down, Left, Right
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    
    // Visited array tracker
    bool** visited = (bool**)malloc(rows * sizeof(bool*));
    for (int i = 0; i < rows; i++) {
        visited[i] = (bool*)calloc(cols, sizeof(bool));
    }
    
    // Queue initialization
    Point* queue = (Point*)malloc(rows * cols * sizeof(Point));
    int head = 0, tail = 0;
    
    // Start from the top-left cell
    queue[tail++] = (Point){0, 0};
    visited[0][0] = true;
    
    bool reached = false;
    
    while (head < tail) {
        Point curr = queue[head++];
        
        // If we reached the bottom-right cell
        if (curr.r == rows - 1 && curr.c == cols - 1) {
            reached = true;
            break;
        }
        
        // Explore 4-directional neighbors
        for (int i = 0; i < 4; i++) {
            int nr = curr.r + dr[i];
            int nc = curr.c + dc[i];
            
            // Check boundary conditions and if already visited
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !visited[nr][nc]) {
                // Check if the edge effort is within the current allowed maxEffort
                int currentEffort = abs(heights[curr.r][curr.c] - heights[nr][nc]);
                if (currentEffort <= maxEffort) {
                    visited[nr][nc] = true;
                    queue[tail++] = (Point){nr, nc};
                }
            }
        }
    }
    
    // Free allocated memory
    for (int i = 0; i < rows; i++) {
        free(visited[i]);
    }
    free(visited);
    free(queue);
    
    return reached;
}

int minimumEffortPath(int** heights, int heightsSize, int* heightsColSize) {
    int rows = heightsSize;
    int cols = heightsColSize[0];
    
    // The minimum possible effort is 0, maximum possible height difference is 10^6
    int low = 0;
    int high = 1000000; 
    int ans = high;
    
    // Binary search for the minimum viable maxEffort
    while (low <= high) {
        int mid = low + (high - low) / 2;
        
        if (canReachDestination(heights, rows, cols, mid)) {
            ans = mid;        // Record valid effort
            high = mid - 1;   // Try to find a smaller valid effort
        } else {
            low = mid + 1;    // Increase permitted effort limit
        }
    }
    
    return ans;
}
