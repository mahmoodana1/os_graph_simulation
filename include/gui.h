#ifndef GUI_H
#define GUI_H

#include "car.h"
#include "graph.h"
#include "ipc.h"
#include "utils.h"
#include <sys/types.h>

/* --- Simulation Layout Constants --- */

#ifndef GUI_WIN_W
#define GUI_WIN_W 1280
#endif
#ifndef GUI_WIN_H
#define GUI_WIN_H 720
#endif

#define NUM_STATION_TYPES 4
#define MAX_TOASTS 8

/* --- Toast Notification --- */
typedef struct {
    char text[32]; /* "Traveler X" */
    Color color;
    float timer; /* counts down from TOAST_LIFETIME to 0 */
    bool active;
} Toast;

void DrawWeightBadge(Vector2 mid, int w, bool on_path);

/* --- Main Renderer Context --- */
typedef struct {
    Vector2 *positions;
    int node_count;
    Car *cars;
    int numCars;
    bool paused;
    bool running;
    bool all_arrived;
    Texture2D stationTextures[NUM_STATION_TYPES];
    Toast toasts[MAX_TOASTS];
} RenderCtx;

/* --- Public API --- */
RenderCtx *InitRenderer(int num_nodes, Vector2 *positions, int num_cars);
void RenderFrame(RenderCtx *ctx, Graph *g, float dt);
void FreeRenderer(RenderCtx *ctx);
void DrawArrowAt(Vector2 pos, float ca, float sa, float sz, Color col);
void DrawCarShape(float cx, float cy, float ca, float sa, float sz, Color col);
void DrawBackground(void);
RenderCtx *initGuiSetup(Graph *g, int num_travelers);
void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem,
                                      int count);
void ApplyTravelerUpdate(RenderCtx *ctx, int traveler_idx, int current_node,
                         int next_node, int total_hops);
void freeRenderer(RenderCtx *ctx);
#endif
