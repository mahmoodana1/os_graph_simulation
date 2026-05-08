#include "../../include/dijkstra.h"
#include "../../include/graph.h"
#include <stdio.h>

extern int BuildDijkstraPath(Graph* g, int start, int end, int* out_path); //

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

  int start_node, end_node;

    Graph* g = loadGraph(argv[1], &start_node, &end_node);
    PathResult result = solveDijkstra(g, start_node, end_node);
    if (!g)
    {
        return 1;
    }

    solveDijkstra(g, start_node, end_node);
    printPathResult(result); 
    freeAll(g);

  return 0;
}
