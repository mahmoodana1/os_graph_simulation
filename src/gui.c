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
void startGui(Graph* g, int src, int dst)
{
    int path[64];

    /* 1. Calculate the path internally to keep main.c clean */
    int path_len = BuildDijkstraPath(g, src, dst, path);

    if (path_len <= 0)
    {
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

/* Draws a weight marker badge at an explicit midpoint position */
void DrawWeightBadge(Vector2 mid, int w, bool on_path)
{
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", w);
    int fontSize = 15;
    int tw = MeasureText(buf, fontSize);

    float padX = 6.0f, padY = 3.0f;
    float bw = (float)tw + padX * 2.0f;
    float bh = (float)fontSize + padY * 2.0f;
    Rectangle badge = {mid.x - bw * 0.5f, mid.y - bh * 0.5f, bw, bh};

    Color badgeColor = on_path ? MM_HEART : (Color){45, 52, 70, 220};

    DrawRectangleRounded((Rectangle){badge.x + 2, badge.y + 2, bw, bh}, 0.6f, 8, (Color){0, 0, 0, 70});
    DrawRectangleRounded((Rectangle){badge.x - 1.5f, badge.y - 1.5f, bw + 3.0f, bh + 3.0f}, 0.6f, 8,
                         (Color){255, 255, 255, 200});
    DrawRectangleRounded(badge, 0.6f, 8, badgeColor);
    DrawText(buf, (int)(mid.x - tw / 2), (int)(mid.y - fontSize / 2), fontSize, WHITE);
}


/* Internal Helper: Cubic Bezier point calculation */
static Vector2 GetBezierPoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t)
{
    float invT = 1.0f - t;
    return (Vector2){
        p0.x * (invT * invT * invT) + p1.x * (3 * invT * invT * t) + p2.x * (3 * invT * t * t) + p3.x * (t * t * t),
        p0.y * (invT * invT * invT) + p1.y * (3 * invT * invT * t) + p2.y * (3 * invT * t * t) + p3.y * (t * t * t)
    };
}

/* Compute cubic bezier tangent (derivative) at t */
static Vector2 GetBezierTangent(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t)
{
    float u = 1.0f - t;
    return (Vector2){
        3.0f * (u * u * (p1.x - p0.x) + 2.0f * u * t * (p2.x - p1.x) + t * t * (p3.x - p2.x)),
        3.0f * (u * u * (p1.y - p0.y) + 2.0f * u * t * (p2.y - p1.y) + t * t * (p3.y - p2.y))
    };
}

/* Compute control points for a road edge — curves left relative to travel direction */
static void GetEdgeBezier(Vector2 from, Vector2 to, Vector2* c1, Vector2* c2)
{
    Vector2 dir = Vector2Subtract(to, from);
    float len = Vector2Length(dir);
    if (len < 1.0f)
    {
        *c1 = from;
        *c2 = to;
        return;
    }
    Vector2 perp = {-dir.y / len, dir.x / len};
    float curve = fminf(len * 0.20f, 40.0f);
    *c1 = (Vector2){
        from.x + dir.x * 0.33f + perp.x * curve,
        from.y + dir.y * 0.33f + perp.y * curve
    };
    *c2 = (Vector2){
        to.x - dir.x * 0.33f + perp.x * curve,
        to.y - dir.y * 0.33f + perp.y * curve
    };
}

/* Draw a weight sign beside the road — offset perpendicularly with a thin post */
static void DrawRoadSign(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3,
                         float t, int w, bool on_path)
{
    Vector2 roadPt = GetBezierPoint(p0, c1, c2, p3, t);
    Vector2 tang = GetBezierTangent(p0, c1, c2, p3, t);
    float tLen = Vector2Length(tang);
    if (tLen < 0.1f) return;

    /* Right-perpendicular to travel direction */
    Vector2 rperp = {tang.y / tLen, -tang.x / tLen};
    float dist = 24.0f;
    Vector2 signPt = {roadPt.x + rperp.x * dist, roadPt.y + rperp.y * dist};

    /* Thin post from road edge to sign base */
    Vector2 postBase = {
        roadPt.x + rperp.x * (ROAD_THICK * 0.5f + 3.0f),
        roadPt.y + rperp.y * (ROAD_THICK * 0.5f + 3.0f)
    };
    DrawLineEx(postBase, signPt, 1.5f, (Color){50, 50, 55, 210});

    DrawWeightBadge(signPt, w, on_path);
}

/* Internal Helper: Draw curved river branches with floating labels */
static void DrawRiverBranch(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p1, float thick)
{
    Vector2 pts[4] = {p0, c1, c2, p1};
    DrawSplineBezierCubic(pts, 4, thick, MM_RIVER);

    for (float t = 0.3f; t <= 0.7f; t += 0.4f)
    {
        Vector2 pos = GetBezierPoint(p0, c1, c2, p1, t);
        Vector2 next = GetBezierPoint(p0, c1, c2, p1, t + 0.01f);
        float angle = atan2f(next.y - pos.y, next.x - pos.x) * (180.0f / PI);
        DrawTextPro(GetFontDefault(), "ISAR", pos, (Vector2){0, 0}, angle, 12.0f, 2.0f, MM_RIVER_LABEL);
    }
}

/* Internal Helper: Draw directional arrows along a bezier road */
static void DrawRoadArrow(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3, Color color)
{
    static const float ts[2] = {0.25f, 0.75f};
    for (int i = 0; i < 2; i++)
    {
        Vector2 tang = GetBezierTangent(p0, c1, c2, p3, ts[i]);
        if (Vector2Length(tang) < 1.0f) continue;
        Vector2 pos = GetBezierPoint(p0, c1, c2, p3, ts[i]);
        float angle = atan2f(tang.y, tang.x) * (180.0f / PI);
        DrawPoly(pos, 3, 8.0f, angle, color);
    }
}

/* Internal Helper: Draw node tile with shadows and decorations */
static void DrawNodeTile(RenderCtx* ctx, int idx)
{
    Vector2 p = ctx->positions[idx];
    float yardSize = NODE_SIZE * 1.5f;

    // 1. Drop shadow
    DrawRectangleRounded((Rectangle){p.x - yardSize / 2 + 4, p.y - yardSize / 2 + 4, yardSize, yardSize}, 0.3f, 10,
                         MM_SHADOW);
    // 2. Yard area
    DrawRectangleRounded((Rectangle){p.x - yardSize / 2, p.y - yardSize / 2, yardSize, yardSize}, 0.3f, 10, MM_YARD);
    // 3. Main building tile
    Rectangle r = {p.x - NODE_SIZE / 2, p.y - NODE_SIZE / 2, NODE_SIZE, NODE_SIZE};
    DrawRectangleRounded(r, 0.25f, 8, MM_NODE_BG);

    Color acc = (idx == ctx->src) ? MM_BTN_STOP : (idx == ctx->dst) ? MM_BTN_PLAY : ctx->accents[idx % 4];
    DrawRectangleRounded((Rectangle){r.x, r.y, NODE_SIZE, NODE_SIZE * 0.5f}, 0.25f, 6, acc);

    // 4. Windows/Eyes decoration
    if (idx % 3 == 0)
    {
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 1.5f, BLACK);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 1.5f, BLACK);
    }
    else
    {
        DrawCircleV((Vector2){p.x, p.y - 6}, 4, WHITE);
        DrawCircleV((Vector2){p.x, p.y - 6}, 2, BLACK);
    }

    // 5. Node ID Label
    char buf[8];
    sprintf(buf, "%d", idx);
    DrawText(buf, (int)(p.x - MeasureText(buf, 16) / 2), (int)(p.y + 6), 16, MM_TEXT_DARK);
}

