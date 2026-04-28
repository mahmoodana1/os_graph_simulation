#include "../include/graph.h"
#include "../include/dijkstra.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Initialize the graph and read from the input file
    int start_node, end_node;

    Graph* g = loadGraph(argv[1], &start_node, &end_node);
    if (!g) {
        return 1;
    }

    solveDijkstra(g, start_node, end_node);
    freeAll(g);

    return 0;
}
