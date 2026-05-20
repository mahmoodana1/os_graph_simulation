#include "../include/gui.h"
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const Color NODE_ACCENTS[] = {{210, 80, 70, 255},
                                     {60, 155, 155, 255},
                                     {50, 65, 90, 255},
                                     {200, 150, 60, 255}};

void startGui(Graph *g, int paths[][64], int *path_lens, int num_travelers, pid_t *pids) {
    InitWindow(SCREEN_W, SCREEN_H, "OS Graph Simulation - Traffic Flow");
    SetTargetFPS(60);

    RenderCtx *ctx = InitRenderer(g, num_travelers);
    for (int i = 0; i < num_travelers; i++) {
        memcpy(ctx->cars[i].path, paths[i], path_lens[i] * sizeof(int));
        ctx->cars[i].path_len = path_lens[i];
    }

    bool signaled[num_travelers];
    for (int i = 0; i < num_travelers; i++) signaled[i] = false;

    while (RenderFrame(ctx, g, GetFrameTime())) {
        for (int i = 0; i < num_travelers; i++) {
            if (!signaled[i] && ctx->cars[i].state == CAR_ARRIVED) {
                kill(pids[i], SIGTERM);
                signaled[i] = true;
            }
        }
    }

    FreeRenderer(ctx);
    CloseWindow();
}

void DrawWeightBadge(Vector2 mid, int w, bool on_path) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", w);
    int fontSize = 15;
    int tw = MeasureText(buf, fontSize);

    float padX = 6.0f, padY = 3.0f;
    float bw = (float)tw + padX * 2.0f;
    float bh = (float)fontSize + padY * 2.0f;
    Rectangle badge = {mid.x - bw * 0.5f, mid.y - bh * 0.5f, bw, bh};

    Color badgeColor = on_path ? MM_HEART : (Color){45, 52, 70, 220};

    DrawRectangleRounded((Rectangle){badge.x + 2, badge.y + 2, bw, bh}, 0.6f, 8,
                         (Color){0, 0, 0, 70});
    DrawRectangleRounded(
        (Rectangle){badge.x - 1.5f, badge.y - 1.5f, bw + 3.0f, bh + 3.0f}, 0.6f,
        8, (Color){255, 255, 255, 200});
    DrawRectangleRounded(badge, 0.6f, 8, badgeColor);
    DrawText(buf, (int)(mid.x - tw / 2), (int)(mid.y - fontSize / 2), fontSize,
             WHITE);
}

static Vector2 GetBezierPoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3,
                              float t) {
    float invT = 1.0f - t;
    return (Vector2){
        p0.x * (invT * invT * invT) + p1.x * (3 * invT * invT * t) +
            p2.x * (3 * invT * t * t) + p3.x * (t * t * t),
        p0.y * (invT * invT * invT) + p1.y * (3 * invT * invT * t) +
            p2.y * (3 * invT * t * t) + p3.y * (t * t * t)};
}

static Vector2 GetBezierTangent(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3,
                                float t) {
    float u = 1.0f - t;
    return (Vector2){
        3.0f * (u * u * (p1.x - p0.x) + 2.0f * u * t * (p2.x - p1.x) +
                t * t * (p3.x - p2.x)),
        3.0f * (u * u * (p1.y - p0.y) + 2.0f * u * t * (p2.y - p1.y) +
                t * t * (p3.y - p2.y))};
}

static void GetEdgeBezier(Vector2 from, Vector2 to, Vector2 *c1, Vector2 *c2) {
    Vector2 dir = Vector2Subtract(to, from);
    float len = Vector2Length(dir);
    if (len < 1.0f) {
        *c1 = from;
        *c2 = to;
        return;
    }
    Vector2 perp = {-dir.y / len, dir.x / len};
    float curve = fminf(len * 0.20f, 40.0f);
    *c1 = (Vector2){from.x + dir.x * 0.33f + perp.x * curve,
                    from.y + dir.y * 0.33f + perp.y * curve};
    *c2 = (Vector2){to.x - dir.x * 0.33f + perp.x * curve,
                    to.y - dir.y * 0.33f + perp.y * curve};
}

