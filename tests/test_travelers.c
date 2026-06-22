/*
 * test_travelers.c
 *
 * Extensive unit tests for traveler routing.
 *
 * "Travelers travel as expected" means three things, all checked here:
 *   1. Dijkstra picks a valid optimal path for every traveler's (src,dst).
 *   2. The returned PathResult is a real walk in the graph (every
 *      consecutive pair of nodes is connected by a real edge whose
 *      weight contributes to the reported totalWeight).
 *   3. loadGraph() parses the input format (nodes + edges + traveler
 *      list) and feeds Dijkstra correct queries.
 *
 * Reference oracle: Floyd-Warshall on the same adjacency list, so
 * Dijkstra's distances are cross-checked against an independent
 * algorithm. Any divergence fails the test.
 *
 * Run:
 *   make test_travelers && ./test_travelers
 *   (or: make test)
 */

#include "../include/dijkstra.h"
#include "../include/graph.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BIG 1000000

static int g_pass = 0;
static int g_fail = 0;
static const char *g_case = "<unset>";

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) {                                                            \
            g_pass++;                                                          \
        } else {                                                               \
            g_fail++;                                                          \
            fprintf(stderr, "  FAIL [%s] %s  (%s:%d)\n", g_case, msg,          \
                    __FILE__, __LINE__);                                       \
        }                                                                     \
    } while (0)

#define SECTION(name)                                                          \
    do {                                                                       \
        g_case = name;                                                        \
        printf("== %s ==\n", name);                                           \
    } while (0)

/* ---------- helpers ---------- */

static Graph *mk_graph(int n) {
    Graph *g = createGraph(n);
    if (!g) {
        fprintf(stderr, "createGraph(%d) returned NULL\n", n);
        exit(2);
    }
    return g;
}

/* Floyd-Warshall using min weight across parallel edges. */
static void floyd(Graph *g, int dist[MAX_NODES][MAX_NODES]) {
    int n = g->num_nodes;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dist[i][j] = (i == j) ? 0 : BIG;

    for (int u = 0; u < n; u++) {
        for (Node *e = g->adj[u]; e; e = e->next) {
            if (e->weight < dist[u][e->id])
                dist[u][e->id] = e->weight;
        }
    }
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
}

/* Returns true iff every consecutive (u,v) in path.nodes has a real
 * directed edge u->v in g, and the sum of cheapest matching edges
 * equals path.totalWeight. */
static int path_is_real_walk(Graph *g, PathResult r) {
    if (!r.found || r.length < 1) return 0;
    int sum = 0;
    for (int i = 0; i + 1 < r.length; i++) {
        int u = r.nodes[i];
        int v = r.nodes[i + 1];
        int best = BIG;
        for (Node *e = g->adj[u]; e; e = e->next) {
            if (e->id == v && e->weight < best) best = e->weight;
        }
        if (best == BIG) return 0;
        sum += best;
    }
    if (r.length == 1) return r.totalWeight == 0;
    return sum == r.totalWeight;
}

static void expect_path(Graph *g, int src, int dst, const char *label) {
    PathResult r = solveDijkstra(g, src, dst);

    int dist[MAX_NODES][MAX_NODES];
    floyd(g, dist);

    if (dist[src][dst] >= BIG) {
        char msg[160];
        snprintf(msg, sizeof msg, "%s: expected NO path %d->%d", label, src, dst);
        CHECK(r.found == 0, msg);
        return;
    }

    char msg[160];
    snprintf(msg, sizeof msg, "%s: path %d->%d found", label, src, dst);
    CHECK(r.found == 1, msg);
    if (!r.found) return;

    snprintf(msg, sizeof msg, "%s: path %d->%d starts at src", label, src, dst);
    CHECK(r.length >= 1 && r.nodes[0] == src, msg);

    snprintf(msg, sizeof msg, "%s: path %d->%d ends at dst", label, src, dst);
    CHECK(r.length >= 1 && r.nodes[r.length - 1] == dst, msg);

    snprintf(msg, sizeof msg, "%s: walk %d->%d is real", label, src, dst);
    CHECK(path_is_real_walk(g, r), msg);

    snprintf(msg, sizeof msg, "%s: weight %d->%d optimal (got %d, opt %d)",
             label, src, dst, r.totalWeight, dist[src][dst]);
    CHECK(r.totalWeight == dist[src][dst], msg);

    /* Cross-check BuildDijkstraPath (alternate API used by the GUI). */
    int buf[MAX_PATH];
    int len = BuildDijkstraPath(g, src, dst, buf);
    snprintf(msg, sizeof msg, "%s: BuildDijkstraPath len matches", label);
    CHECK(len == r.length, msg);

    int same = (len == r.length);
    for (int i = 0; same && i < len; i++)
        if (buf[i] != r.nodes[i]) same = 0;
    snprintf(msg, sizeof msg, "%s: BuildDijkstraPath nodes match", label);
    CHECK(same, msg);
}

