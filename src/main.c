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

  // calculate path for each traveler
  for (int i = 0; i < travelers.count; i++) {
    paths[i] = BuildDijkstraPath(g, travelers.travelers[i].src,
                                 travelers.travelers[i].dst, gui_paths[i]);

    pid_t pid = fork();

    if (pid == 0) {
      printf("[%d] started\n", getpid());
      pause();
      return EXIT_SUCCESS;
    }
    pids[i] = pid;
  }

  startGui(g, gui_paths, paths, travelers.count);
  // cleanup after GUI closes incase some processes are still alive
  for (int i = 0; i < travelers.count; i++) {
    waitpid(pids[i], NULL, 0);
  }

  if (shmdt(shm_ptr) == -1) {
    perror("shmdt failed");
    exit(EXIT_FAILURE);
  }

  if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
    perror("IPC_RMID failed");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
