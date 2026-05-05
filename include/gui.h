// #ifndef GUI_H
// #define GUI_H
//
// #include "raylib.h"
// #include "graph.h"
//
// // open a window and initialize the GUI
// void startGui(Graph* g);
// void DrawArrowHead(Vector2 start, Vector2 end, Color color);
// #endif
//
// //
 #ifndef GUI_H
 #define GUI_H
// //
// ////////
//
//
// #include "raylib.h"
// #include "graph.h"
// #include <stdbool.h>
//
// /* ─── Palette ─────────────────────────────────────────────────── */
// #define COLOR_BG         (Color){ 244, 241, 222, 255 }   /* #F4F1DE paper */
// #define COLOR_NODE_SRC   (Color){ 224, 122,  95, 255 }   /* #E07A5F coral  */
// #define COLOR_NODE_DST   (Color){ 129, 178, 154, 255 }   /* #81B29A sage   */
// #define COLOR_NODE_DEF   (Color){  61,  64,  91, 255 }   /* #3D405B teal   */
// #define COLOR_NODE_PATH  (Color){ 241, 180,  47, 255 }   /* #F1B42F amber  */
// #define COLOR_EDGE       (Color){ 180, 176, 160, 255 }   /* warm grey      */
// #define COLOR_EDGE_PATH  (Color){ 241, 180,  47, 255 }   /* amber highlight*/
// #define COLOR_SHADOW     (Color){   0,   0,   0,  28 }
// #define COLOR_LABEL_DARK (Color){  61,  64,  91, 255 }
// #define COLOR_LABEL_LITE (Color){ 244, 241, 222, 255 }
// #define COLOR_CAR        (Color){ 255, 255, 255, 230 }
//
// /* ─── Layout constants ────────────────────────────────────────── */
// #define SCREEN_W        900
// #define SCREEN_H        620
// #define NODE_W           52
// #define NODE_H           34
// #define NODE_ROUND      0.45f
// #define SHADOW_OFF        4
// #define EDGE_THICK       5.0f
// #define PATH_THICK       7.0f
// #define CAR_W            14
// #define CAR_H             9
// #define CAR_ROUND       0.55f
// #define FONT_SIZE_NODE   17
// #define FONT_SIZE_WEIGHT 13
// #define FONT_SIZE_UI     18
// #define BUTTON_W        110
// #define BUTTON_H         38
//
// /* ─── Animation timing ────────────────────────────────────────── */
// #define HOP_DURATION_SEC  0.30f   /* 300 ms per hop (weight unit) */
// #define NODE_WAIT_SEC     1.00f   /* 1 s wait at intermediate nodes */
//
// /* ─── Visual node (layout, colour, label) ────────────────────── */
// typedef struct {
//     float   cx, cy;       /* centre in screen-space               */
//     Color   fill;
//     int     id;
// } VisualNode;
//
// /* ─── Car / traffic entity ───────────────────────────────────── */
// typedef enum {
//     CAR_STATE_WAITING,    /* paused at a node                     */
//     CAR_STATE_MOVING,     /* interpolating along an edge          */
//     CAR_STATE_ARRIVED,    /* reached destination                  */
//     CAR_STATE_STOPPED     /* user pressed Stop                    */
// } CarState;
//
// typedef struct {
//     float    x, y;        /* current screen position              */
//     int      path[64];    /* node id sequence                     */
//     int      path_len;
//     int      path_idx;    /* current segment start index          */
//     int      hop;         /* current hop within edge              */
//     int      total_hops;  /* weight of current edge               */
//     float    timer;       /* seconds elapsed in current phase     */
//     CarState state;
// } Car;
//
// /* ─── Full renderer context ──────────────────────────────────── */
// typedef struct {
//     VisualNode  nodes[64];
//     int         node_count;
//     int         src, dst;           /* query endpoints              */
//     int         dijkstra_path[64];
//     int         dijkstra_path_len;
//     Car         car;
//     bool        playing;
// } RenderCtx;
//
// /* ─── Public API ─────────────────────────────────────────────── */
//
// RenderCtx* InitRenderer(Graph* g, int src, int dst,
//                         int* dijkstra_path, int path_len);
//
// bool RenderFrame(RenderCtx* ctx, Graph* g, float dt);
//
// void FreeRenderer(RenderCtx* ctx);



////////


//////////////////

//
// #include "raylib.h"
// #include "graph.h"
// #include <stdbool.h>

/* ═══════════════════════════════════════════════════════════════
 *  PALETTE  — sampled from the Mini Motorways screenshot
 * ═══════════════════════════════════════════════════════════════ */
