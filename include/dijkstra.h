#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

#define MAX_PATH 100

typedef struct {
    int nodes[MAX_PATH];
    int length;
    int totalWeight;
    int found;
} PathResult;

PathResult solveDijkstra(Graph *g, int start, int end);
void printPathResult(PathResult result);

#endif