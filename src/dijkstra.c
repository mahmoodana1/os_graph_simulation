#ifndef FILE_H
#define FILE_H

#define INF 1000000

#include <stdio.h>
#include "../include/graph.h"

static int getMinDst(int dist[], int visited[], int n) {
    int min = INF;
    int minIndex = -1;

    for (int i = 0; i < n; i++) {
        if (dist[i] < min && visited[i] == 0) {
            min = dist[i];
            minIndex = i;
        }
    }

    return minIndex;
}

static void printPath(int parent[], int v) {
    if (parent[v] == -1) {
        printf("%d", v);
        return;
    }

    printPath(parent, parent[v]);
    printf(" -> %d", v);
}

void solveDijkstra(Graph *g, int start, int end) {
    if (start == end) {
        printf("%d\n", start);
        return;
    }

    int numofVertises = g->num_nodes;
    int dst[numofVertises];
    int visited[numofVertises];
    int parent[numofVertises];

    for (int i = 0; i < numofVertises; i++) {
        dst[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    dst[start] = 0;

    for (int i = 0; i < numofVertises; i++) {
        int temp = getMinDst(dst, visited, numofVertises);

        if (temp == -1) {
            break;
        }

        visited[temp] = 1;

        Node *cuurrent = g->adj[temp];

        while (cuurrent != NULL) {
            int v = cuurrent->id;
            int w = cuurrent->weight;

            if (visited[v] == 0 && dst[temp] + w < dst[v]) {
                dst[v] = dst[temp] + w;
                parent[v] = temp;
            }

            cuurrent = cuurrent->next;
        }
    }

    if (dst[end] == INF) {
        printf("No path found\n");
        return;
    }

    printPath(parent, end);
    printf("\n");
    printf("%d\n", dst[end]);
}

#endif
