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

  createShm();
  TravelerMsg *shared_mem = (TravelerMsg *)shm_ptr;
  // calculate path for each traveler
  for (int i = 0; i < travelers.count; i++) {
    paths[i] = BuildDijkstraPath(g, travelers.travelers[i].src,
                                 travelers.travelers[i].dst, gui_paths[i]);

    pid_t pid = fork();

    if (pid == 0) {
      PathResult result = solveDijkstra(
     g,
     travelers.travelers[i].src,
     travelers.travelers[i].dst
 );

      for (int j = 0; j < result.length; j++) {
        shared_mem[i].pid = getpid();
        shared_mem[i].current_node = result.nodes[j];

        if (j + 1 < result.length) {
          shared_mem[i].next_node = result.nodes[j + 1];
        } else {
          shared_mem[i].next_node = -1;
        }

        shared_mem[i].ready = 1;

        usleep(300000);
      }

      exit(0);
    }
    pids[i] = pid;
  }

  startGui(g, gui_paths, paths, travelers.count);
  // cleanup after GUI closes incase some processes are still alive
  for (int i = 0; i < travelers.count; i++) {
    waitpid(pids[i], NULL, 0);
  }

  cleanup(0);

  return EXIT_SUCCESS;
}
