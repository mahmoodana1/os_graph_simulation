#include "../include/dijkstra.h"
#include "../include/graph.h"
#include "../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {

    // checking if input file was provided
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // stores all travelers from input
    TravelerList travelers;


    // load graph + traveler queries
    Graph *g = loadGraph(argv[1], &travelers);

    if (!g) {
        return EXIT_FAILURE;
    }
    PathResult paths[travelers.count];
    pid_t pids[travelers.count];
    // just for testing
    printf("travelers count: %d\n", travelers.count);
    // print traveler queries
    for (int i = 0; i < travelers.count; i++) {
        printf("traveler %d: %d -> %d\n",
               i,
               travelers.travelers[i].src,
               travelers.travelers[i].dst);
    }
    // calculate path for each traveler
    for (int i = 0; i < travelers.count; i++) {

        paths[i] = solveDijkstra(
    g,
    travelers.travelers[i].src,
    travelers.travelers[i].dst
);

        printPathResult(paths[i]);
        pid_t pid = fork();

        if (pid == 0) {
            printf("[%d] started\n", getpid());
            sleep(1);
            return EXIT_SUCCESS;
        }

        pids[i] = pid;
    }
    // check stored child pids
    for (int i = 0; i < travelers.count; i++) {
        printf("pids[%d] = %d\n", i, pids[i]);
    }
    return EXIT_SUCCESS;


}