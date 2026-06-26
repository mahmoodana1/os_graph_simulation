#include "../include/scheduler.h"
#include "../include/graph.h"
#include <stdio.h>

sched_t g_scheduler = SCHED_FCFS;

/* Logs the scheduling decision for a node: who is waiting, who was chosen,
   and why. pick_winner runs every frame while cars contend for a node, so we
   remember the last car we announced per node and only re-print when the
   chosen traveler actually changes — otherwise the log would flood. */
static void log_decision(int target, Car **q, int n, int winner) {
    static int last_winner[MAX_NODES];
    static int initialized = 0;

    if (!initialized) {
        for (int i = 0; i < MAX_NODES; i++)
            last_winner[i] = -1;
        initialized = 1;
    }

    if (target < 0 || target >= MAX_NODES || winner < 0)
        return;

    int winner_id = q[winner]->id;
    if (last_winner[target] == winner_id)
        return; /* same traveler already announced for this node */
    last_winner[target] = winner_id;

    printf("\n");
    printf("    +==================== SCHEDULER DECISION ====================+\n");
    printf("    | scheduler : %-46s|\n", scheduler_name());
    printf("    | node      : %-46d|\n", target);
    printf("    | waiting   : %-46d|\n", n);
    printf("    +------------------------------------------------------------+\n");

    for (int i = 0; i < n; i++) {
        char line[64];
        if (g_scheduler == SCHED_SJF)
            snprintf(line, sizeof(line), "car %d  (remaining cost %d)%s",
                     q[i]->id, q[i]->remaining_cost,
                     i == winner ? "  <= CHOSEN" : "");
        else
            snprintf(line, sizeof(line), "car %d  (queued at tick %d)%s",
                     q[i]->id, q[i]->queued_since,
                     i == winner ? "  <= CHOSEN" : "");
        printf("    |   %-57s|\n", line);
    }

    printf("    +------------------------------------------------------------+\n");
    char verdict[64];
    if (g_scheduler == SCHED_SJF)
        snprintf(verdict, sizeof(verdict),
                 "=> car %d let in (SJF: shortest remaining cost %d)",
                 winner_id, q[winner]->remaining_cost);
    else
        snprintf(verdict, sizeof(verdict),
                 "=> car %d let in (FCFS: earliest in queue, tick %d)",
                 winner_id, q[winner]->queued_since);
    printf("    | %-59s|\n", verdict);
    printf("    +============================================================+\n");
    printf("\n");
    fflush(stdout);
}

int pick_winner(Car **queued, int n, int target, Graph *g) {
    if (!queued || n <= 0)
        return -1;

    int winner;
    if (g_scheduler == SCHED_SJF)
        winner = sjf_pick(queued, n, g);
    else
        winner = fcfs_pick(queued, n);

    log_decision(target, queued, n, winner);

    return winner;
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
    return "SJF";
}
