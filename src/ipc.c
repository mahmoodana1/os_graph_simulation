#include "../include/ipc.h"
#include "../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int shm_id = -1;
int shm_travelers = 0;
pid_t main_pid;
void *raw_shm = NULL;
sem_t *node_locks = NULL;
TravelerMsg *travelers_shm_ptr = NULL;

void detachShm() {
    if (!raw_shm || shm_id == -1)
        return;
    if (getpid() == main_pid) {
        for (int n = 0; n < MAX_NODES; n++)
            sem_destroy(&node_locks[n]);
        for (int i = 0; i < shm_travelers; i++) {
            sem_destroy(&travelers_shm_ptr[i].sem_ready_to_read);
            sem_destroy(&travelers_shm_ptr[i].sem_ready_to_write);
        }
    }

    shmdt(raw_shm);
    raw_shm = NULL;
    travelers_shm_ptr = NULL;
    node_locks = NULL;
    if (getpid() == main_pid)
        shmctl(shm_id, IPC_RMID, NULL);
}

void cleanup(int sig) {
    (void)sig;
    detachShm();
    exit(0);
}

void createShm(const int travelers_count) {
    main_pid = getpid();
    shm_travelers = travelers_count;
    size_t locks_size = sizeof(sem_t) * MAX_NODES;
    size_t SHM_SIZE = locks_size + sizeof(TravelerMsg) * travelers_count;

    key_t key = ftok("/tmp", 'y');
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id == -1) {
        int stale = shmget(key, 0, 0600);
        if (stale != -1) shmctl(stale, IPC_RMID, NULL);
        shm_id = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    }
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    raw_shm = shmat(shm_id, NULL, 0);
    if (raw_shm == (void *)-1) {
        perror("shmat failed");
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    node_locks = (sem_t *)raw_shm;
    travelers_shm_ptr = (TravelerMsg *)((char *)raw_shm + locks_size);
}

void initSemaphores(TravelerMsg *shared_mem, const int travelers_count) {
    // sem_init for node locks
    for (int n = 0; n < MAX_NODES; n++) {
        if (sem_init(&node_locks[n], 1, 1) !=
            0) { // pshared=1, initial value=1 (free)
            perror("sem_init node_lock failed");
            exit(EXIT_FAILURE);
        }
    }

    // sem_init for traveler messages
    for (int i = 0; i < travelers_count; i++) {
        shared_mem[i].pid = -1;
        shared_mem[i].current_node = -1;
        shared_mem[i].next_node = -1;
        shared_mem[i].queued_at_node = -1;

        // Initial value = 0 (Red Light). The mailbox is currently empty.
        if (sem_init(&shared_mem[i].sem_ready_to_read, 1, 0) != 0) {
            perror("sem_init sem_ready_to_read failed");
            exit(EXIT_FAILURE);
        }
        // Initial value = 1 (Green Light). There is 1 available empty slot.
        if (sem_init(&shared_mem[i].sem_ready_to_write, 1, 1) != 0) {
            perror("sem_init sem_ready_to_write failed");
            exit(EXIT_FAILURE);
        }
    }
}

void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result) {
    pid_t pid = getpid();

    // no path: publish synthetic arrival
    if (result.length <= 0) {
        shared_mem[traveler_index].total_hops = 0;
        sem_wait(&shared_mem[traveler_index].sem_ready_to_write);
        shared_mem[traveler_index].pid = pid;
        shared_mem[traveler_index].current_node = -1;
        shared_mem[traveler_index].next_node = -1;
        sem_post(&shared_mem[traveler_index].sem_ready_to_read);
        return;
    }

    shared_mem[traveler_index].total_hops = result.length - 1;

    // lock starting node (car is parked at the start)
    int curr = result.nodes[0];
    shared_mem[traveler_index].queued_at_node = curr;
    sem_wait(&node_locks[curr]);
    shared_mem[traveler_index].queued_at_node = -1;
    printf("[LOCK] PID=%d ACQUIRED node %d (start)\n", pid, curr);
    fflush(stdout);

    // consume initial sem_ready_to_write so future waits gate on real arrivals
    sem_wait(&shared_mem[traveler_index].sem_ready_to_write);

    // Lock lifecycle is split: child releases curr the moment it departs; GUI
    // acquires next at approach distance (and freezes the car outside if next
    // is held by another traveler).
    for (int j = 0; j < result.length - 1; j++) {
        int next = result.nodes[j + 1];

        // dwell at curr while parked (lock still held)
        sleep(1);

        // leaving curr: release its lock the moment the car departs
        printf("[LOCK] PID=%d RELEASING node %d\n", pid, curr);
        fflush(stdout);
        sem_post(&node_locks[curr]);

        // publish move; GUI moves the car and acquires next at APPROACH_T
        shared_mem[traveler_index].pid = pid;
        shared_mem[traveler_index].current_node = curr;
        shared_mem[traveler_index].next_node = next;
        sem_post(&shared_mem[traveler_index].sem_ready_to_read);

        // wait for GUI to signal visual arrival at next
        sem_wait(&shared_mem[traveler_index].sem_ready_to_write);

        curr = next;
    }

    // final arrival publish — car is parked at destination, lock held
    sleep(1);
    shared_mem[traveler_index].pid = pid;
    shared_mem[traveler_index].current_node = curr;
    shared_mem[traveler_index].next_node = -1;
    sem_post(&shared_mem[traveler_index].sem_ready_to_read);

    printf("[LOCK] PID=%d RELEASING node %d (final)\n", pid, curr);
    fflush(stdout);
    sem_post(&node_locks[curr]);
}

void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem,
                                      int count) {
  for (int i = 0; i < count; i++) {
    Car *car = &ctx->cars[i];

    if (!ctx->running)
      continue;

    // Consume a hop when the car is parked. CAR_NODE_WAIT is included because
    // UpdateCar no longer counts down a float timer and so never flips the
    // state back to CAR_IDLE on its own.
    if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT) {
      if (sem_trywait(&shared_mem[i].sem_ready_to_read) == 0) {
        int pid = shared_mem[i].pid;
        int curr = shared_mem[i].current_node;
        int next = shared_mem[i].next_node;

        if (next == -1) {
          printf("[PID=%d] arrived at node %d | DESTINATION\n", pid, curr);
          printf("[PID=%d] finished\n", pid);
        } else {
          printf("[PID=%d] arrived at node %d | next node: %d\n", pid, curr,
                 next);
        }
        fflush(stdout);
        ApplyTravelerUpdate(ctx, i, curr, next, shared_mem[i].total_hops);
        // sem_ready_to_write posted by UpdateCar at t>=1
      }
    }
  }
}
