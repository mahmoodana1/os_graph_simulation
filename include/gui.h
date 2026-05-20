#ifndef GUI_H
#define GUI_H

#include "graph.h"
#include "utils.h"
#include <sys/types.h>

/* --- Simulation Layout Constants --- */
#define SCREEN_W 900
#define SCREEN_H 620
#define NODE_SIZE 48
#define ROAD_THICK 12.0f
#define HOP_DURATION_SEC 0.30f
#define NODE_WAIT_SEC 1.00f

/* --- Visual Theme Palette (Mini Motorways Style) --- */
#define MM_BG (Color){240, 185, 90, 255}
#define MM_ROAD (Color){45, 52, 70, 255}
#define MM_ROAD_PATH (Color){255, 255, 255, 255}
#define MM_RIVER (Color){160, 220, 230, 255}
#define MM_RIVER_LABEL (Color){140, 190, 200, 160}
#define MM_NODE_BG (Color){240, 230, 210, 255}
#define MM_YARD (Color){46, 52, 69, 255}
#define MM_SHADOW (Color){0, 0, 0, 45}
#define MM_HEART (Color){230, 50, 50, 255}
#define MM_TEXT_DARK (Color){40, 40, 40, 255}
#define MM_BTN_PLAY (Color){60, 155, 155, 255}
#define MM_BTN_STOP (Color){210, 80, 70, 255}

/* --- High-level GUI Entry Point --- */
/* paths[i] is the node-ID array for traveler i, path_lens[i] is its length */
// void startGui(Graph* g, int paths[][64], int* path_lens, int num_travelers);
void startGui(Graph *g, int paths[][64], int *path_lens, int num_travelers, pid_t *pids);
void DrawWeightBadge(Vector2 mid, int w, bool on_path);

/* --- Car State Machine --- */
typedef enum { CAR_IDLE, CAR_MOVING, CAR_NODE_WAIT, CAR_ARRIVED } CarState;

typedef struct {
    float x, y;
    int path[64];
    int path_len, seg, hop;
    int total_hops;
    float timer;
    CarState state;
    Color color;
} Car;

/* --- Main Renderer Context --- */
typedef struct {
    Vector2 positions[64];
    Color accents[64];
    int node_count;
    Car *cars;
    int numCars;
    bool playing;
} RenderCtx;

/* --- Public API --- */
RenderCtx *InitRenderer(Graph *g, int num_cars);
bool RenderFrame(RenderCtx *ctx, Graph *g, float dt);
void FreeRenderer(RenderCtx *ctx);

#endif