/* Update simulation logic: Car movement and state transitions */
void UpdateCar(Car* car, RenderCtx* ctx, Graph* g, float dt)
{
    car->timer += dt;
    if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT)
    {
        car->x = ctx->positions[car->path[car->seg]].x;
        car->y = ctx->positions[car->path[car->seg]].y;
        float wait = (car->seg == 0) ? 0.0f : NODE_WAIT_SEC;
        if (car->timer >= wait)
        {
            if (car->seg + 1 >= car->path_len)
            {
                car->state = CAR_ARRIVED;
                return;
            }
            car->timer = 0;
            car->hop = 0;
            Node* e = g->adj[car->path[car->seg]];
            while (e)
            {
                if (e->id == car->path[car->seg + 1])
                {
                    car->total_hops = e->weight;
                    break;
                }
                e = e->next;
            }
            car->state = CAR_MOVING;
        }
    }
    else if (car->state == CAR_MOVING)
    {
        Vector2 s = ctx->positions[car->path[car->seg]], d = ctx->positions[car->path[car->seg + 1]];
        Vector2 c1, c2;
        GetEdgeBezier(s, d, &c1, &c2);
        float denom = (float)(car->total_hops > 0 ? car->total_hops : 1);
        float t = (car->hop + fminf(car->timer / HOP_DURATION_SEC, 1.0f)) / denom;
        Vector2 pos = GetBezierPoint(s, c1, c2, d, t);
        car->x = pos.x;
        car->y = pos.y;
        if (car->timer >= HOP_DURATION_SEC)
        {
            car->timer = 0;
            car->hop++;
            if (car->hop >= car->total_hops)
            {
                car->seg++;
                car->state = CAR_NODE_WAIT;
            }
        }
    }
}

