#ifndef CAR_H
#define CAR_H

/* --- Car State Machine --- */
#include <raylib.h>
typedef enum {
    CAR_IDLE,
    CAR_MOVING,
    CAR_NODE_WAIT,
    CAR_QUEUED_OUTSIDE,
    CAR_ARRIVED
} CarState;

typedef struct {
    float x, y;
    float t;     /* Param along current edge   [0..1] */
    float speed; /* dt multiplier for t               */
    int id;      /* Display index (0-based) */
    /* Path through the graph (array of node indices, owned externally
       or by the caller; gui does not free it). */
    int *path;
    int path_len;
    int path_idx;
    int hops_done;
    int total_hops;
    float timer;
    CarState state;
    Color color;
    char path_str[128];
    float last_ca, last_sa; /* last heading, held across non-moving states */
    bool notified;          /* true once the arrival toast has fired */
    bool hop_mode;
    int queued_node;    /* node the car is queued outside, or -1 */
    bool target_locked; /* true once next node's lock is acquired (approach) */
    int queued_since;   /* monotonic tick stamped when car enters
                           CAR_QUEUED_OUTSIDE for the FCFS schedular, -1 otherwise
                         */
    int remaining_cost; /* weighted distance still to traverse from
                           current_node to dst; used by SJF */
    bool lock_contested; /* true once we've confirmed the target lock is held
                            by someone else or we lost a scheduler tie — gates
                            the wait spinner so a free-node pass doesn't
                            flicker the spinner for one frame */
} Car;

#endif // !DEBUG
