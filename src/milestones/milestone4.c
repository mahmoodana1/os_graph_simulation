#include "../../include/dijkstra.h"
#include "../../include/graph.h"
#include "../../include/gui.h"
#include <stdio.h>

extern int BuildDijkstraPath(Graph *g, int start, int end, int *out_path);

// helper to print a single traveler
static void printTraveler(int i, TravelerQuery t) {
  printf("  travelers[%d] = { src: %d, dst: %d }\n", i, t.src, t.dst);
}

// ── TEST 1: normal valid file ────────────────────────────────────────────────
static void test_normal_load() {
  printf("=== TEST 1: Normal load ===\n");

  TravelerList travelers;
  Graph *g = loadGraph("data/input.txt", &travelers);

  if (!g) {
    printf("  FAIL: loadGraph returned NULL\n\n");
    return;
  }

  // graph structure
  printf("  Nodes : %d (expected 6)\n", g->num_nodes);
  printf("  %s\n", g->num_nodes == 6 ? "PASS" : "FAIL");

  // traveler count
  printf("  Traveler count: %d (expected 3)\n", travelers.count);
  printf("  %s\n", travelers.count == 3 ? "PASS" : "FAIL");

  // each traveler
  int expected_src[] = {0, 1, 2};
  int expected_dst[] = {5, 4, 3};
  for (int i = 0; i < travelers.count; i++) {
    printTraveler(i, travelers.travelers[i]);
    int ok = travelers.travelers[i].src == expected_src[i] &&
             travelers.travelers[i].dst == expected_dst[i];
    printf("  %s\n", ok ? "PASS" : "FAIL");
  }

  free(travelers.travelers);
  freeAll(g);
  printf("\n");
}

// ── TEST 2: file does not exist ──────────────────────────────────────────────
static void test_missing_file() {
  printf("=== TEST 2: Missing file ===\n");

  TravelerList travelers;
  Graph *g = loadGraph("data/no_such_file.txt", &travelers);

  printf("  loadGraph returned: %s\n", g == NULL ? "NULL" : "non-NULL");
  printf("  %s\n", g == NULL ? "PASS" : "FAIL");
  printf("\n");
}

// ── TEST 3: adjacency list spot-check ────────────────────────────────────────
static void test_edges() {
  printf("=== TEST 3: Edge list spot-check ===\n");

  TravelerList travelers;
  Graph *g = loadGraph("data/input.txt", &travelers);

  if (!g) {
    printf("  FAIL: loadGraph returned NULL\n\n");
    return;
  }

  // node 0 should have edges to 1 (w=7) and 2 (w=2)
  // edges are prepended so order is reversed: 2 first, then 1
  int found_01 = 0, found_02 = 0;
  Node *cur = g->adj[0];
  while (cur) {
    if (cur->id == 1 && cur->weight == 7)
      found_01 = 1;
    if (cur->id == 2 && cur->weight == 2)
      found_02 = 1;
    cur = cur->next;
  }
  printf("  Edge 0->1 (w=7): %s\n", found_01 ? "PASS" : "FAIL");
  printf("  Edge 0->2 (w=2): %s\n", found_02 ? "PASS" : "FAIL");

  free(travelers.travelers);
  freeAll(g);
  printf("\n");
}
int main(int argc, char *argv[]) {
  // edit as needed for testing
  printf("in milestone 4\n");
  test_normal_load();
  test_missing_file();
  test_edges();
}