/* Initialization: Prepare positions and car initial state */
RenderCtx* InitRenderer(Graph* g, int src, int dst, int* path, int path_len)
{
    RenderCtx* ctx = calloc(1, sizeof(RenderCtx));
    if (!ctx) return NULL;

    int numOfCars = ctx->carCount;
    if (numOfCars > 0)
    {
        ctx->cars = calloc(numOfCars, sizeof(Car));
        if (!ctx->cars)
        {
            free(ctx);
            return NULL;
        }
    }

    ctx->src = src;
    ctx->dst = dst;
    ctx->dijk_len = path_len;
    memcpy(ctx->dijk_path, path, path_len * sizeof(int));
    for (int i = 0; i < g->num_nodes; i++)
    {
        float a = (float)i / g->num_nodes * 2.0f * PI - 1.57f;
        ctx->positions[i] = (Vector2){SCREEN_W / 2 + 210 * cosf(a), SCREEN_H / 2 + 210 * sinf(a)};
        ctx->accents[i] = NODE_ACCENTS[i % 4];
    }

    int num_available_colors = 10;
    for (int i = 0; i < numOfCars; i++)
    {
        ctx->cars[i].color = distinct_colors[i % num_available_colors];
        ctx->cars[i].state = CAR_IDLE;
    }

    memcpy(ctx->car.path, path, path_len * sizeof(int));
    ctx->car.path_len = path_len;
    ctx->car.state = CAR_IDLE;
    return ctx;
}

