#ifndef GRAPH_H
#define GRAPH_H

#define MAX_NODES 15

// node for adjacency list
typedef struct Node {
    int id;
    int weight;
    struct Node* next;
} Node;

typedef struct {
    Node* adj[MAX_NODES];
    int num_nodes;
} Graph;

// load data from file and create graph
Graph* load_graph(const char* file, int* start, int* end);
void free_all(Graph* g);

#endif
