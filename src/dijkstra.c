#define INF 1000000

#include "../include/dijkstra.h"
#include "../include/graph.h"

/* get the unvisited node with the smallest distance */
int getMinDst(int dist[], int visited[], int n) {
    int min = INF;
    int minIndex = -1;

    for (int i = 0; i < n; i++) {
        if (visited[i] == 0 && dist[i] < min) {
            min = dist[i];
            minIndex = i;
        }
    }

    return minIndex;
}

/* build the ordered path from start to end using parent array */
PathResult buildPathResult(int parent[], int start, int end, int totalWeight) {
    PathResult result;

    result.length = 0;
    result.totalWeight = totalWeight;
    result.found = 1;

    int temp[MAX_PATH];
    int count = 0;
    int current = end;

    while (current != -1) {
        temp[count] = current;
        count++;

        if (current == start) {
            break;
        }

        current = parent[current];
    }

    for (int i = count - 1; i >= 0; i--) {
        result.nodes[result.length] = temp[i];
        result.length++;
    }

    return result;
}

/* only calculate the shortest path, no printing here */
PathResult solveDijkstra(Graph *g, int start, int end) {
    PathResult result;

    result.length = 0;
    result.totalWeight = 0;
    result.found = 0;

    int numofVertises = g->num_nodes;

    int dst[numofVertises];
    int visited[numofVertises];
    int parent[numofVertises];

    for (int i = 0; i < numofVertises; i++) {
        dst[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    dst[start] = 0;

    for (int i = 0; i < numofVertises; i++) {
        int temp = getMinDst(dst, visited, numofVertises);

        if (temp == -1) {
            break;
        }

        visited[temp] = 1;

        Node *current = g->adj[temp];

        while (current != NULL) {
            int v = current->id;
            int w = current->weight;

            if (visited[v] == 0 && dst[temp] + w < dst[v]) {
                dst[v] = dst[temp] + w;
                parent[v] = temp;
            }

            current = current->next;
        }
    }

    if (dst[end] == INF) {
        return result;
    }

    return buildPathResult(parent, start, end, dst[end]);
}

/**
 * Computes the shortest path using Dijkstra's Algorithm
 * @param g The input graph structure
 * @param start Source node ID
 * @param end Destination node ID
 * @param out_path Buffer to store the resulting node sequence
 * @return Number of nodes in the shortest path
 */
int BuildDijkstraPath(Graph *g, int start, int end, int *out_path) {
    int n = g->num_nodes;
    int dist[64], visited[64], parent[64];
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[start] = 0;

    for (int iter = 0; iter < n; iter++) {
        int u = -1, minD = INF;
        for (int i = 0; i < n; i++)
            if (!visited[i] && dist[i] < minD) {
                minD = dist[i];
                u = i;
            }
        if (u == -1 || u == end)
            break;
        visited[u] = 1;

        Node *edge = g->adj[u];
        while (edge) {
            if (!visited[edge->id] && dist[u] + edge->weight < dist[edge->id]) {
                dist[edge->id] = dist[u] + edge->weight;
                parent[edge->id] = u;
            }
            edge = edge->next;
        }
    }

    if (dist[end] == INF)
        return 0;
    int tmp[64], len = 0, v = end;
    while (v != -1) {
        tmp[len++] = v;
        v = parent[v];
    }
    for (int i = 0; i < len; i++)
        out_path[i] = tmp[len - 1 - i];
    return len;
}