static void DrawRoadSign(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3,
                         float t, int w, bool on_path) {
    Vector2 roadPt = GetBezierPoint(p0, c1, c2, p3, t);
    Vector2 tang = GetBezierTangent(p0, c1, c2, p3, t);
    float tLen = Vector2Length(tang);
    if (tLen < 0.1f)
        return;
    Vector2 rperp = {tang.y / tLen, -tang.x / tLen};
    Vector2 signPt = {roadPt.x + rperp.x * 24.0f, roadPt.y + rperp.y * 24.0f};
    Vector2 postBase = {roadPt.x + rperp.x * (ROAD_THICK * 0.5f + 3.0f),
                        roadPt.y + rperp.y * (ROAD_THICK * 0.5f + 3.0f)};
    DrawLineEx(postBase, signPt, 1.5f, (Color){50, 50, 55, 210});
    DrawWeightBadge(signPt, w, on_path);
}

static void DrawRiverBranch(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p1,
                            float thick) {
    Vector2 pts[4] = {p0, c1, c2, p1};
    DrawSplineBezierCubic(pts, 4, thick, MM_RIVER);
    for (float t = 0.3f; t <= 0.7f; t += 0.4f) {
        Vector2 pos = GetBezierPoint(p0, c1, c2, p1, t);
        Vector2 next = GetBezierPoint(p0, c1, c2, p1, t + 0.01f);
        float angle = atan2f(next.y - pos.y, next.x - pos.x) * (180.0f / PI);
        DrawTextPro(GetFontDefault(), "ISAR", pos, (Vector2){0, 0}, angle,
                    12.0f, 2.0f, MM_RIVER_LABEL);
    }
}

static void DrawRoadArrow(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3,
                          Color color) {
    static const float ts[2] = {0.25f, 0.75f};
    for (int i = 0; i < 2; i++) {
        Vector2 tang = GetBezierTangent(p0, c1, c2, p3, ts[i]);
        if (Vector2Length(tang) < 1.0f)
            continue;
        Vector2 pos = GetBezierPoint(p0, c1, c2, p3, ts[i]);
        float angle = atan2f(tang.y, tang.x) * (180.0f / PI);
        DrawPoly(pos, 3, 8.0f, angle, color);
    }
}

static void DrawNodeTile(RenderCtx *ctx, int idx) {
    Vector2 p = ctx->positions[idx];
    float yardSize = NODE_SIZE * 1.5f;

    DrawRectangleRounded((Rectangle){p.x - yardSize / 2 + 4,
                                     p.y - yardSize / 2 + 4, yardSize,
                                     yardSize},
                         0.3f, 10, MM_SHADOW);
    DrawRectangleRounded(
        (Rectangle){p.x - yardSize / 2, p.y - yardSize / 2, yardSize, yardSize},
        0.3f, 10, MM_YARD);

    Rectangle r = {p.x - NODE_SIZE / 2, p.y - NODE_SIZE / 2, NODE_SIZE,
                   NODE_SIZE};
    DrawRectangleRounded(r, 0.25f, 8, MM_NODE_BG);

    Color acc = ctx->accents[idx % 4];
    DrawRectangleRounded((Rectangle){r.x, r.y, NODE_SIZE, NODE_SIZE * 0.5f},
                         0.25f, 6, acc);

    if (idx % 3 == 0) {
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 3.5f, WHITE);
        DrawCircleV((Vector2){p.x - 5, p.y - 6}, 1.5f, BLACK);
        DrawCircleV((Vector2){p.x + 5, p.y - 6}, 1.5f, BLACK);
    } else {
        DrawCircleV((Vector2){p.x, p.y - 6}, 4, WHITE);
        DrawCircleV((Vector2){p.x, p.y - 6}, 2, BLACK);
    }

    char buf[8];
    sprintf(buf, "%d", idx);
    DrawText(buf, (int)(p.x - MeasureText(buf, 16) / 2), (int)(p.y + 6), 16,
             MM_TEXT_DARK);
}

void UpdateCar(Car *car, RenderCtx *ctx, Graph *g, float dt) {
    if (car->path_len <= 0) {
        car->state = CAR_ARRIVED;
        return;
    }

    car->timer += dt;
    if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT) {
        car->x = ctx->positions[car->path[car->seg]].x;
        car->y = ctx->positions[car->path[car->seg]].y;
        float wait = (car->seg == 0) ? 0.0f : NODE_WAIT_SEC;
        if (car->timer >= wait) {
            if (car->seg + 1 >= car->path_len) {
                car->state = CAR_ARRIVED;
                return;
            }
            car->timer = 0;
            car->hop = 0;
            Node *e = g->adj[car->path[car->seg]];
            while (e) {
                if (e->id == car->path[car->seg + 1]) {
                    car->total_hops = e->weight;
                    break;
                }
                e = e->next;
            }
            car->state = CAR_MOVING;
        }
    } else if (car->state == CAR_MOVING) {
        Vector2 s = ctx->positions[car->path[car->seg]];
        Vector2 d = ctx->positions[car->path[car->seg + 1]];
        Vector2 c1, c2;
        GetEdgeBezier(s, d, &c1, &c2);
        float denom = (float)(car->total_hops > 0 ? car->total_hops : 1);
        float t =
            (car->hop + fminf(car->timer / HOP_DURATION_SEC, 1.0f)) / denom;
        Vector2 pos = GetBezierPoint(s, c1, c2, d, t);
        car->x = pos.x;
        car->y = pos.y;
        if (car->timer >= HOP_DURATION_SEC) {
            car->timer = 0;
            car->hop++;
            if (car->hop >= car->total_hops) {
                car->seg++;
                car->state = CAR_NODE_WAIT;
            }
        }
    }
}

