#include "../include/gui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raymath.h>

static const Color NODE_ACCENTS[] = {
    {210, 80, 70, 255}, {60, 155, 155, 255},
    {50, 65, 90, 255}, {200, 150, 60, 255}
};

/**
 * Entry point for the graphical interface.
 * Handles window lifecycle, context initialization, and the main frame loop.
 */
void startGui(Graph* g, int src, int dst) {
    int path[64];

    /* 1. Calculate the path internally to keep main.c clean */
    int path_len = BuildDijkstraPath(g, src, dst, path);

    if (path_len <= 0) {
        printf("GUI Error: No valid path found between nodes %d and %d.\n", src, dst);
        return;
    }

    /* 2. Initialize Raylib window context */
    InitWindow(SCREEN_W, SCREEN_H, "OS Graph Simulation - Traffic Flow");
    SetTargetFPS(60);

    /* 3. Prepare the visual rendering context */
    RenderCtx* ctx = InitRenderer(g, src, dst, path, path_len);

    /* 4. Main Simulation loop */
    while (RenderFrame(ctx, g, GetFrameTime()));

    /* 5. Resource cleanup and window shutdown */
    FreeRenderer(ctx);
    CloseWindow();
}


/* Internal Helper: Cubic Bezier point calculation */
static Vector2 GetBezierPoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t) {
    float invT = 1.0f - t;
    return (Vector2){
        p0.x * (invT*invT*invT) + p1.x * (3*invT*invT*t) + p2.x * (3*invT*t*t) + p3.x * (t*t*t),
        p0.y * (invT*invT*invT) + p1.y * (3*invT*invT*t) + p2.y * (3*invT*t*t) + p3.y * (t*t*t)
    };
}

/* Internal Helper: Draw curved river branches with floating labels */
static void DrawRiverBranch(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p1, float thick) {
    Vector2 pts[4] = { p0, c1, c2, p1 };
    DrawSplineBezierCubic(pts, 4, thick, MM_RIVER);

    for (float t = 0.3f; t <= 0.7f; t += 0.4f) {
        Vector2 pos = GetBezierPoint(p0, c1, c2, p1, t);
        Vector2 next = GetBezierPoint(p0, c1, c2, p1, t + 0.01f);
        float angle = atan2f(next.y - pos.y, next.x - pos.x) * (180.0f / PI);
        DrawTextPro(GetFontDefault(), "ISAR", pos, (Vector2){0,0}, angle, 12.0f, 2.0f, MM_RIVER_LABEL);
    }
}

/* Internal Helper: Draw directional road markers */
static void DrawRoadArrow(Vector2 start, Vector2 end, Color color) {
    Vector2 dir = Vector2Subtract(end, start);
    if (Vector2Length(dir) < 40.0f) return;
    float angle = atan2f(dir.y, dir.x) * (180.0f / PI);
    DrawPoly(Vector2Add(start, Vector2Scale(dir, 0.25f)), 3, 8.0f, angle, color);
    DrawPoly(Vector2Add(start, Vector2Scale(dir, 0.75f)), 3, 8.0f, angle, color);
}

