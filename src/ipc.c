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

    // Initial value = 0 (Red Light). The mailbox is currently empty.
    if (sem_init(&shared_mem[i].sem_ready_to_read, 1, 0) != 0) {
      perror("sem_init sem_ready_to_read failed");
      exit(EXIT_FAILURE);
    }
    // Initial value = 1 (Green Light). There is 1 available empty slot.
    if (sem_init(&shared_mem[i].sem_ready_to_write, 1, 0) != 0) {
      perror("sem_init sem_ready_to_write failed");
      exit(EXIT_FAILURE);
    }
  }
}

void writeTravelerPathToSharedMemory(TravelerMsg *shared_mem,
                                     int traveler_index, PathResult result) {
  shared_mem[traveler_index].total_hops = result.length - 1;
  for (int j = 0; j < result.length; j++) {
    int node = result.nodes[j];
    shared_mem[traveler_index].queued_at_node = node;

<<<<<<< Updated upstream
    // critical section, wait for the node to be free, then enter it
    sem_wait(&node_locks[node]);

    printf("[LOCK] PID=%d ACQUIRED node %d\n", getpid(), node);
    fflush(stdout);

    // after we are in the node, we can set queued_at_node to -1 to indicate
    // we are no longer waiting
    shared_mem[traveler_index].queued_at_node = -1;
    sem_wait(&shared_mem[traveler_index].sem_ready_to_write);

    // Write hop data
    shared_mem[traveler_index].pid = getpid();
    shared_mem[traveler_index].current_node = node;
    shared_mem[traveler_index].next_node =
        (j + 1 < result.length) ? result.nodes[j + 1] : -1;

    // signal to the gui that there is new data ready to animate
    sem_post(&shared_mem[traveler_index].sem_ready_to_read);
    // hold the intersectoin for 1 second before releasing
    sleep(1);
    // Release the intersection (critical section exit)
    printf("[LOCK] PID=%d RELEASING node %d\n", getpid(), node);
    fflush(stdout);
    sem_post(&node_locks[node]);
  }
=======
    // No path (e.g. directed graph has no 4->0 route): publish a synthetic
    // arrival so the GUI marks the car ARRIVED instead of sitting in CAR_IDLE.
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
    for (int j = 0; j < result.length; j++) {
        int node = result.nodes[j];
        shared_mem[traveler_index].queued_at_node = node;

        sem_wait(&node_locks[node]);
        printf("[LOCK] PID=%d ACQUIRED node %d\n", pid, node);
        fflush(stdout);

        shared_mem[traveler_index].queued_at_node = -1;
        sem_wait(&shared_mem[traveler_index].sem_ready_to_write);

        // Dwell at the intersection while still holding its lock. This used to
        // happen after the publish below (so it ran in parallel with the GUI
        // animating the *next* edge). Moving it here makes the lock span the
        // actual visit, matched with the GUI posting sem_ready_to_write the
        // moment t >= 1.0 (see UpdateCar) instead of running its own timer.
        sleep(1);

        shared_mem[traveler_index].pid = pid;
        shared_mem[traveler_index].current_node = node;
        shared_mem[traveler_index].next_node =
            (j + 1 < result.length) ? result.nodes[j + 1] : -1;

        sem_post(&shared_mem[traveler_index].sem_ready_to_read);

        printf("[LOCK] PID=%d RELEASING node %d\n", pid, node);
        fflush(stdout);
        sem_post(&node_locks[node]);
    }
>>>>>>> Stashed changes
}

void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem,
                                      int count) {
  for (int i = 0; i < count; i++) {

<<<<<<< Updated upstream
    Car *car = &ctx->cars[i];

    // Poll queued_at_node for cars that are idle or already queued outside
    if (car->state == CAR_IDLE || car->state == CAR_QUEUED_OUTSIDE) {
      int qnode = shared_mem[i].queued_at_node;
      if (qnode != -1) {
        car->state = CAR_QUEUED_OUTSIDE;
        car->queued_node = qnode;
        // Small per-car offset so multiple queued cars don't stack on the same
        // pixel
        int col = (i % 3) - 1; // gives -1, 0, or 1
        int row = (i / 3 % 3) - 1;
        car->x = ctx->positions[qnode].x + 18.0f * col;
        car->y = ctx->positions[qnode].y + 18.0f * row;
        printf("[GUI] car %d queued at node %d\n", i, qnode);
      } else if (car->state == CAR_QUEUED_OUTSIDE) {
        // queued_at_node flipped to -1: the child acquired the lock and is
        // about to post sem_ready_to_read.  Drop back to IDLE so the hop-
        // consumption block below can fire this frame (or the next one).
        car->state = CAR_IDLE;
        car->queued_node = -1;
      }
=======
        // Poll queued_at_node for cars that are idle or already queued outside
        if (car->state == CAR_IDLE || car->state == CAR_QUEUED_OUTSIDE) {
            int qnode = shared_mem[i].queued_at_node;
            if (qnode != -1) {
                car->state = CAR_QUEUED_OUTSIDE;
                car->queued_node = qnode;
                int col = (i % 3) - 1;
                int row = (i / 3 % 3) - 1;
                car->x = ctx->positions[qnode].x + 18.0f * col;
                car->y = ctx->positions[qnode].y + 18.0f * row;
                printf("[GUI] car %d queued at node %d\n", i, qnode);
            } else if (car->state == CAR_QUEUED_OUTSIDE) {
                car->state = CAR_IDLE;
                car->queued_node = -1;
            }
        }

        if (!ctx->running)
            continue;

        // Consume a hop when the car is parked. CAR_NODE_WAIT is included
        // because UpdateCar no longer counts down a float timer and so never
        // flips the state back to CAR_IDLE on its own.
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
>>>>>>> Stashed changes
    }

    // only consume hop data when simulation is running
    if (!ctx->running)
      continue;

    // Only consume a new hop when the car has fully finished the last one
    if (car->state == CAR_IDLE) {
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

        car->queued_node = -1;
        ApplyTravelerUpdate(ctx, i, curr, next, shared_mem[i].total_hops);
      }
    }
  }
}
