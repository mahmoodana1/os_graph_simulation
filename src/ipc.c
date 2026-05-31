#include "../include/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#define MAX_TRAVELERS 8



char *shm_ptr;
int shm_id;
pid_t main_pid;

void cleanup(int sig) {
    shmdt(shm_ptr);

    if (getpid() == main_pid) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("IPC_RMID failed");
            exit(EXIT_FAILURE);
        }
    }

    exit(0);
}

void createShm() {
    main_pid = getpid();

    key_t key = ftok("/tmp", 'y');
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    size_t SHM_SIZE = sizeof(TravelerMsg) * MAX_TRAVELERS;

    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
}
void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index,
                                     PathResult result) {
    for (int j = 0; j < result.length; j++) {
        shared_mem[traveler_index].pid = getpid();
        shared_mem[traveler_index].current_node = result.nodes[j];

        if (j + 1 < result.length) {
            shared_mem[traveler_index].next_node = result.nodes[j + 1];
        } else {
            shared_mem[traveler_index].next_node = -1;
        }

        shared_mem[traveler_index].ready = 1;

        usleep(300000);
    }
}
