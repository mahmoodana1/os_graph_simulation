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

/* Parses argv into input_path and writes g_scheduler. Returns 0 on success.
 * On failure prints usage and returns non-zero. Accepts either:
 *   prog <input_file>
 *   prog -schd fcfs|sjf <input_file>
 */
int parse_args(int argc, char *argv[], const char **input_path);
void print_usage(const char *prog);

#endif
