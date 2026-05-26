#include "../include/dijkstra.h"
#include "../include/graph.h"
#include "../include/gui.h"
#include "../include/ipc.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_TRAVELERS 5

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <input_file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  TravelerList travelers;
  Graph *g = loadGraph(argv[1], &travelers);

  if (!g) {
    return EXIT_FAILURE;
  }
  pid_t pids[travelers.count];
  int gui_paths[travelers.count][64];
  int paths[travelers.count];

  key_t key = ftok("/tmp", 'y');
  if (key == -1) {
    perror("ftok failed");
    exit(EXIT_FAILURE);
  }

  size_t SHM_SIZE = sizeof(TravelerMsg) * MAX_TRAVELERS;

  int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
  if (shm_id == -1) {
    perror("shmget failed");
    exit(EXIT_FAILURE);
  }

  char *shm_ptr;
  shm_ptr = (char *)shmat(shm_id, NULL, 0);
  if (shm_ptr == (void *)-1) {
    perror("shmat failed");
    exit(EXIT_FAILURE);
  }

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

  return EXIT_SUCCESS;
}
