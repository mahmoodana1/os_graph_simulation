#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "graph.h"
#include "gui.h"

typedef enum {
    SCHED_FCFS,
    SCHED_SJF,
    SCHED_PRIORITY
} sched_t;

extern sched_t g_scheduler;

int pick_winner(Car **queued, int n, int target, Graph *g);
int fcfs_pick(Car **q, int n);
int sjf_pick(Car **q, int n, Graph *g);
const char *scheduler_name(void);

#endif
