#include "../include/utils.h"
#include "../include/scheduler.h"
#define INF 1000000

void print_usage(const char *prog) {
    printf("Usage: %s [-schd fcfs|sjf] <input_file>\n", prog);
}

int parse_args(int argc, char *argv[], const char **input_path) {
    if (argc == 2) {
        *input_path = argv[1];
        g_scheduler = SCHED_FCFS;
        return 0;
    }

    if (argc == 4 && strcmp(argv[1], "-schd") == 0) {
        if (strcmp(argv[2], "fcfs") == 0) {
            g_scheduler = SCHED_FCFS;
        } else if (strcmp(argv[2], "sjf") == 0) {
            g_scheduler = SCHED_SJF;
        } else {
            print_usage(argv[0]);
            return 1;
        }
        *input_path = argv[3];
        return 0;
    }

    print_usage(argv[0]);
    return 1;
}

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
