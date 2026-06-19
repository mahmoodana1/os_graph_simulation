#include "../include/scheduler.h"

sched_t g_scheduler = SCHED_FCFS;

int pick_winner(Car **queued, int n, int target, Graph *g) { return 0; }

int fcfs_pick(Car **q, int n) { return 0; }

int sjf_pick(Car **q, int n, Graph *g) {
    if (n <= 0)
        return -1;
    return 0;
}

const char *scheduler_name(void) {
    if (g_scheduler == SCHED_FCFS)
        return "FCFS";
    return "SJF";
}
