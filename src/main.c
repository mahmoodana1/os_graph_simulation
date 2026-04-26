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

    // mohammed TODO:
    Graph* g = loadGraph(argv[1], &start_node, &end_node);
    if (!g) {
        return 1;
    }

    // Loai TODO: Solve and print to terminal (Milestone 1)
    // This handles the 0->2->1 format and weight output
    solveDijkstra(g, start_node, end_node);

    // 4. Ahmed TODO: Launch the GUI (Milestone 2)
    // The window stays open until the user closes it
    startGui(g);

    freeAll(g);

    return 0;
}
