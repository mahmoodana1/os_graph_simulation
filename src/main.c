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

    signal(SIGINT, cleanup); // to cleanup after closing the window
    signal(SIGTERM, cleanup);

    TravelerList travelers;
    Graph *g = loadGraph(argv[1], &travelers);

    if (!g) {
        return EXIT_FAILURE;
    }

    pid_t pids[travelers.count];
    int gui_paths[travelers.count][64];
    int paths[travelers.count];

    createShm(travelers.count);
    initTravelerMsg(shm_ptr, travelers.count);

    // calculate path for each traveler
    for (int i = 0; i < travelers.count; i++) {
        paths[i] = BuildDijkstraPath(g, travelers.travelers[i].src,
                                     travelers.travelers[i].dst, gui_paths[i]);

        pid_t pid = fork();

        if (pid == 0) {
            shm_ptr[i].pid = getpid();
            PathResult result = solveDijkstra(g, travelers.travelers[i].src,
                                              travelers.travelers[i].dst);

            writeTravelerPathToSharedMemory(shm_ptr, i, result);

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

    for (int i = 0; i < travelers.count; i++) {
        waitpid(pids[i], NULL, 0);
    }

    cleanup(0);

    return EXIT_SUCCESS;
}