/* Write a temp input file in the project's documented format. Returns
 * the path (static buffer). */
static const char *write_temp_input(const char *contents) {
    static char path[256];
    snprintf(path, sizeof path, "/tmp/og_sim_test_%d_%ld.txt", (int)getpid(),
             (long)time(NULL));
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen temp");
        exit(2);
    }
    fputs(contents, f);
    fclose(f);
    return path;
}

/* ---------- topology tests ---------- */

static void test_self_loop_traveler(void) {
    SECTION("traveler with src == dst");
    Graph *g = mk_graph(4);
    addEdge(g, 0, 1, 5);
    addEdge(g, 1, 2, 5);
    addEdge(g, 2, 3, 5);

    for (int v = 0; v < 4; v++) {
        char buf[32];
        snprintf(buf, sizeof buf, "self-loop@%d", v);
        PathResult r = solveDijkstra(g, v, v);
        CHECK(r.found == 1, "self-loop must be found");
        CHECK(r.length == 1 && r.nodes[0] == v, "self-loop length 1");
        CHECK(r.totalWeight == 0, "self-loop weight 0");
        (void)buf;
    }
    freeAll(g);
}

static void test_single_edge_direction(void) {
    SECTION("single directed edge");
    Graph *g = mk_graph(2);
    addEdge(g, 0, 1, 7);

    expect_path(g, 0, 1, "forward");
    expect_path(g, 1, 0, "reverse-disconnected");

    PathResult fwd = solveDijkstra(g, 0, 1);
    CHECK(fwd.totalWeight == 7, "directed forward weight 7");

    freeAll(g);
}

static void test_linear_chain(void) {
    SECTION("linear chain 0->1->2->3->4->5");
    Graph *g = mk_graph(6);
    int w[5] = {2, 3, 1, 4, 2};
    for (int i = 0; i < 5; i++) addEdge(g, i, i + 1, w[i]);

    for (int s = 0; s < 6; s++)
        for (int d = 0; d < 6; d++)
            expect_path(g, s, d, "linear");

    PathResult full = solveDijkstra(g, 0, 5);
    CHECK(full.length == 6, "chain 0..5 visits 6 nodes");
    CHECK(full.totalWeight == 12, "chain 0..5 weight 12");

    freeAll(g);
}

static void test_two_alternative_routes(void) {
    SECTION("cheap vs expensive parallel route");
    /* 0 -> 1 -> 3 (cost 4)   vs    0 -> 2 -> 3 (cost 100) */
    Graph *g = mk_graph(4);
    addEdge(g, 0, 1, 1);
    addEdge(g, 1, 3, 3);
    addEdge(g, 0, 2, 50);
    addEdge(g, 2, 3, 50);

    PathResult r = solveDijkstra(g, 0, 3);
    CHECK(r.found && r.totalWeight == 4, "picks cheap route");
    CHECK(r.length == 3 && r.nodes[1] == 1, "cheap route via 1");

    expect_path(g, 0, 3, "two-routes");
    freeAll(g);
}

static void test_diamond(void) {
    SECTION("diamond graph");
    /*       1
     *      / \
     *     0   3
     *      \ /
     *       2
     *  weights: 0->1=4, 1->3=2, 0->2=1, 2->3=10
     *  shortest 0->3 via 1: 6, NOT via 2 (11)
     */
    Graph *g = mk_graph(4);
    addEdge(g, 0, 1, 4);
    addEdge(g, 1, 3, 2);
    addEdge(g, 0, 2, 1);
    addEdge(g, 2, 3, 10);

    PathResult r = solveDijkstra(g, 0, 3);
    CHECK(r.found && r.totalWeight == 6, "diamond picks 0->1->3");
    CHECK(r.length == 3, "diamond length 3");
    CHECK(r.nodes[1] == 1, "diamond mid is 1");

    expect_path(g, 0, 3, "diamond");
    expect_path(g, 3, 0, "diamond-reverse-disconnected");
    freeAll(g);
}

