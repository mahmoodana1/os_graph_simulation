#ifndef UTILS_H
#define UTILS_H

#define MAX_PATH 100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <stdbool.h>

typedef struct {
  int nodes[MAX_PATH];
  int length;
  int totalWeight;
  int found;
} PathResult;

void printPathResult(PathResult result);

#endif
