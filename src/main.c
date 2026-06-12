#include "../include/dijkstra.h"
#include "../include/graph.h"
#include "../include/gui.h"
#include "../include/ipc.h"
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    TravelerList travelers;
    travelers.travelers = NULL;
    Graph *g = loadGraph(argv[1], &travelers);

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    if (!g)
        return EXIT_FAILURE;

    pid_t pids[travelers.count];

    createShm(travelers.count);
    initSemaphores(travelers_shm_ptr, travelers.count);

    for (int i = 0; i < travelers.count; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            travelers_shm_ptr[i].pid = getpid();
            PathResult result = solveDijkstra(g, travelers.travelers[i].src,
                                              travelers.travelers[i].dst);
            writeTravelerPathToSharedMemory(travelers_shm_ptr, i, result);
            exit(0);
        }

        pids[i] = pid;
    }

    RenderCtx *ctx = initGuiSetup(g, travelers.count);

    // seat cars at source nodes
    for (int i = 0; i < travelers.count; i++) {
        int src = travelers.travelers[i].src;
        if (src >= 0 && src < ctx->node_count) {
            ctx->cars[i].x = ctx->positions[src].x;
            ctx->cars[i].y = ctx->positions[src].y;
        }
    }

    while (!WindowShouldClose()) {
        readTravelerPathFromSharedMemory(ctx, travelers_shm_ptr, travelers.count);
        BeginDrawing();
        RenderFrame(ctx, g, GetFrameTime());
        EndDrawing();
    }

    for (int i = 0; i < travelers.count; i++)
        waitpid(pids[i], NULL, 0);

    CloseWindow();
    freeRenderer(ctx);
    freeAll(g);
    free(travelers.travelers);
    detachShm();

    return EXIT_SUCCESS;
}
