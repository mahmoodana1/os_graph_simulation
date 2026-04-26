#include "../include/graph.h"
#include <stdio.h>
#include <stdlib.h>

Graph* createGraph(int numNodes) {

	Graph* graph = malloc( sizeof(Graph) );
    	if (!graph) {
        	printf("Error: failed to allocate graph\n");
        	return NULL;
    	}

	graph->num_nodes = numNodes;

    	// Initialize all adjacency list heads to NULL
    	for (int i = 0; i < numNodes; i++) {
        	graph->adj[i] = NULL;
    	}

	return graph;
}

void addEdge(Graph* graph, int src, int dst, int weight) {
	Node* newNode = malloc( sizeof(Node) );
	if( !newNode ) {
        printf("Error: failed to allocate edge node\n");
        return;
    	}
	
	    newNode->id = dst;
    	newNode->weight = weight;
    	newNode->next = graph->adj[src]; // prepend to list
    	graph->adj[src] = newNode;
}



Graph* load_graph(const char* filename, int* srcQuery, int* dstQuery) {
	FILE* file = fopen(filename, "r");
	if(!file) {
		printf("Could not open the file");
		return NULL;
	}

	
    // Read first line from input file → N (nodes), M (edges)
	int N,M;
	if(fscanf(file,"%d %d", &N, &M) != 2) {
		printf("Invalid file format on first line\nYou should provide nodes number & edges number");
		fclose(file);
		return NULL;
	}

	Graph* graph= createGraph(N);
	if( !graph ) {
		printf("Error creating the graph");
		return NULL;
	}

    // Loop throught input file M times to read adjacency list
	
   	for (int i = 0; i < M; i++) {
        	int src, dst, weight;

        	if (fscanf(file, "%d %d %d", &src, &dst, &weight) != 3) {
            		printf("Error: invalid edge format at edge %d\n", i+1);
			fclose(file);
			free_all(graph);
            		return NULL;
        	}

        	// Validate node indices are within range to avoid issues
        	if (src < 0 || src >= N || dst < 0 || dst >= N) {
            		printf("Error: node index out of range at edge %d\n", i+1);
            		fclose(file);
			free_all(graph);
            		return NULL;
        	}

        	addEdge(graph, src, dst, weight);
	}

	if( fscanf(file, "%d %d", srcQuery, dstQuery) != 2 ) {
		printf("Error: last line format is wrong\n");
		fclose(file);
		free_all(graph);
		return NULL;
	}
	
 	fclose(file);
	return graph;
}


// free the graph
void free_all(Graph* graph) {
	if (!graph) return;

	for (int i = 0; i < graph->num_nodes; i++) {
        	Node* curr = graph->adj[i];
        	while (curr != NULL) {
            		Node* tmp = curr;
            		curr = curr->next;
            		free(tmp);
        	}
    	}

	free(graph);
}