static void test_disconnected_components(void) {
    SECTION("disconnected components");
    Graph *g = mk_graph(6);
    /* Component A: 0-1-2 */
    addEdge(g, 0, 1, 1);
    addEdge(g, 1, 2, 1);
    /* Component B: 3-4-5 */
    addEdge(g, 3, 4, 1);
    addEdge(g, 4, 5, 1);

    expect_path(g, 0, 2, "A internal");
    expect_path(g, 3, 5, "B internal");
    expect_path(g, 0, 5, "A->B blocked");
    expect_path(g, 5, 0, "B->A blocked");
    expect_path(g, 2, 3, "A2->B3 blocked");

    freeAll(g);
}

static void test_cycle_with_shortcut(void) {
    SECTION("cycle with shortcut");
    /* Ring 0->1->2->3->4->5->0 each weight 5
     * Plus shortcut 0->3 weight 3 */
    Graph *g = mk_graph(6);
    for (int i = 0; i < 6; i++) addEdge(g, i, (i + 1) % 6, 5);
    addEdge(g, 0, 3, 3);

    PathResult r = solveDijkstra(g, 0, 3);
    CHECK(r.found && r.totalWeight == 3, "ring 0->3 uses shortcut");
    CHECK(r.length == 2, "shortcut path length 2");

    PathResult r2 = solveDijkstra(g, 1, 3);
    CHECK(r2.found && r2.totalWeight == 10, "ring 1->3 walks ring (10)");
    expect_path(g, 0, 3, "ring-shortcut");
    expect_path(g, 1, 3, "ring-walk");
    expect_path(g, 5, 4, "ring-long-way");
    freeAll(g);
}

static void test_parallel_edges(void) {
    SECTION("parallel edges between same pair");
    Graph *g = mk_graph(3);
    /* Three parallel edges 0->1 with different weights; cheapest wins. */
    addEdge(g, 0, 1, 9);
    addEdge(g, 0, 1, 2);
    addEdge(g, 0, 1, 7);
    addEdge(g, 1, 2, 1);

    PathResult r = solveDijkstra(g, 0, 2);
    CHECK(r.found && r.totalWeight == 3, "parallel: picks min edge 2 (+1)");
    CHECK(r.length == 3, "parallel: length 3");
    CHECK(path_is_real_walk(g, r), "parallel: walk is real");
    expect_path(g, 0, 2, "parallel-edges");
    freeAll(g);
}

static void test_star_graph(void) {
    SECTION("star graph from node 0");
    Graph *g = mk_graph(6);
    for (int i = 1; i < 6; i++) addEdge(g, 0, i, i);

    /* From center: 1 hop each. */
    for (int i = 1; i < 6; i++) {
        PathResult r = solveDijkstra(g, 0, i);
        CHECK(r.found && r.length == 2 && r.totalWeight == i,
              "star: center->leaf");
        expect_path(g, 0, i, "star-out");
    }
    /* Leaf->leaf: unreachable (directed star). */
    expect_path(g, 2, 3, "leaf->leaf blocked");
    freeAll(g);
}

static void test_back_edges(void) {
    SECTION("back edges enable longer-detour shortest");
    /* 0->1->2  (2)
     * 0->3->2 (1+1=2) ties; tie-breaking is implementation-defined but
     * Dijkstra should still pick weight 2. */
    Graph *g = mk_graph(4);
    addEdge(g, 0, 1, 1);
    addEdge(g, 1, 2, 1);
    addEdge(g, 0, 3, 1);
    addEdge(g, 3, 2, 1);

    PathResult r = solveDijkstra(g, 0, 2);
    CHECK(r.found && r.totalWeight == 2, "tie: weight 2 either route");
    CHECK(r.length == 3, "tie: length 3");
    CHECK(path_is_real_walk(g, r), "tie: walk is real");
    expect_path(g, 0, 2, "back-edges");
    freeAll(g);
}

static void test_max_nodes_chain(void) {
    SECTION("chain at MAX_NODES");
    Graph *g = mk_graph(MAX_NODES);
    for (int i = 0; i + 1 < MAX_NODES; i++) addEdge(g, i, i + 1, 1);

    PathResult r = solveDijkstra(g, 0, MAX_NODES - 1);
    CHECK(r.found && r.length == MAX_NODES, "MAX_NODES chain length");
    CHECK(r.totalWeight == MAX_NODES - 1, "MAX_NODES chain weight");
    CHECK(path_is_real_walk(g, r), "MAX_NODES chain real walk");
    expect_path(g, 0, MAX_NODES - 1, "max-chain");
    freeAll(g);
}

