#ifndef UTILS_H
#define UTILS_H

#define MAX_PATH 100

#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int nodes[MAX_PATH];
    int length;
    int totalWeight;
    int found;
} PathResult;

typedef struct {
    int src;
    int dst;
} TravelerQuery;

typedef struct {
    TravelerQuery *travelers;
    int count;
} TravelerList;

void printPathResult(PathResult result);

#endif
