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
    initTravelerMsg(shm_ptr, travelers.count);

    // calculate path for each traveler
    for (int i = 0; i < travelers.count; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            shm_ptr[i].pid = getpid();
            PathResult result = solveDijkstra(g, travelers.travelers[i].src,
                                              travelers.travelers[i].dst);

            writeTravelerPathToSharedMemory(shm_ptr, i, result);
            printf("[PID=%d] child exiting\n", getpid()); // <-- add this
            fflush(stdout);
            exit(0);
        }

        pids[i] = pid;
    }

    RenderCtx *ctx = initGuiSetup(g, travelers.count);

    while (!WindowShouldClose()) {
        readTravelerPathFromSharedMemory(ctx, shm_ptr, travelers.count);
        BeginDrawing();
        RenderFrame(ctx, g, GetFrameTime());
        EndDrawing();
    }

    for (int i = 0; i < travelers.count; i++)
        waitpid(pids[i], NULL, 0);

    freeRenderer(ctx);
    CloseWindow();
    freeAll(g);
    free(travelers.travelers);
    detachShm();

    return EXIT_SUCCESS;
}
