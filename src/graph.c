#include "../include/graph.h"

Graph *createGraph(int numNodes) {
  Graph *graph = malloc(sizeof(Graph) + numNodes * sizeof(Node *));
  if (!graph) {
    printf("Error: failed to allocate graph\n");
    return NULL;
  }
  graph->num_nodes = numNodes;
  for (int i = 0; i < numNodes; i++) {
    graph->adj[i] = NULL;
  }
  return graph;
}

void addEdge(Graph *graph, int src, int dst, int weight) {
  Node *newNode = malloc(sizeof(Node));
  if (!newNode) {
    printf("Error: failed to allocate edge node\n");
    return;
  }
  newNode->id = dst;
  newNode->weight = weight;
  newNode->next = graph->adj[src];
  graph->adj[src] = newNode;
}

Graph *loadGraph(const char *filename, TravelerList *travelers) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Could not open the file");
    return NULL;
  }

  char buf[256];
  int offset;
  int N, M;

    // Read first line: N (nodes), M (edges)
    if (!fgets(buf, sizeof(buf), file) ||
        sscanf(buf, "%d %d%n", &N, &M, &offset) != 2 ||
        (buf[offset] != '\n' && buf[offset] != '\0') || N < 1 || N > MAX_NODES ||
        M < 0) {
        printf("Invalid file format on first line\nYou should provide nodes number "
               "& edges number");
        fclose(file);
        return NULL;
    }

  Graph *graph = createGraph(N);
  if (!graph) {
    printf("Error creating the graph");
    fclose(file);
    return NULL;
  }

    // Read adjacency list
    for (int i = 0; i < M; i++) {
        int src, dst, weight;
        if (!fgets(buf, sizeof(buf), file) ||
            sscanf(buf, "%d %d %d%n", &src, &dst, &weight, &offset) != 3 ||
            (buf[offset] != '\n' && buf[offset] != '\0')) {
            printf("Error: invalid edge format at edge %d\n", i + 1);
            fclose(file);
            freeAll(graph);
            return NULL;
        }
        if (src < 0 || src >= N || dst < 0 || dst >= N || weight <= 0) {
            printf("Error: invalid edge at edge %d (node range or weight<=0)\n",
                   i + 1);
            fclose(file);
            freeAll(graph);
            return NULL;
        }
        addEdge(graph, src, dst, weight);
    }

  // Read traveler count (capped at MAX_TRAVELERS — downstream queues and
  // contender arrays are sized to that constant)
  int count;
  if (!fgets(buf, sizeof(buf), file) ||
      sscanf(buf, "%d%n", &count, &offset) != 1 ||
      (buf[offset] != '\n' && buf[offset] != '\0') || count < 1 ||
      count > MAX_TRAVELERS) {
    printf("Error: missing or invalid traveler count (1..%d)\n",
           MAX_TRAVELERS);
    fclose(file);
    freeAll(graph);
    return NULL;
  }

  travelers->travelers = malloc(count * sizeof(TravelerQuery));
  if (!travelers->travelers) {
    printf("Error: failed to allocate travelersList\n");
    fclose(file);
    freeAll(graph);
    return NULL;
  }
  travelers->count = count;

  for (int i = 0; i < count; i++) {
    if (!fgets(buf, sizeof(buf), file) ||
        sscanf(buf, "%d %d%n", &travelers->travelers[i].src,
               &travelers->travelers[i].dst, &offset) != 2 ||
        (buf[offset] != '\n' && buf[offset] != '\0') ||
        travelers->travelers[i].src < 0 || travelers->travelers[i].src >= N ||
        travelers->travelers[i].dst < 0 || travelers->travelers[i].dst >= N) {
      printf("Error: invalid traveler at traveler %d\n", i + 1);
      free(travelers->travelers);
      fclose(file);
      freeAll(graph);
      return NULL;
    }
  }

  // after reading all travelers
  char extra[256];
  if (fgets(extra, sizeof(extra), file) && extra[0] != '\n' &&
      extra[0] != '\0') {
    printf("Error: file has more data than expected\n");
    free(travelers->travelers);
    fclose(file);
    freeAll(graph);
    return NULL;
  }

  fclose(file);
  return graph;
}

void freeAll(Graph *graph) {
  if (!graph)
    return;
  for (int i = 0; i < graph->num_nodes; i++) {
    Node *curr = graph->adj[i];
    while (curr != NULL) {
      Node *tmp = curr;
      curr = curr->next;
      free(tmp);
    }
  }
  free(graph);
}