/* Main Render Loop: Coordinates background, roads, nodes, and UI */
bool RenderFrame(RenderCtx* ctx, Graph* g, float dt)
{
    if (WindowShouldClose()) return false;

    /* Update animation logic */
    if (ctx->playing && ctx->car.state != CAR_ARRIVED)
    {
        UpdateCar(&ctx->car, ctx, g, dt);
    }

    BeginDrawing();
    ClearBackground(MM_BG);

    // 1. Background Environment (Rivers)
    DrawRiverBranch((Vector2){-100, 450}, (Vector2){300, 650}, (Vector2){500, 250}, (Vector2){1000, 350}, 30.0f);
    DrawRiverBranch((Vector2){300, -50}, (Vector2){200, 250}, (Vector2){450, 400}, (Vector2){700, 800}, 20.0f);

    // 2. Roads Infrastructure (Regular Roads)
    for (int i = 0; i < g->num_nodes; i++)
    {
        Node* e = g->adj[i];
        while (e)
        {
            bool on = false;
            for (int k = 0; k + 1 < ctx->dijk_len; k++)
            {
                if (ctx->dijk_path[k] == i && ctx->dijk_path[k + 1] == e->id)
                {
                    on = true;
                    break;
                }
            }
            if (!on)
            {
                Vector2 ps = ctx->positions[i], pd = ctx->positions[e->id];
                Vector2 rc1, rc2;
                GetEdgeBezier(ps, pd, &rc1, &rc2);
                Vector2 rpts[4] = {ps, rc1, rc2, pd};
                DrawSplineBezierCubic(rpts, 4, ROAD_THICK + 4.0f, (Color){0, 0, 0, 50});
                DrawSplineBezierCubic(rpts, 4, ROAD_THICK, MM_ROAD);
                DrawRoadArrow(ps, rc1, rc2, pd, (Color){200, 200, 200, 150});
                DrawRoadSign(ps, rc1, rc2, pd, 0.5f, e->weight, false);
            }
            e = e->next;
        }
    }

    // 3. Highlighted Shortest Path
    for (int i = 0; i + 1 < ctx->dijk_len; i++)
    {
        int u = ctx->dijk_path[i], v = ctx->dijk_path[i + 1];
        Vector2 s = ctx->positions[u], d = ctx->positions[v];

        Vector2 pc1, pc2;
        GetEdgeBezier(s, d, &pc1, &pc2);
        Vector2 ppts[4] = {s, pc1, pc2, d};
        DrawSplineBezierCubic(ppts, 4, ROAD_THICK + 10.0f, (Color){0, 0, 0, 50});
        DrawSplineBezierCubic(ppts, 4, ROAD_THICK + 6.0f, MM_ROAD_PATH);
        DrawRoadArrow(s, pc1, pc2, d, MM_YARD);

        int weight = 1;
        Node* tmp = g->adj[u];
        while (tmp)
        {
            if (tmp->id == v)
            {
                weight = tmp->weight;
                break;
            }
            tmp = tmp->next;
        }
        DrawRoadSign(s, pc1, pc2, d, 0.5f, weight, true);
    }

    // 4. Tiles and Entities (Nodes and Car)
    for (int i = 0; i < g->num_nodes; i++) DrawNodeTile(ctx, i);

    if ((ctx->car.state == CAR_MOVING || ctx->car.state == CAR_NODE_WAIT) &&
        ctx->car.seg + 1 < ctx->car.path_len)
    {
        Vector2 from = ctx->positions[ctx->car.path[ctx->car.seg]];
        Vector2 to = ctx->positions[ctx->car.path[ctx->car.seg + 1]];
        Vector2 cc1, cc2;
        GetEdgeBezier(from, to, &cc1, &cc2);
        float denom = (float)(ctx->car.total_hops > 0 ? ctx->car.total_hops : 1);
        float t = (ctx->car.state == CAR_MOVING)
                      ? fmaxf(0.0f, fminf((ctx->car.hop + fminf(ctx->car.timer / HOP_DURATION_SEC, 1.0f)) / denom,
                                          1.0f))
                      : 0.0f;
        Vector2 tang = GetBezierTangent(from, cc1, cc2, to, t);
        float angle = (Vector2Length(tang) > 0.1f) ? atan2f(tang.y, tang.x) * (180.0f / PI) : 0.0f;
        DrawRectanglePro((Rectangle){ctx->car.x + 4, ctx->car.y + 4, 22, 12}, (Vector2){11, 6}, angle,
                         (Color){0, 0, 0, 70});
        DrawRectanglePro((Rectangle){ctx->car.x, ctx->car.y, 22, 12}, (Vector2){11, 6}, angle,
                         (Color){50, 120, 220, 255});
    }

    // 5. Destination Reached Banner + Restart
    if (ctx->car.state == CAR_ARRIVED)
    {
        float bW = 400.0f, bH = 130.0f;
        Rectangle banner = {(SCREEN_W - bW) * 0.5f, (SCREEN_H - bH) * 0.5f - 30.0f, bW, bH};

        // Drop shadow
        DrawRectangleRounded((Rectangle){banner.x + 3, banner.y + 3, bW, bH}, 0.3f, 10, (Color){0, 0, 0, 90});
        // Panel
        DrawRectangleRounded(banner, 0.3f, 10, MM_ROAD);
        // Left teal accent bar
        DrawRectangleRounded((Rectangle){banner.x, banner.y, 6, bH}, 0.3f, 6, MM_BTN_PLAY);
        // Divider line
        DrawLineEx((Vector2){banner.x + 16, banner.y + 56}, (Vector2){banner.x + bW - 16, banner.y + 56}, 1.0f,
                   (Color){255, 255, 255, 35});

        // Check circle + hand-drawn checkmark
        Vector2 cc = {banner.x + 38.0f, banner.y + 30.0f};
        DrawCircleV(cc, 14, MM_BTN_PLAY);
        DrawLineEx((Vector2){cc.x - 6, cc.y + 1}, (Vector2){cc.x - 1, cc.y + 6}, 2.5f, WHITE);
        DrawLineEx((Vector2){cc.x - 1, cc.y + 6}, (Vector2){cc.x + 7, cc.y - 4}, 2.5f, WHITE);

        // Title
        const char* title = "DESTINATION REACHED";
        DrawText(title, (int)(banner.x + 60), (int)(banner.y + 14), 22, WHITE);

        // Subtitle
        const char* sub = "Shortest path found successfully.";
        DrawText(sub, (int)(banner.x + 60), (int)(banner.y + 40), 13, (Color){190, 200, 210, 200});

        // Restart button
        float rbW = 150.0f, rbH = 36.0f;
        Rectangle restartBtn = {
            banner.x + (bW - rbW) * 0.5f,
            banner.y + bH - rbH - 14.0f,
            rbW, rbH
        };
        Color rbCol = (Color){210, 130, 35, 255};
        DrawRectangleRounded((Rectangle){restartBtn.x + 2, restartBtn.y + 2, rbW, rbH}, 0.45f, 8, (Color){0, 0, 0, 60});
        DrawRectangleRounded(restartBtn, 0.45f, 8, rbCol);
        const char* rbText = "Restart";
        DrawText(rbText,
                 (int)(restartBtn.x + (rbW - MeasureText(rbText, 16)) * 0.5f),
                 (int)(restartBtn.y + (rbH - 16) * 0.5f),
                 16, WHITE);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), restartBtn))
        {
            ctx->car.state = CAR_IDLE;
            ctx->car.seg = 0;
            ctx->car.hop = 0;
            ctx->car.timer = 0.0f;
            ctx->car.total_hops = 0;
            ctx->playing = false;
        }
    }

    // 6. User Interface (Buttons)
    Rectangle btn = {SCREEN_W - 116, SCREEN_H - 50, 100, 36};
    DrawRectangleRounded(btn, 0.45f, 6, ctx->playing ? MM_BTN_STOP : MM_BTN_PLAY);

    const char* btnText = ctx->playing ? "Stop" : "Play";
    DrawText(btnText, (int)(btn.x + (100 - MeasureText(btnText, 16)) / 2), (int)(btn.y + 10), 16, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn))
    {
        ctx->playing = !ctx->playing;
    }

    EndDrawing();
    return true;
}

void FreeRenderer(RenderCtx* ctx)
{
    free(ctx->cars);
    free(ctx);
}