static void test_complete_graph(void) {
    SECTION("complete directed graph");
    int n = 7;
    Graph *g = mk_graph(n);
    /* fully connected with weight = |i-j|+1 */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j) addEdge(g, i, j, (i > j ? i - j : j - i) + 1);

    for (int s = 0; s < n; s++)
        for (int d = 0; d < n; d++)
            expect_path(g, s, d, "complete");

    freeAll(g);
}

/* ---------- file loader + multi-traveler tests ---------- */

static void test_load_input_file_provided(void) {
    SECTION("loadGraph: data/input.txt");
    TravelerList tl = {0};
    Graph *g = loadGraph("data/input.txt", &tl);
    CHECK(g != NULL, "data/input.txt loads");
    if (!g) return;

    CHECK(g->num_nodes == 10, "data/input.txt num_nodes == 10");
    CHECK(tl.count == 3, "data/input.txt traveler count == 3");

    /* Run Dijkstra for every traveler; verify all paths are valid walks. */
    for (int i = 0; i < tl.count; i++) {
        char lbl[48];
        snprintf(lbl, sizeof lbl, "input.txt traveler %d", i);
        expect_path(g, tl.travelers[i].src, tl.travelers[i].dst, lbl);
    }

    free(tl.travelers);
    freeAll(g);
}

