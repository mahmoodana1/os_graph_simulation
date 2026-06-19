#ifndef GRAPH_H
#define GRAPH_H

#define MAX_NODES 15
#include "utils.h"

struct Car;
// node for adjacency list
typedef struct Node {
  int id;
  int weight;
  struct Node *next;
} Node;

typedef struct {
  Node *adj[MAX_NODES];
  int num_nodes;
} Graph;

// load data from file and create graph
Graph *createGraph(int numNodes);
void addEdge(Graph *graph, int src, int dst, int weight);
Graph *loadGraph(const char *file, TravelerList *travelers);
int path_remaining_cost(Car *car, Graph *g);
void freeAll(Graph *g);
#endif