/* Internal Helper: Draw node tile with shadows and decorations */
static void DrawNodeTile(RenderCtx* ctx, int idx) {
    Vector2 p = ctx->positions[idx];
    float yardSize = NODE_SIZE * 1.5f;

    // 1. Drop shadow
    DrawRectangleRounded((Rectangle){p.x - yardSize/2 + 4, p.y - yardSize/2 + 4, yardSize, yardSize}, 0.3f, 10, MM_SHADOW);
    // 2. Yard area
    DrawRectangleRounded((Rectangle){p.x - yardSize/2, p.y - yardSize/2, yardSize, yardSize}, 0.3f, 10, MM_YARD);
    // 3. Main building tile
    Rectangle r = {p.x - NODE_SIZE/2, p.y - NODE_SIZE/2, NODE_SIZE, NODE_SIZE};
    DrawRectangleRounded(r, 0.25f, 8, MM_NODE_BG);

    Color acc = (idx == ctx->src) ? MM_BTN_STOP : (idx == ctx->dst) ? MM_BTN_PLAY : ctx->accents[idx % 4];
    DrawRectangleRounded((Rectangle){r.x, r.y, NODE_SIZE, NODE_SIZE*0.5f}, 0.25f, 6, acc);

    // 4. Windows/Eyes decoration
    if (idx % 3 == 0) {
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 1.5f, BLACK);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 1.5f, BLACK);
    } else {
        DrawCircleV((Vector2){p.x, p.y - 6}, 4, WHITE);
        DrawCircleV((Vector2){p.x, p.y - 6}, 2, BLACK);
    }

    // 5. Node ID Label
    char buf[8]; sprintf(buf, "%d", idx);
    DrawText(buf, (int)(p.x - MeasureText(buf, 16)/2), (int)(p.y + 6), 16, MM_TEXT_DARK);
}

/* Update simulation logic: Car movement and state transitions */
void UpdateCar(Car* car, RenderCtx* ctx, Graph* g, float dt) {
    car->timer += dt;
    if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT) {
        car->x = ctx->positions[car->path[car->seg]].x;
        car->y = ctx->positions[car->path[car->seg]].y;
        float wait = (car->seg == 0) ? 0.0f : NODE_WAIT_SEC;
        if (car->timer >= wait) {
            if (car->seg + 1 >= car->path_len) { car->state = CAR_ARRIVED; return; }
            car->timer = 0; car->hop = 0;
            Node* e = g->adj[car->path[car->seg]];
            while(e) { if(e->id == car->path[car->seg+1]) { car->total_hops = e->weight; break; } e = e->next; }
            car->state = CAR_MOVING;
        }
    } else if (car->state == CAR_MOVING) {
        Vector2 s = ctx->positions[car->path[car->seg]], d = ctx->positions[car->path[car->seg+1]];
        float progress = (car->hop + fminf(car->timer/HOP_DURATION_SEC, 1.0f)) / car->total_hops;
        car->x = Lerp(s.x, d.x, progress); car->y = Lerp(s.y, d.y, progress);
        if (car->timer >= HOP_DURATION_SEC) {
            car->timer = 0; car->hop++;
            if (car->hop >= car->total_hops) { car->seg++; car->state = CAR_NODE_WAIT; }
        }
    }
}

/* Initialization: Prepare positions and car initial state */
RenderCtx* InitRenderer(Graph* g, int src, int dst, int* path, int path_len) {
    RenderCtx* ctx = calloc(1, sizeof(RenderCtx));
    ctx->src = src; ctx->dst = dst; ctx->dijk_len = path_len;
    memcpy(ctx->dijk_path, path, path_len * sizeof(int));
    for (int i = 0; i < g->num_nodes; i++) {
        float a = (float)i/g->num_nodes * 2.0f * PI - 1.57f;
        ctx->positions[i] = (Vector2){ SCREEN_W/2 + 210*cosf(a), SCREEN_H/2 + 210*sinf(a) };
        ctx->accents[i] = NODE_ACCENTS[i % 4];
    }
    memcpy(ctx->car.path, path, path_len * sizeof(int));
    ctx->car.path_len = path_len;
    ctx->car.state = CAR_IDLE;
    return ctx;
}

