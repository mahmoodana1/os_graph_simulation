#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

// calculate shortest path using Dijkstra's algorithm
void solveDijkstra(Graph* g, int start, int end);
int getMinDst(int dist[], int visited[], int n);
void printPath(int parent[], int v);
void printDijkstraResult(int parent[], int dst[], int start, int end) ;

#endif
