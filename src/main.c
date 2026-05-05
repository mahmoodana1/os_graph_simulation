#include "../include/graph.h"
#include "../include/dijkstra.h"
#include "../include/gui.h"
#include <stdio.h>

extern int BuildDijkstraPath(Graph* g, int start, int end, int* out_path); //

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Initialize the graph and read from the input file
    int start_node, end_node;

    Graph* g = loadGraph(argv[1], &start_node, &end_node);
    if (!g)
    {
        return 1;
    }

    solveDijkstra(g, start_node, end_node);
    //startGui(g);
    int src = 0, dst = 0;

    // /* حساب المسار الأقصر باستخدام الأسماء الجديدة */
    int path[64];
    int path_len = BuildDijkstraPath(g, start_node, end_node, path);

    InitWindow(SCREEN_W, SCREEN_H, "Graph Simulation – Milestone 3");
    SetTargetFPS(60);

    RenderCtx* ctx = InitRenderer(g, start_node, end_node, path, path_len);
    if (!ctx)
    {
        freeAll(g);
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (!RenderFrame(ctx, g, dt)) break;
    }

    FreeRenderer(ctx);

    freeAll(g);
    CloseWindow();


    return 0;
}
