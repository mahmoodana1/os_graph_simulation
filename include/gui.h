#ifndef GUI_H
#define GUI_H

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

/* --- Car State Machine --- */
typedef enum {
  CAR_IDLE,
  CAR_QUEUED_OUTSIDE,
  CAR_MOVING,
  CAR_NODE_WAIT,
  CAR_ARRIVED
} CarState;

typedef struct {
  float x, y;
  float t;     /* Param along current edge   [0..1] */
  float speed; /* dt multiplier for t               */
  int id;      /* Display index (0-based) */
  /* Path through the graph (array of node indices, owned externally
     or by the caller; gui does not free it). */
  int *path;
  int path_len;
  int path_idx;
  int hops_done;
  int total_hops;
  int queued_node; /* node this car is waiting to enter (= -1 if not queued)*/
  float timer;
  CarState state;
  Color color;
  char path_str[128];
  float last_ca, last_sa; /* last heading, held across non-moving states */
  bool notified;          /* true once the arrival toast has fired */
  bool hop_mode;
} Car;

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
