#ifndef GUI_H
#define GUI_H

#include "utils.h"
#include "graph.h"

/* --- Simulation Layout Constants --- */
#define SCREEN_W         900
#define SCREEN_H         620
#define NODE_SIZE        48
#define ROAD_THICK       12.0f
#define HOP_DURATION_SEC 0.30f  /* Time to move between weight units */
#define NODE_WAIT_SEC    1.00f  /* Wait time at each intersection */

/* --- Visual Theme Palette (Mini Motorways Style) --- */
#define MM_BG            (Color){ 240, 185, 90, 255 }
#define MM_ROAD          (Color){ 45, 52, 70, 255 }
#define MM_ROAD_PATH     (Color){ 255, 255, 255, 255 }
#define MM_RIVER         (Color){ 160, 220, 230, 255 }
#define MM_RIVER_LABEL   (Color){ 140, 190, 200, 160 }
#define MM_NODE_BG       (Color){ 240, 230, 210, 255 }
#define MM_YARD          (Color){ 46, 52, 69, 255 }
#define MM_SHADOW        (Color){ 0, 0, 0, 45 }
#define MM_HEART         (Color){ 230, 50, 50, 255 }
#define MM_TEXT_DARK     (Color){ 40, 40, 40, 255 }
#define MM_BTN_PLAY      (Color){ 60, 155, 155, 255 }
#define MM_BTN_STOP      (Color){ 210, 80, 70, 255 }


Color distinct_colors[] = {RED,BLUE,GREEN,ORANGE,PURPLE,GOLD,LIME,MAGENTA,SKYBLUE,VIOLET};

/* --- High-level GUI Entry Point --- */
/**
 * Starts the GUI window, initializes the renderer, and runs the main loop.
 * This encapsulates all raylib and rendering logic.
 */
void startGui(Graph* g, int src, int dst);

void DrawWeightBadge(Vector2 mid, int w, bool on_path);

/* --- Car State Machine --- */
typedef enum
{
    CAR_IDLE,
    CAR_MOVING,
    CAR_NODE_WAIT,
    CAR_ARRIVED
} CarState;

typedef struct
{
    float x, y; /* Screen position */
    int path[64]; /* Sequence of node IDs */
    int path_len, seg, hop;
    int total_hops; /* Weight of current edge */
    float timer;
    CarState state;

    Color color;

} Car;

/* --- Main Renderer Context --- */
typedef struct
{
    Vector2 positions[64];
    Color accents[64];
    int node_count, src, dst;
    int dijk_path[64], dijk_len;
    Car car;
    bool playing;

    // We store the cars in a dynamic matrix
    Car* cars;
    int carCount;
} RenderCtx;

/* --- Public API --- */
RenderCtx* InitRenderer(Graph* g, int src, int dst, int* path, int path_len);
bool RenderFrame(RenderCtx* ctx, Graph* g, float dt);
void FreeRenderer(RenderCtx* ctx);

/* --- Algorithm API --- */
int BuildDijkstraPath(Graph* g, int start, int end, int* out_path);

#endif
