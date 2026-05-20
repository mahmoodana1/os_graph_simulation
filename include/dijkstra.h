#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include "utils.h"

PathResult solveDijkstra(Graph *g, int start, int end);
int BuildDijkstraPath(Graph *g, int start, int end, int *out_path);

#endif
