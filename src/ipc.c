#include "../include/ipc.h"
#include "../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define MAX_TRAVELERS 8

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

        // Initial value = 0, The mailbox is currently empty
        if (sem_init(&shared_mem[i].sem_ready_to_read, 1, 0) != 0) {
            perror("sem_init sem_ready_to_read failed");
            exit(EXIT_FAILURE);
        }
        // Initial value = 1, There is 1 available empty slot
        if (sem_init(&shared_mem[i].sem_ready_to_write, 1, 0) != 0) {
            perror("sem_init sem_ready_to_write failed");
            exit(EXIT_FAILURE);
        }
    }
}

void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result) {
    pid_t pid = getpid();

    // instead of sitting idle with an empty path, we can immediately write the
    // "arrived at destination" message to the GUI so it can show the car at the
    // destination node without any delay
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

    int src = result.nodes[0];
    shared_mem[traveler_index].queued_at_node = src;
    sem_wait(&node_locks[src]);

    // block until the main loop posts the PLAY signal
    sem_wait(&shared_mem[traveler_index].sem_ready_to_write);
    shared_mem[traveler_index].queued_at_node = -1;

    // car is parked at curr.
    for (int j = 0; j < result.length - 1; j++) {
        int curr = result.nodes[j];
        int next = result.nodes[j + 1];

        // release curr BEFORE acquiring next so cars never hold one lock while
        // waiting on another. Holding both would deadlock on cyclic paths
        sem_post(&node_locks[curr]);

        shared_mem[traveler_index].queued_at_node = next;
        sem_wait(&node_locks[next]);
        shared_mem[traveler_index].queued_at_node = -1;

        // hand the GUI the new hop, It will animate curr -> next
        shared_mem[traveler_index].pid = pid;
        shared_mem[traveler_index].current_node = curr;
        shared_mem[traveler_index].next_node = next;
        sem_post(&shared_mem[traveler_index].sem_ready_to_read);

        // Block until the GUI finishes animating the edge into next
        sem_wait(&shared_mem[traveler_index].sem_ready_to_write);

        // hold next for one second
        sleep(1);
    }

    // car has arrived at the destination.
    int dst = result.nodes[result.length - 1];
    shared_mem[traveler_index].pid = pid;
    shared_mem[traveler_index].current_node = dst;
    shared_mem[traveler_index].next_node = -1;
    sem_post(&shared_mem[traveler_index].sem_ready_to_read);
    sem_post(&node_locks[dst]);
}

void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem,
                                      int count) {
    for (int i = 0; i < count; i++) {
        Car *car = &ctx->cars[i];

        // Poll queued_at_node so a car blocked on an intersection lock is shown
        if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT ||
            car->state == CAR_QUEUED_OUTSIDE) {
            int qnode = shared_mem[i].queued_at_node;
            if (qnode != -1) {
                car->queued_node = qnode;
                // Only place the car the first time we see it. Once it has been
                // animated we keep its current visual position to avoid jumps
                // when the child queues for the next_node along the path
                if (car->state == CAR_IDLE) {
                    int col = (i % 3) - 1;
                    int row = (i / 3 % 3) - 1;
                    car->x = ctx->positions[qnode].x + 18.0f * col;
                    car->y = ctx->positions[qnode].y + 18.0f * row;
                    printf("[GUI] car %d queued at node %d\n", i, qnode);
                }
                car->state = CAR_QUEUED_OUTSIDE;
            } else if (car->state == CAR_QUEUED_OUTSIDE) {
                // Lock acquired by the child. Fall back to the parked NODE_WAIT
                // state so the hopconsume block below can run.
                car->queued_node = -1;
                car->state = CAR_NODE_WAIT;
            }
        }
        // fixes pause update loop not consuming hops, but still allowing the
        // user to see queued cars
        if (!ctx->running)
            continue;

        // Consume a hop when the car is parked, either CAR_IDLE first hop
        // after spawn or CAR_NODE_WAIT animation into the previous node
        // completed
        if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT) {
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

                car->queued_node = -1;
                ApplyTravelerUpdate(ctx, i, curr, next,
                                    shared_mem[i].total_hops);
            }
        }
    }
}
