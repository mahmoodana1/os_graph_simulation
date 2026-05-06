#include "../include/utils.h"
#include <stdio.h>
#define INF 1000000

void printPath(int parent[], int v) {
  if (parent[v] == -1) {
    printf("%d", v);
    return;
  }

  printPath(parent, parent[v]);
  printf(" -> %d", v);
}

void printDijkstraResult(int parent[], int dst[], int start, int end) {
  if (start == end) {
    printf("%d\n", start);
    return;
  }

  if (dst[end] == INF) {
    printf("No path found\n");
    return;
  }

  printPath(parent, end);
  printf("\n");
  printf("%d\n", dst[end]);
}
