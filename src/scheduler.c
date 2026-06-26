#include "../include/scheduler.h"

sched_t g_scheduler = SCHED_FCFS;

int pick_winner(Car **queued, int n, int target, Graph *g) {
    (void)target;

    if (!queued || n <= 0)
        return -1;

    if (g_scheduler == SCHED_SJF)
        return sjf_pick(queued, n, g);

    if (g_scheduler == SCHED_PRIORITY)
        return priority_pick(queued, n);

    return fcfs_pick(queued, n);
}

int priority_pick(Car **q, int n) {
    if (!q || n <= 0)
        return -1;

    int best = 0;

    for (int i = 1; i < n; i++) {
        if (q[i]->process_pid < q[best]->process_pid) {
            best = i;
        }
    }

    return best;
}
/* FCFS chooses the car that entered the queue first.
   If two cars have the same queue tick, the lower car id wins. */
int fcfs_pick(Car **q, int n) {
    if (!q || n <= 0)
        return -1;

    int best = 0;

    for (int i = 1; i < n; i++) {
        if (q[i]->queued_since < q[best]->queued_since ||
            (q[i]->queued_since == q[best]->queued_since &&
             q[i]->id < q[best]->id))
            best = i;
    }

    return best;
}

/* SJF chooses the car with the lowest remaining weighted path cost.
   The cost is computed by the child process (which owns the full Dijkstra
   path) and published to shared memory; the GUI mirrors it onto Car so the
   scheduler can rank queued cars without re-walking the graph here.
   If two cars have equal remaining cost, the lower car id wins. */
int sjf_pick(Car **q, int n, Graph *g) {
    (void)g;
    if (!q || n <= 0)
        return -1;

    int best = 0;

    for (int i = 1; i < n; i++) {
        if (q[i]->remaining_cost < q[best]->remaining_cost ||
            (q[i]->remaining_cost == q[best]->remaining_cost &&
             q[i]->id < q[best]->id))
            best = i;
    }

    return best;
}

const char *scheduler_name(void) {
    if (g_scheduler == SCHED_FCFS)
        return "FCFS";
    if (g_scheduler == SCHED_SJF)
        return "SJF";
    return "PRIORITY";
}