// #define MM_BG           (Color){ 240, 185,  90, 255 }  /* الخلفية الصفراء الكهرمانية */
// #define MM_ROAD         (Color){  45,  52,  70, 255 }  /* الطرق الداكنة (رمادي مزرق) */
// #define MM_ROAD_PATH    (Color){ 255, 255, 255, 255 } /* white active road  */
// #define MM_TEAL         (Color){  60, 155, 155, 255 }  /* teal accent        */
// #define MM_CORAL        (Color){ 210,  80,  70, 255 }  /* coral/red node     */
// #define MM_NAVY         (Color){  50,  65,  90, 255 }  /* dark navy node     */
// #define MM_NODE_BG      (Color){ 240, 230, 210, 255 }  /* node tile cream    */
// #define MM_NODE_SHADOW  (Color){   0,   0,   0,  40 }
// //#define MM_CAR          (Color){ 255, 255, 255, 245 }
// #define MM_HEART (Color){ 230, 50, 50, 255 } /* أحمر حيوي للقلب */
//
// #define MM_CAR_SHADOW   (Color){   0,   0,   0,  50 }
// #define MM_TEXT_DARK    (Color){  40,  40,  40, 255 }
// #define MM_TEXT_LIGHT   (Color){ 250, 248, 240, 255 }
// #define MM_BTN_PLAY     (Color){  60, 155, 155, 255 }
// #define MM_BTN_STOP     (Color){ 210,  80,  70, 255 }
//
// /* ═══════════════════════════════════════════════════════════════
//  *  SCREEN & LAYOUT
//  * ═══════════════════════════════════════════════════════════════ */
// #define SCREEN_W        900
// #define SCREEN_H        620
//
// #define ROAD_THICK       22.0f
// #define ROAD_THICK_PATH  34.0f
//
// #define NODE_SIZE        48
// #define NODE_ROUND       0.25f
// #define NODE_SHADOW_OFF   4
//
// #define CAR_W            16
// #define CAR_H            10
// #define CAR_ROUND        0.55f
//
// #define BTN_W            100
// #define BTN_H             36
//
// /* ═══════════════════════════════════════════════════════════════
//  *  ANIMATION TIMING
//  * ═══════════════════════════════════════════════════════════════ */
// #define HOP_DURATION_SEC  0.30f
// #define NODE_WAIT_SEC     1.00f
//
// /* ═══════════════════════════════════════════════════════════════
//  *  TYPES
//  * ═══════════════════════════════════════════════════════════════ */
//
// typedef struct {
//     float cx, cy;
//     int   id;
//     Color accent;
// } VisualNode;
//
// typedef enum {
//     CAR_IDLE,
//     CAR_MOVING,
//     CAR_NODE_WAIT,
//     CAR_ARRIVED,
//     CAR_PAUSED
// } CarState;
//
// typedef struct {
//     float    x, y;
//     int      path[64];
//     int      path_len;
//     int      seg;
//     int      hop;
//     int      total_hops;
//     float    timer;
//     CarState state;
// } Car;
//
// typedef struct {
//     VisualNode nodes[64];
//     int        node_count;
//     int        src, dst;
//     int        dijk_path[64];
//     int        dijk_len;
//     Car        car;
//     bool       playing;
// } RenderCtx;
//
// /* ═══════════════════════════════════════════════════════════════
//  *  PUBLIC API
//  * ═══════════════════════════════════════════════════════════════ */
// RenderCtx* InitRenderer(Graph* g, int src, int dst,
//                         int* path, int path_len);
// bool       RenderFrame(RenderCtx* ctx, Graph* g, float dt);
// void       FreeRenderer(RenderCtx* ctx);
//


///////////////////





#include "raylib.h"
#include "graph.h"
#include <stdbool.h>

#define MM_BG           (Color){ 240, 185,  90, 255 }
#define MM_ROAD         (Color){  45,  52,  70, 255 }
#define MM_ROAD_PATH    (Color){ 255, 255, 255, 255 }
#define MM_HEART        (Color){ 230,  50,  50, 255 }
#define MM_NODE_BG      (Color){ 240, 230, 210, 255 }
#define MM_TEXT_DARK    (Color){  40,  40,  40, 255 }
#define MM_BTN_PLAY     (Color){  60, 155, 155, 255 }
#define MM_BTN_STOP     (Color){ 210,  80,  70, 255 }

#define SCREEN_W        900
#define SCREEN_H        620
#define NODE_SIZE        48
#define ROAD_THICK       12.0f
#define HOP_DURATION_SEC  0.30f
#define NODE_WAIT_SEC     1.00f

typedef enum { CAR_IDLE, CAR_MOVING, CAR_NODE_WAIT, CAR_ARRIVED } CarState;

typedef struct {
 float x, y;
 int   path[64];
 int   path_len, seg, hop, total_hops;
 float timer;
 CarState state;
} Car;

typedef struct {
 Vector2 positions[64];
 Color   accents[64];
 int     node_count, src, dst;
 int     dijk_path[64], dijk_len;
 Car     car;
 bool    playing;
} RenderCtx;

RenderCtx* InitRenderer(Graph* g, int src, int dst, int* path, int path_len);
bool       RenderFrame(RenderCtx* ctx, Graph* g, float dt);
void       FreeRenderer(RenderCtx* ctx);


#endif