RenderCtx *InitRenderer(Graph *g, int num_cars) {
    RenderCtx *ctx = calloc(1, sizeof(RenderCtx));
    ctx->node_count = g->num_nodes;
    ctx->numCars = num_cars;
    ctx->playing = false;

    ctx->cars = malloc(num_cars * sizeof(Car));
    if (!ctx->cars) {
        fprintf(stderr, "Error: malloc failed for cars\n");
        exit(1);
    }

    for (int i = 0; i < g->num_nodes; i++) {
        float a = (float)i / g->num_nodes * 2.0f * PI - 1.57f;
        ctx->positions[i] = (Vector2){SCREEN_W / 2 + 210 * cosf(a),
                                      SCREEN_H / 2 + 210 * sinf(a)};
        ctx->accents[i] = NODE_ACCENTS[i % 4];
    }

    Color traveler_colors[] = {{0, 102, 254, 255}, {255, 0, 127, 255},
                               {255, 102, 0, 255}, {128, 0, 255, 255},
                               {0, 220, 220, 255}, {50, 205, 50, 255}};

    for (int i = 0; i < num_cars; i++) {
        ctx->cars[i] = (Car){0};
        ctx->cars[i].state = CAR_IDLE;
        ctx->cars[i].color = traveler_colors[i % 6];
    }

    return ctx;
}

