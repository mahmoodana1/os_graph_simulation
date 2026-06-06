#ifndef IPC_H
#define IPC_H

#include "utils.h"
#include <semaphore.h>
#include <sys/types.h>

typedef struct {
    pid_t pid;
    int current_node;
    int next_node;
    int total_hops;
    sem_t sem_ready_to_read;
    sem_t sem_ready_to_write;
} TravelerMsg;

extern TravelerMsg *shm_ptr;
extern int shm_id;
extern pid_t main_pid;

void cleanup(int);
void detachShm();
void createShm(const int travelers_count);
void initTravelerMsg(TravelerMsg *msg, const int taraveler_count);
void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result);
void readTravelerMsgFromSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, int *current_node,
                                     int *next_node);

#endif
