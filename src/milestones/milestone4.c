#include "../../include/dijkstra.h"
#include "../../include/graph.h"
#include "../../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_TRAVELERS 16

/* defined in graph.c — not in header, so declare here */
extern Graph* createGraph(int numNodes);
extern void   addEdge(Graph* graph, int src, int dst, int weight);

static void on_sigusr1(int sig) { (void)sig; }

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    /* --- Parse input file --- */
    FILE* f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); return 1; }

    int N, M;
    if (fscanf(f, "%d %d", &N, &M) != 2) {
        fprintf(stderr, "Error: invalid file format\n");
        fclose(f); return 1;
    }

    Graph* g = createGraph(N);
    for (int i = 0; i < M; i++) {
        int src, dst, w;
        if (fscanf(f, "%d %d %d", &src, &dst, &w) != 3) {
            fprintf(stderr, "Error: invalid edge at index %d\n", i);
            fclose(f); freeAll(g); return 1;
        }
        addEdge(g, src, dst, w);
    }

    int T;
    if (fscanf(f, "%d", &T) != 1 || T <= 0 || T > MAX_TRAVELERS) {
        fprintf(stderr, "Error: invalid traveler count\n");
        fclose(f); freeAll(g); return 1;
    }

    int srcs[MAX_TRAVELERS], dsts[MAX_TRAVELERS];
    for (int i = 0; i < T; i++) {
        if (fscanf(f, "%d %d", &srcs[i], &dsts[i]) != 2) {
            fprintf(stderr, "Error: invalid traveler pair %d\n", i);
            fclose(f); freeAll(g); return 1;
        }
    }
    fclose(f);

    /* --- Compute paths and fork one child per traveler --- */
    int paths[MAX_TRAVELERS][64];
    int path_lens[MAX_TRAVELERS];
    pid_t pids[MAX_TRAVELERS];

    for (int i = 0; i < T; i++) {
        /* Parent computes the path before forking */
        path_lens[i] = BuildDijkstraPath(g, srcs[i], dsts[i], paths[i]);
        if (path_lens[i] <= 0)
            fprintf(stderr, "Warning: no path for traveler %d (%d->%d)\n", i, srcs[i], dsts[i]);

        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); freeAll(g); return 1; }

        if (pids[i] == 0) {
            /* Child: register handler, announce start, then wait for parent signal */
            signal(SIGUSR1, on_sigusr1);
            printf("[%d] started\n", getpid());
            fflush(stdout);
            pause();
            printf("[%d] finished\n", getpid());
            fflush(stdout);
            exit(0);
        }
    }

    /* --- Parent: run the GUI with all travelers simultaneously --- */
    startGui(g, paths, path_lens, T);

    /* --- Signal every child that the simulation is done --- */
    for (int i = 0; i < T; i++)
        kill(pids[i], SIGUSR1);

    /* --- Wait for all children to exit --- */
    for (int i = 0; i < T; i++)
        waitpid(pids[i], NULL, 0);

    freeAll(g);
    return 0;
}
