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
    TravelerMsg *shared_mem = (TravelerMsg *)shm_ptr;
    initTravelerMsg(shared_mem, travelers.count);

    // calculate path for each traveler
    for (int i = 0; i < travelers.count; i++) {
        paths[i] = BuildDijkstraPath(g, travelers.travelers[i].src,
                                     travelers.travelers[i].dst, gui_paths[i]);

        pid_t pid = fork();

        if (pid == 0) {
            PathResult result = solveDijkstra(g, travelers.travelers[i].src,
                                              travelers.travelers[i].dst);

            writeTravelerPathToSharedMemory(shared_mem, i, result);

            exit(0);
        }

        pids[i] = pid;
    }

    startGui(g, gui_paths, paths, travelers.count);

    for (int i = 0; i < travelers.count; i++) {
        waitpid(pids[i], NULL, 0);
    }

    cleanup(0);

    return EXIT_SUCCESS;
}
