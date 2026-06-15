#ifndef IPC_H
#define IPC_H

#include "utils.h"
#include <semaphore.h>
#include <sys/types.h>

#define MAX_TRAVELERS 8

typedef struct {
    pid_t pid;
    int current_node;
    int next_node;
    int total_hops;
    volatile int queued_at_node; // its set to target node while queued, and set
                                 // // to -1 when not queued
    sem_t sem_ready_to_read;
    sem_t sem_ready_to_write;
} TravelerMsg;

extern TravelerMsg *travelers_shm_ptr;
extern int shm_id; // shared memory ID for cleanup
extern pid_t main_pid;
extern sem_t *node_locks; // points to sem_t[MAX_NODES] at start of SHM
extern void *raw_shm;

void cleanup(int);
void detachShm();
void createShm(const int travelers_count);
void initSemaphores(TravelerMsg *msg, const int taraveler_count);
void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result);
void readTravelerMsgFromSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, int *current_node,
                                     int *next_node);

#endif
