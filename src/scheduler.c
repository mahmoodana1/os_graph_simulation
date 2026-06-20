#include "../include/scheduler.h"

sched_t g_scheduler = SCHED_FCFS;

int pick_winner(Car **queued, int n, int target, Graph *g) {
    (void)target;

    if (n <= 0)
        return -1;

    if (g_scheduler == SCHED_SJF)
        return sjf_pick(queued, n, g);

    return fcfs_pick(queued, n);
}

int fcfs_pick(Car **q, int n) {
    if (n <= 0)
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

int sjf_pick(Car **q, int n, Graph *g) {
    if (n <= 0)
        return -1;

    int best = 0;
    int best_cost = path_remaining_cost(q[0], g);

    for (int i = 1; i < n; i++) {
        int cost = path_remaining_cost(q[i], g);

        if (cost < best_cost ||
            (cost == best_cost && q[i]->id < q[best]->id)) {
            best = i;
            best_cost = cost;
        }
    }

    return best;
}

const char *scheduler_name(void) {
    if (g_scheduler == SCHED_FCFS)
        return "FCFS";
    return "SJF";
}
