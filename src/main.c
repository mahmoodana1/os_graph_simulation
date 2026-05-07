#include "../include/graph.h"
#include "../include/dijkstra.h"
#include "../include/gui.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    int start_node, end_node;

    Graph* g = loadGraph(argv[1], &start_node, &end_node);
    if (!g) {
        return 1;
    }

    PathResult result = solveDijkstra(g, start_node, end_node);

    printPathResult(result);   // terminal

    startGui(g);

    freeAll(g);

    return 0;
}