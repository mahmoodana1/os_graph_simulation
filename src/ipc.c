#include "../include/ipc.h"
#include <sys/shm.h>

void cleanup(int sig) {
  char *shm_ptr;
  int shm_id;
  shmdt(shm_ptr);
  shmctl(shm_id, IPC_RMID, NULL);
  exit(0);
}