static void test_load_many_travelers(void) {
    SECTION("loadGraph: max travelers (8)");
    const char *path = write_temp_input(
        "5 6\n"
        "0 1 2\n"
        "1 2 2\n"
        "2 3 2\n"
        "3 4 2\n"
        "0 4 10\n"
        "4 0 1\n"
        "8\n"
        "0 4\n"
        "1 3\n"
        "2 4\n"
        "4 0\n"
        "0 0\n"
        "3 3\n"
        "1 4\n"
        "2 2\n");

    TravelerList tl = {0};
    Graph *g = loadGraph(path, &tl);
    CHECK(g != NULL, "multi-traveler file loads");
    if (!g) return;

    CHECK(tl.count == 8, "8 travelers parsed");
    CHECK(g->num_nodes == 5, "5 nodes parsed");

    int dist[MAX_NODES][MAX_NODES];
    floyd(g, dist);

    int expect_found[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    for (int i = 0; i < tl.count; i++) {
        PathResult r = solveDijkstra(g, tl.travelers[i].src,
                                     tl.travelers[i].dst);
        char m[80];
        snprintf(m, sizeof m, "traveler %d found", i);
        CHECK(r.found == expect_found[i], m);
        if (!r.found) continue;
        snprintf(m, sizeof m, "traveler %d walk real", i);
        CHECK(path_is_real_walk(g, r), m);
        snprintf(m, sizeof m, "traveler %d weight optimal", i);
        CHECK(r.totalWeight ==
                  dist[tl.travelers[i].src][tl.travelers[i].dst],
              m);
    }

    free(tl.travelers);
    freeAll(g);
    unlink(path);
}

static void test_load_unreachable_traveler(void) {
    SECTION("loadGraph: unreachable traveler returns not-found");
    /* Two disconnected components; one traveler goes across. */
    const char *path = write_temp_input(
        "4 2\n"
        "0 1 5\n"
        "2 3 5\n"
        "2\n"
        "0 1\n"
        "0 3\n");
    TravelerList tl = {0};
    Graph *g = loadGraph(path, &tl);
    CHECK(g != NULL, "disconnected file loads");
    if (!g) return;
    CHECK(tl.count == 2, "2 travelers parsed");

    PathResult ok = solveDijkstra(g, tl.travelers[0].src, tl.travelers[0].dst);
    CHECK(ok.found && ok.totalWeight == 5, "reachable traveler ok");

    PathResult bad = solveDijkstra(g, tl.travelers[1].src, tl.travelers[1].dst);
    CHECK(bad.found == 0, "cross-component traveler not found");

    free(tl.travelers);
    freeAll(g);
    unlink(path);
}

static void test_load_rejects_bad_files(void) {
    SECTION("loadGraph: rejects malformed input");

    /* missing traveler section */
    const char *p1 = write_temp_input("3 2\n0 1 1\n1 2 1\n");
    TravelerList tl1 = {0};
    Graph *g1 = loadGraph(p1, &tl1);
    CHECK(g1 == NULL, "missing traveler count rejected");
    if (g1) freeAll(g1);
    unlink(p1);

    /* negative weight */
    const char *p2 =
        write_temp_input("3 1\n0 1 -3\n1\n0 2\n");
    TravelerList tl2 = {0};
    Graph *g2 = loadGraph(p2, &tl2);
    CHECK(g2 == NULL, "negative weight rejected");
    if (g2) freeAll(g2);
    unlink(p2);

    /* node out of range */
    const char *p3 = write_temp_input("3 1\n0 9 1\n1\n0 2\n");
    TravelerList tl3 = {0};
    Graph *g3 = loadGraph(p3, &tl3);
    CHECK(g3 == NULL, "edge to out-of-range node rejected");
    if (g3) freeAll(g3);
    unlink(p3);

    /* traveler with out-of-range node */
    const char *p4 = write_temp_input("3 1\n0 1 1\n1\n0 5\n");
    TravelerList tl4 = {0};
    Graph *g4 = loadGraph(p4, &tl4);
    CHECK(g4 == NULL, "traveler with bad node rejected");
    if (g4) freeAll(g4);
    unlink(p4);

    /* too many nodes */
    char too_big[32];
    snprintf(too_big, sizeof too_big, "%d 0\n1\n0 0\n", MAX_NODES + 1);
    const char *p5 = write_temp_input(too_big);
    TravelerList tl5 = {0};
    Graph *g5 = loadGraph(p5, &tl5);
    CHECK(g5 == NULL, "nodes > MAX_NODES rejected");
    if (g5) freeAll(g5);
    unlink(p5);

    /* traveler count over MAX_TRAVELERS */
    char too_many[256];
    snprintf(too_many, sizeof too_many, "2 1\n0 1 1\n%d\n", MAX_TRAVELERS + 1);
    size_t off = strlen(too_many);
    for (int i = 0; i <= MAX_TRAVELERS; i++)
        off += snprintf(too_many + off, sizeof(too_many) - off, "0 1\n");
    const char *p6 = write_temp_input(too_many);
    TravelerList tl6 = {0};
    Graph *g6 = loadGraph(p6, &tl6);
    CHECK(g6 == NULL, "travelers > MAX_TRAVELERS rejected");
    if (g6) freeAll(g6);
    unlink(p6);
}

/* ---------- randomized stress test ---------- */

static void test_random_graphs(void) {
    SECTION("randomized graphs vs Floyd-Warshall");
    srand(0xC0FFEE);

    int trials = 60;
    for (int t = 0; t < trials; t++) {
        int n = 3 + (rand() % (MAX_NODES - 2));
        Graph *g = mk_graph(n);

        /* sparse-to-dense random directed edges, weights 1..9 */
        int edge_attempts = n * n / 2;
        for (int e = 0; e < edge_attempts; e++) {
            int u = rand() % n;
            int v = rand() % n;
            if (u == v) continue;
            int w = 1 + (rand() % 9);
            addEdge(g, u, v, w);
        }

        int dist[MAX_NODES][MAX_NODES];
        floyd(g, dist);

        for (int s = 0; s < n; s++) {
            for (int d = 0; d < n; d++) {
                PathResult r = solveDijkstra(g, s, d);
                if (dist[s][d] >= BIG) {
                    char m[80];
                    snprintf(m, sizeof m,
                             "random t=%d %d->%d expected no path", t, s, d);
                    CHECK(r.found == 0, m);
                } else {
                    char m[80];
                    snprintf(m, sizeof m, "random t=%d %d->%d weight", t, s, d);
                    CHECK(r.found == 1 && r.totalWeight == dist[s][d], m);
                    if (r.found) {
                        snprintf(m, sizeof m,
                                 "random t=%d %d->%d real walk", t, s, d);
                        CHECK(path_is_real_walk(g, r), m);
                    }
                }
            }
        }

        freeAll(g);
    }
}

/* ---------- main ---------- */

int main(void) {
    test_self_loop_traveler();
    test_single_edge_direction();
    test_linear_chain();
    test_two_alternative_routes();
    test_diamond();
    test_disconnected_components();
    test_cycle_with_shortcut();
    test_parallel_edges();
    test_star_graph();
    test_back_edges();
    test_max_nodes_chain();
    test_complete_graph();

    test_load_input_file_provided();
    test_load_many_travelers();
    test_load_unreachable_traveler();
    test_load_rejects_bad_files();

    test_random_graphs();

    printf("\n--------------------------------\n");
    printf("Traveler tests: %d passed, %d failed\n", g_pass, g_fail);
    printf("--------------------------------\n");
    return g_fail == 0 ? 0 : 1;
}