bool RenderFrame(RenderCtx *ctx, Graph *g, float dt) {
    if (WindowShouldClose())
        return false;

    if (ctx->playing) {
        for (int i = 0; i < ctx->numCars; i++)
            if (ctx->cars[i].state != CAR_ARRIVED)
                UpdateCar(&ctx->cars[i], ctx, g, dt);
    }

    BeginDrawing();
    ClearBackground(MM_BG);

    DrawRiverBranch((Vector2){-100, 450}, (Vector2){300, 650},
                    (Vector2){500, 250}, (Vector2){1000, 350}, 30.0f);
    DrawRiverBranch((Vector2){300, -50}, (Vector2){200, 250},
                    (Vector2){450, 400}, (Vector2){700, 800}, 20.0f);

    for (int i = 0; i < g->num_nodes; i++) {
        Node *e = g->adj[i];
        while (e) {
            Vector2 ps = ctx->positions[i], pd = ctx->positions[e->id];
            Vector2 rc1, rc2;
            GetEdgeBezier(ps, pd, &rc1, &rc2);
            Vector2 rpts[4] = {ps, rc1, rc2, pd};
            DrawSplineBezierCubic(rpts, 4, ROAD_THICK + 4.0f,
                                  (Color){0, 0, 0, 50});
            DrawSplineBezierCubic(rpts, 4, ROAD_THICK, WHITE);
            DrawRoadArrow(ps, rc1, rc2, pd, (Color){15, 32, 67, 255});
            DrawRoadSign(ps, rc1, rc2, pd, 0.5f, e->weight, false);
            e = e->next;
        }
    }

    for (int i = 0; i < g->num_nodes; i++)
        DrawNodeTile(ctx, i);

    for (int i = 0; i < ctx->numCars; i++) {
        Car *car = &ctx->cars[i];
        if ((car->state == CAR_MOVING || car->state == CAR_NODE_WAIT) &&
            car->seg + 1 < car->path_len) {
            Vector2 from = ctx->positions[car->path[car->seg]];
            Vector2 to = ctx->positions[car->path[car->seg + 1]];
            Vector2 cc1, cc2;
            GetEdgeBezier(from, to, &cc1, &cc2);
            float denom = (float)(car->total_hops > 0 ? car->total_hops : 1);
            float t =
                (car->state == CAR_MOVING)
                    ? fmaxf(0.0f,
                            fminf((car->hop +
                                   fminf(car->timer / HOP_DURATION_SEC, 1.0f)) /
                                      denom,
                                  1.0f))
                    : 0.0f;
            Vector2 tang = GetBezierTangent(from, cc1, cc2, to, t);
            float angle = (Vector2Length(tang) > 0.1f)
                              ? atan2f(tang.y, tang.x) * (180.0f / PI)
                              : 0.0f;
            DrawRectanglePro((Rectangle){car->x + 4, car->y + 4, 22, 12},
                             (Vector2){11, 6}, angle, (Color){0, 0, 0, 70});
            DrawRectanglePro((Rectangle){car->x, car->y, 22, 12},
                             (Vector2){11, 6}, angle, car->color);
        }
    }

    bool all_arrived = (ctx->numCars > 0);
    for (int i = 0; i < ctx->numCars; i++)
        if (ctx->cars[i].state != CAR_ARRIVED) {
            all_arrived = false;
            break;
        }

    if (all_arrived) {
        float bW = 440.0f, bH = 130.0f;
        Rectangle banner = {(SCREEN_W - bW) * 0.5f,
                            (SCREEN_H - bH) * 0.5f - 30.0f, bW, bH};
        DrawRectangleRounded((Rectangle){banner.x + 3, banner.y + 3, bW, bH},
                             0.3f, 10, (Color){0, 0, 0, 90});
        DrawRectangleRounded(banner, 0.3f, 10, MM_ROAD);
        DrawRectangleRounded((Rectangle){banner.x, banner.y, 6, bH}, 0.3f, 6,
                             MM_BTN_PLAY);
        DrawLineEx((Vector2){banner.x + 16, banner.y + 56},
                   (Vector2){banner.x + bW - 16, banner.y + 56}, 1.0f,
                   (Color){255, 255, 255, 35});
        Vector2 cc = {banner.x + 38.0f, banner.y + 30.0f};
        DrawCircleV(cc, 14, MM_BTN_PLAY);
        DrawLineEx((Vector2){cc.x - 6, cc.y + 1}, (Vector2){cc.x - 1, cc.y + 6},
                   2.5f, WHITE);
        DrawLineEx((Vector2){cc.x - 1, cc.y + 6}, (Vector2){cc.x + 7, cc.y - 4},
                   2.5f, WHITE);
        DrawText("ALL TRAVELERS ARRIVED", (int)(banner.x + 60),
                 (int)(banner.y + 14), 20, WHITE);
        DrawText("All shortest paths completed.", (int)(banner.x + 60),
                 (int)(banner.y + 40), 13, (Color){190, 200, 210, 200});
        float rbW = 150.0f, rbH = 36.0f;
        Rectangle restartBtn = {banner.x + (bW - rbW) * 0.5f,
                                banner.y + bH - rbH - 14.0f, rbW, rbH};
        DrawRectangleRounded(
            (Rectangle){restartBtn.x + 2, restartBtn.y + 2, rbW, rbH}, 0.45f, 8,
            (Color){0, 0, 0, 60});
        DrawRectangleRounded(restartBtn, 0.45f, 8, (Color){210, 130, 35, 255});
        const char *rbText = "Restart";
        DrawText(rbText,
                 (int)(restartBtn.x + (rbW - MeasureText(rbText, 16)) * 0.5f),
                 (int)(restartBtn.y + (rbH - 16) * 0.5f), 16, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(GetMousePosition(), restartBtn)) {
            for (int i = 0; i < ctx->numCars; i++) {
                ctx->cars[i].state = CAR_IDLE;
                ctx->cars[i].seg = 0;
                ctx->cars[i].hop = 0;
                ctx->cars[i].timer = 0.0f;
                ctx->cars[i].total_hops = 0;
            }
            ctx->playing = false;
        }
    }

    Rectangle btn = {SCREEN_W - 116, SCREEN_H - 50, 100, 36};
    DrawRectangleRounded(btn, 0.45f, 6,
                         ctx->playing ? MM_BTN_STOP : MM_BTN_PLAY);
    const char *btnText = ctx->playing ? "Stop" : "Play";
    DrawText(btnText, (int)(btn.x + (100 - MeasureText(btnText, 16)) / 2),
             (int)(btn.y + 10), 16, WHITE);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), btn))
        ctx->playing = !ctx->playing;

    EndDrawing();
    return true;
}

void FreeRenderer(RenderCtx *ctx) {
    if (ctx) {
        free(ctx->cars);
        free(ctx);
    }
}
