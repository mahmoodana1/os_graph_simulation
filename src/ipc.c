#include "../include/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_TRAVELERS 5

void cleanup(int sig) {
  char *shm_ptr;
  int shm_id;
  shmdt(shm_ptr);
  shmctl(shm_id, IPC_RMID, NULL);
  exit(0);
}

char *createShm() {
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

  char *shm_ptr = (char *)shmat(shm_id, NULL, 0);
  if (shm_ptr == (void *)-1) {
    perror("shmat failed");
    exit(EXIT_FAILURE);
  }

  return shm_ptr;
}