/* Main Render Loop: Coordinates background, roads, nodes, and UI */
bool RenderFrame(RenderCtx* ctx, Graph* g, float dt) {
    if (WindowShouldClose()) return false;
    if (ctx->playing && ctx->car.state != CAR_ARRIVED) UpdateCar(&ctx->car, ctx, g, dt);

    BeginDrawing();
    ClearBackground(MM_BG);

    // 1. Background Environment
    DrawRiverBranch((Vector2){-100, 450}, (Vector2){300, 650}, (Vector2){500, 250}, (Vector2){1000, 350}, 30.0f);
    DrawRiverBranch((Vector2){300, -50}, (Vector2){200, 250}, (Vector2){450, 400}, (Vector2){700, 800}, 20.0f);

    // 2. Roads Infrastructure
    for (int i = 0; i < g->num_nodes; i++) {
        Node* e = g->adj[i];
        while (e) {
            bool on = false;
            for (int k = 0; k + 1 < ctx->dijk_len; k++)
                if (ctx->dijk_path[k] == i && ctx->dijk_path[k+1] == e->id) { on = true; break; }
            if (!on) {
                DrawLineEx(ctx->positions[i], ctx->positions[e->id], ROAD_THICK, MM_ROAD);
                DrawRoadArrow(ctx->positions[i], ctx->positions[e->id], (Color){200,200,200,150});
            }
            e = e->next;
        }
    }

    // 3. Highlighted Shortest Path
    for (int i = 0; i + 1 < ctx->dijk_len; i++) {
        Vector2 s = ctx->positions[ctx->dijk_path[i]], d = ctx->positions[ctx->dijk_path[i+1]];
        DrawLineEx(s, d, ROAD_THICK + 6, MM_ROAD_PATH);
        DrawRoadArrow(s, d, MM_YARD);
    }

    // 4. Tiles and Entities
    for (int i = 0; i < g->num_nodes; i++) DrawNodeTile(ctx, i);
    if (ctx->playing && (ctx->car.state == CAR_MOVING || ctx->car.state == CAR_NODE_WAIT)) {
        Vector2 s = ctx->positions[ctx->car.path[ctx->car.seg]], d = ctx->positions[ctx->car.path[ctx->car.seg+1]];
        float angle = atan2f(d.y - s.y, d.x - s.x) * (180/PI);
        DrawRectanglePro((Rectangle){ctx->car.x, ctx->car.y, 16, 9}, (Vector2){8, 4.5f}, angle, (Color){50,120,220,255});
    }

    if (ctx->car.state == CAR_ARRIVED) {
        // Define banner dimensions
        float bannerW = 350;
        float bannerH = 80;
        Rectangle banner = { (SCREEN_W - bannerW)/2, (SCREEN_H - bannerH)/2 - 50, bannerW, bannerH };

        // Draw shadow and main banner (Rounded style)
        //DrawRectangleRounded((Rectangle){ banner.x + 2, banner.y + 2, banner.x + bannerW, banner.y + bannerH }, 0.4f, 10, MM_SHADOW);
        DrawRectangleRounded(banner, 0.4f, 10, MM_ROAD); // Dark banner to contrast with amber BG

        // Add a thin accent line at the top of the banner
        DrawRectangleRounded((Rectangle){ banner.x, banner.y, bannerW, 8 }, 0.4f, 10, MM_BTN_PLAY);

        // Center the text
        const char* msg = "DESTINATION REACHED";
        int fontSize = 22;
        int textX = (int)(banner.x + (bannerW - MeasureText(msg, fontSize)) / 2) + 10;
        int textY = (int)(banner.y + (bannerH - fontSize) / 2 + 4);

        DrawText(msg, textX, textY, fontSize, WHITE);

        // Small "Success" icon (Heart/Circle)
        DrawCircleV((Vector2){banner.x + 30, banner.y + bannerH/2 + 4}, 10, MM_HEART);
    }

    // 5. User Interface
    Rectangle btn = {SCREEN_W - 116, SCREEN_H - 50, 100, 36};
    DrawRectangleRounded(btn, 0.45f, 6, ctx->playing ? MM_BTN_STOP : MM_BTN_PLAY);
    DrawText(ctx->playing ? "Stop" : "Play", SCREEN_W - 91, SCREEN_H - 40, 16, WHITE);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn)) ctx->playing = !ctx->playing;
    // 5.5 Draw "Destination Reached" Message


    EndDrawing(); return true;
}

void FreeRenderer(RenderCtx* ctx) { if (ctx) free(ctx); }
