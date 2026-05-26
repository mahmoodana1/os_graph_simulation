#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

extern char *shm_ptr;
extern int shm_id;
extern pid_t main_pid;

typedef struct {
    pid_t pid;
    int current_node;
    int next_node;
    int ready; // flag: 1= new message available, 0= already read
} TravelerMsg;

void cleanup(int);
void createShm();

#endif
