#include "../include/ipc.h"
#include "../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define MAX_TRAVELERS 8

TravelerMsg *shm_ptr;
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

void createShm(const int travelers_count) {
    main_pid = getpid();

    key_t key = ftok("/tmp", 'y');
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    size_t SHM_SIZE = sizeof(TravelerMsg) * travelers_count;

    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shm_ptr = (TravelerMsg *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
}

void initTravelerMsg(TravelerMsg *shared_mem, const int travelers_count) {
    for (int i = 0; i < travelers_count; i++) {
        shared_mem[i].pid = -1;
        shared_mem[i].current_node = -1;
        shared_mem[i].next_node = -1;

        // Initial value = 0 (Red Light). The mailbox is currently empty.
        sem_init(&shared_mem[i].sem_ready_to_read, 1, 0);
        // Initial value = 1 (Green Light). There is 1 available empty slot.
        sem_init(&shared_mem[i].sem_ready_to_write, 1, 1);
    }
}

void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result) {
    for (int j = 0; j < result.length; j++) {
        sem_wait(&shared_mem[traveler_index].sem_ready_to_write);
        shared_mem[traveler_index].pid = getpid();
        shared_mem[traveler_index].current_node = result.nodes[j];

        if (j + 1 < result.length) {
            shared_mem[traveler_index].next_node = result.nodes[j + 1];
        } else {
            shared_mem[traveler_index].next_node = -1;
        }
        sem_post(&shared_mem[traveler_index].sem_ready_to_read);
    }
}

void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem,
                                      int count) {
    for (int i = 0; i < count; i++) {

        if (ctx->cars[i].state == CAR_IDLE ||
            ctx->cars[i].state == CAR_NODE_WAIT) {
            if (sem_trywait(&shared_mem[i].sem_ready_to_read) == 0) {

                int pid = shared_mem[i].pid;
                int curr = shared_mem[i].current_node;
                int next = shared_mem[i].next_node;

                if (next == -1) {
                    printf("[PID=%d] arrived at node %d | DESTINATION\n", pid,
                           curr);
                    printf("[PID=%d] finished\n", pid);
                } else {
                    printf("[PID=%d] arrived at node %d | next node: %d\n", pid,
                           curr, next);
                }

                fflush(stdout);
                ApplyTravelerUpdate(ctx, i, curr, next);
                sem_post(&shared_mem[i].sem_ready_to_write);
            }
        }
    }
}
