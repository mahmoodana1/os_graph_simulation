#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

// calculate shortest path using Dijkstra's algorithm
void solveDijkstra(Graph* g, int start, int end);
static int getMinDist(int dist[], int visited[], int n);
static void printPath(int parent[], int v);
#endif
