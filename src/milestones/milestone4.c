#include "../../include/dijkstra.h"
#include "../../include/graph.h"
#include "../../include/gui.h"
#include <stdio.h>

extern int BuildDijkstraPath(Graph *g, int start, int end, int *out_path);

int main(int argc, char *argv[]) {
    // edit as needed for testing
    printf("in milestone 4\n");


    int path0[] = {0, 1, 2};
    int path1[] = {2, 3, 4};

    int* fake_paths[] = {path0, path1};
    int fake_lens[] = {3, 3};

    
    InitRenderer(g, 2, fake_paths, fake_lens);
}
