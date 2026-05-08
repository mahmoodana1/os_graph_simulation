#include "../include/utils.h"
#define INF 1000000

/* printing is outside the algorithm logic */
void printPathResult(PathResult result) {
  if (result.found == 0) {
    printf("No path found\n");
    return;
  }

  for (int i = 0; i < result.length; i++) {
    printf("%d", result.nodes[i]);

    if (i < result.length - 1) {
      printf(" -> ");
    }
  }

  printf("\n");
  printf("%d\n", result.totalWeight);
}
