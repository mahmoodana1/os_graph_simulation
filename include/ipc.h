#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
typedef struct {
  pid_t pid;
  int current_node;
  int next_node;
  int ready; // flag: 1= new message available, 0= already read
} TravelerMsg;

#endif
