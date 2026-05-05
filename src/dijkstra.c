#ifndef FILE_H
#define FILE_H

#define INF 1000000

#include <stdio.h>
#include "../include/graph.h"
#include "string.h"

 int getMinDst(int dist[], int visited[], int n) {
     int min = INF;
     int minIndex = -1;

     for (int i = 0; i < n; i++) {
         if (dist[i] < min && visited[i] == 0) {
             min = dist[i];
             minIndex = i;
         }
     }

     return minIndex;
 }

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

void solveDijkstra(Graph *g, int start, int end) {
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

     printDijkstraResult(parent, dst, start, end);
 }


static int ReconstructPath(int parent[], int end, int* out_path)
 {
     int tmp[64], len = 0, v = end;
     while (v != -1 && len < 64) {
         tmp[len++] = v;
         v = parent[v];
     }
     for (int i = 0; i < len; i++)
         out_path[i] = tmp[len - 1 - i];
     return len;
 }




#endif
