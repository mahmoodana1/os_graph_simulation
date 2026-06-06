#include "../include/gui.h"
#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define WIN_W GUI_WIN_W
#define WIN_H GUI_WIN_H
#define GRAPH_W 960
#define PANEL_X 960
#define PANEL_W 320
#define NODE_SZ 42.0f
#define CAR_SZ 7.5f
#define BTN_H 38
#define BTN_MX 14

/* ── Palette ───────────────────────────────────────────────────────────── */
#define C_BG CLITERAL(Color){8, 12, 24, 255}
#define C_GRID CLITERAL(Color){20, 30, 55, 140}
#define C_VIGN CLITERAL(Color){0, 0, 0, 120}
#define C_TRANS CLITERAL(Color){0, 0, 0, 0}
#define C_ROAD_CASE CLITERAL(Color){4, 7, 16, 255}
#define C_ROAD_MID CLITERAL(Color){26, 50, 100, 255}
#define C_ROAD_CTR CLITERAL(Color){55, 105, 185, 255}
#define C_ARROW CLITERAL(Color){0, 155, 215, 90}
#define C_BADGE_BG CLITERAL(Color){12, 20, 42, 230}
#define C_BADGE_BD CLITERAL(Color){0, 155, 215, 130}
#define C_BADGE_TXT CLITERAL(Color){150, 205, 255, 255}
#define C_NODE_RING CLITERAL(Color){0, 175, 255, 170}
#define C_NODE_SHADOW CLITERAL(Color){0, 0, 0, 85}
#define C_NODE_ID CLITERAL(Color){195, 230, 255, 255}
#define C_PANEL_BG CLITERAL(Color){9, 14, 28, 245}
#define C_PANEL_SEP CLITERAL(Color){0, 135, 195, 75}
#define C_PANEL_TITLE CLITERAL(Color){0, 200, 255, 255}
#define C_PANEL_TXT CLITERAL(Color){150, 195, 240, 220}
#define C_BTN_IDLE CLITERAL(Color){16, 27, 55, 255}
#define C_BTN_HOVER CLITERAL(Color){0, 85, 155, 255}
#define C_BTN_PLAY CLITERAL(Color){0, 100, 60, 255}
#define C_BTN_PLAY_HOV CLITERAL(Color){0, 145, 80, 255}
#define C_BTN_BORD CLITERAL(Color){0, 145, 210, 160}
#define C_BTN_TXT CLITERAL(Color){210, 235, 255, 255}
#define C_SUCCESS_BG CLITERAL(Color){0, 25, 18, 210}
#define C_SUCCESS_TXT CLITERAL(Color){0, 230, 130, 255}
#define C_SUCCESS_LINE CLITERAL(Color){0, 200, 110, 255}
#define C_CAR_SHADOW CLITERAL(Color){0, 0, 0, 65}
#define C_TRAIL1 CLITERAL(Color){255, 255, 255, 55}
#define C_TRAIL2 CLITERAL(Color){255, 255, 255, 22}
#define C_FPS_TXT CLITERAL(Color){60, 90, 140, 200}

#define MM_HEART CLITERAL(Color){230, 50, 50, 255}

#define TOAST_LIFETIME 3.0f
#define TOAST_FADE_IN 0.25f
#define TOAST_FADE_OUT 0.50f
#define TOAST_W 238.0f
#define TOAST_H 50.0f
#define TOAST_GAP 8.0f

static float s_time = 0.0f;

/* ── Toast helpers ──────────────────────────────────────────────────────── */
static void FireToast(RenderCtx *ctx, const char *label, Color color) {
    int target = -1, oldest = 0;
    for (int i = 0; i < MAX_TOASTS; i++) {
        if (!ctx->toasts[i].active) {
            target = i;
            break;
        }
        if (ctx->toasts[i].timer < ctx->toasts[oldest].timer)
            oldest = i;
    }
    if (target == -1)
        target = oldest;
    strncpy(ctx->toasts[target].text, label, 31);
    ctx->toasts[target].text[31] = '\0';
    ctx->toasts[target].color = color;
    ctx->toasts[target].timer = TOAST_LIFETIME;
    ctx->toasts[target].active = true;
}

static void UpdateToasts(RenderCtx *ctx, float dt) {
    for (int i = 0; i < MAX_TOASTS; i++) {
        if (!ctx->toasts[i].active)
            continue;
        ctx->toasts[i].timer -= dt;
        if (ctx->toasts[i].timer <= 0.0f)
            ctx->toasts[i].active = false;
    }
}

static void DrawToasts(RenderCtx *ctx) {
    int slot = 0;
    for (int i = 0; i < MAX_TOASTS; i++) {
        if (!ctx->toasts[i].active)
            continue;
        Toast *t = &ctx->toasts[i];


        float elapsed = TOAST_LIFETIME - t->timer;
        float raw = 1.0f;
        if (elapsed < TOAST_FADE_IN)
            raw = elapsed / TOAST_FADE_IN;
        else if (t->timer < TOAST_FADE_OUT)
            raw = t->timer / TOAST_FADE_OUT;
        unsigned char a = (unsigned char) (raw * 255.0f);

        float tx = (float) GRAPH_W - TOAST_W - 14.0f;
        float ty = 14.0f + (float) slot * (TOAST_H + TOAST_GAP);
        slot++;

        /* drop shadow */
        DrawRectangleRounded((Rectangle){tx + 3, ty + 3, TOAST_W, TOAST_H},
                             0.28f, 8,
                             (Color){0, 0, 0, (unsigned char) (80 * raw)});
        /* colored border glow */
        Color bord = t->color;
        bord.a = a;
        DrawRectangleRounded(
            (Rectangle){tx - 1, ty - 1, TOAST_W + 2, TOAST_H + 2}, 0.28f, 8,
            bord);
        /* dark card background */
        DrawRectangleRounded((Rectangle){tx, ty, TOAST_W, TOAST_H}, 0.28f, 8,
                             (Color){6, 10, 22, (unsigned char) (230 * raw)});
        /* left accent bar in traveler color */
        Color accent = t->color;
        accent.a = a;
        DrawRectangle((int) tx, (int) (ty + 6), 4, (int) (TOAST_H - 12), accent);

        /* traveler name — line 1 */
        Color name_col = t->color;
        name_col.a = a;
        DrawText(t->text, (int) (tx + 14), (int) (ty + 9), 14, name_col);

        /* "Arrived!" subtitle — line 2 */
        DrawText("Arrived!", (int) (tx + 14), (int) (ty + 28), 11,
                 (Color){210, 230, 255, a});
    }
}

/* ── initGuiSetup ──────────────────────────────────────────────────────── */
RenderCtx *initGuiSetup(Graph *g, int num_travelers) {
    static const Color traveler_colors[] = {
        {0, 180, 255, 255}, {255, 80, 130, 255}, {255, 160, 30, 255},
        {140, 80, 255, 255}, {0, 220, 160, 255}, {255, 220, 50, 255},
        {60, 220, 80, 255}, {255, 120, 60, 255}, {120, 200, 255, 255}
    };

    Vector2 positions[MAX_NODES];
    float cx = GRAPH_W * 0.5f, cy = WIN_H * 0.5f, rad = 220.0f;
    for (int i = 0; i < g->num_nodes; i++) {
        float a = (float) i / g->num_nodes * 2.0f * PI - PI * 0.5f;
        positions[i] = (Vector2){cx + rad * cosf(a), cy + rad * sinf(a)};
    }

    InitWindow(GUI_WIN_W, GUI_WIN_H, "OS Graph Simulation");
    SetTargetFPS(60);
    RenderCtx *ctx = InitRenderer(g->num_nodes, positions, num_travelers);

    for (int i = 0; i < num_travelers; i++) {
        Car *c = &ctx->cars[i];
        c->id = i;
        c->color = traveler_colors[i % 9];
        c->path = NULL;
        c->path_len = 0;
        c->path_idx = 0;
        c->t = 0.0f;
        c->speed = 1.1f;
        c->state = CAR_IDLE;
        c->hop_mode = true;
        c->path_str[0] = '\0';
    }

    return ctx;
}

void DrawWeightBadge(Vector2 mid, int w, bool on_path) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", w);
    int fontSize = 15;
    int tw = MeasureText(buf, fontSize);

    float padX = 6.0f, padY = 3.0f;
    float bw = (float) tw + padX * 2.0f;
    float bh = (float) fontSize + padY * 2.0f;
    Rectangle badge = {mid.x - bw * 0.5f, mid.y - bh * 0.5f, bw, bh};

    Color badgeColor = on_path ? MM_HEART : (Color){45, 52, 70, 220};

    DrawRectangleRounded((Rectangle){badge.x + 2, badge.y + 2, bw, bh}, 0.6f, 8,
                         (Color){0, 0, 0, 70});
    DrawRectangleRounded(
        (Rectangle){badge.x - 1.5f, badge.y - 1.5f, bw + 3.0f, bh + 3.0f}, 0.6f,
        8, (Color){255, 255, 255, 200});
    DrawRectangleRounded(badge, 0.6f, 8, badgeColor);
    DrawText(buf, (int) (mid.x - tw / 2), (int) (mid.y - fontSize / 2), fontSize,
             WHITE);
}

/* ── Bezier helpers ──────────────────────────────────────────────────── */
static inline Vector2 BezPt(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3,
                            float t) {
    float u = 1.0f - t;
    return (Vector2){
        u * u * u * p0.x + 3.0f * u * u * t * c1.x +
        3.0f * u * t * t * c2.x + t * t * t * p3.x,
        u * u * u * p0.y + 3.0f * u * u * t * c1.y +
        3.0f * u * t * t * c2.y + t * t * t * p3.y
    };
}

static inline Vector2 BezTan(Vector2 p0, Vector2 c1, Vector2 c2, Vector2 p3,
                             float t) {
    float u = 1.0f - t;
    return (Vector2){
        3.0f * (u * u * (c1.x - p0.x) + 2.0f * u * t * (c2.x - c1.x) +
                t * t * (p3.x - c2.x)),
        3.0f * (u * u * (c1.y - p0.y) + 2.0f * u * t * (c2.y - c1.y) +
                t * t * (p3.y - c2.y))
    };
}

static void EdgeCP(Vector2 a, Vector2 b, Vector2 *c1, Vector2 *c2) {
    float dx = b.x - a.x, dy = b.y - a.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) {
        *c1 = a;
        *c2 = b;
        return;
    }
    float px = -dy / len, py = dx / len;
    float off = len * 0.18f;
    *c1 = (Vector2){a.x + dx * 0.33f + px * off, a.y + dy * 0.33f + py * off};
    *c2 = (Vector2){a.x + dx * 0.67f + px * off, a.y + dy * 0.67f + py * off};
}

void DrawArrowAt(Vector2 pos, float ca, float sa, float sz, Color col) {
    float cp = -sa, sp = ca;
    Vector2 tip = {pos.x + ca * sz * 1.5f, pos.y + sa * sz * 1.5f};
    Vector2 bl = {
        pos.x - ca * sz * 0.6f + cp * sz * 0.85f,
        pos.y - sa * sz * 0.6f + sp * sz * 0.85f
    };
    Vector2 br = {
        pos.x - ca * sz * 0.6f - cp * sz * 0.85f,
        pos.y - sa * sz * 0.6f - sp * sz * 0.85f
    };
    DrawTriangle(tip, bl, br, col);
}

void DrawBackground(void) {
    ClearBackground(C_BG);

    /* ── nebula clouds ── */
    DrawCircleGradient(160, 110, 230.0f, (Color){70, 0, 130, 55}, C_TRANS);
    DrawCircleGradient(820, 580, 270.0f, (Color){0, 80, 150, 50}, C_TRANS);
    DrawCircleGradient(760, 100, 190.0f, (Color){50, 0, 95, 48}, C_TRANS);
    DrawCircleGradient(90, 560, 180.0f, (Color){0, 110, 85, 44}, C_TRANS);
    DrawCircleGradient(480, 640, 150.0f, (Color){30, 60, 120, 38}, C_TRANS);

    /* ── star field (deterministic LCG scatter) ── */
    for (int i = 0; i < 140; i++) {
        int x = (i * 6271 + 1543) % GRAPH_W;
        int y = (i * 3947 + 897) % WIN_H;
        float r = (i % 9 == 0) ? 2.0f : (i % 4 == 0) ? 1.3f : 0.7f;
        unsigned char a = (unsigned char) (90 + (i * 53) % 130);
        DrawCircleV((Vector2){(float) x, (float) y}, r,
                    (Color){210, 225, 255, a});
    }

    /* ── faint grid (just enough to read depth) ── */
    for (int x = 0; x <= GRAPH_W; x += 80)
        DrawLine(x, 0, x, WIN_H, (Color){30, 50, 100, 22});
    for (int y = 0; y <= WIN_H; y += 80)
        DrawLine(0, y, GRAPH_W, y, (Color){30, 50, 100, 22});

    /* ── central ambient glow ── */
    DrawCircleGradient(GRAPH_W / 2, WIN_H / 2, 400.0f, (Color){0, 60, 145, 40},
                       C_TRANS);

    DrawRectangleGradientH(0, 0, 90, WIN_H, C_VIGN, C_TRANS);
    DrawRectangleGradientH(GRAPH_W - 90, 0, 90, WIN_H, C_TRANS, C_VIGN);
    DrawRectangleGradientV(0, 0, GRAPH_W, 90, C_VIGN, C_TRANS);
    DrawRectangleGradientV(0, WIN_H - 90, GRAPH_W, 90, C_TRANS, C_VIGN);
}

static void DrawNodeTile(RenderCtx *ctx, int idx) {
    Vector2 p = ctx->positions[idx];
    float size = NODE_SZ * 1.2f;
    float pulse = (sinf(s_time * 2.2f + idx * 0.85f) + 1.0f) * 0.5f;
    Color ring = C_NODE_RING;
    ring.a = (unsigned char) (50 + 70 * pulse);
    DrawCircleLines((int) p.x, (int) p.y, (size * 0.5f) + 4.0f + pulse * 3.0f,
                    ring);

    Texture2D currentTex = ctx->stationTextures[idx % NUM_STATION_TYPES];

    Rectangle sourceRec = {
        0.0f, 0.0f, (float) currentTex.width,
        (float) currentTex.height
    };
    Rectangle destRec = {p.x, p.y, size, size};
    Vector2 origin = {size * 0.5f, size * 0.5f};

    DrawTexturePro(currentTex, sourceRec, destRec, origin, 0.0f, WHITE);

    char id[16];
    snprintf(id, sizeof id, "%d", idx);
    int tw = MeasureText(id, 11);
    DrawText(id, (int) (p.x - tw * 0.5f), (int) (p.y + (size * 0.5f) + 5), 11,
             C_NODE_ID);
}

void DrawNodes(RenderCtx *ctx) {
    for (int i = 0; i < ctx->node_count; i++)
        DrawNodeTile(ctx, i);
}

/* ── Single edge ────────────────────────────────────────────────────────── */
void DrawEdge(Vector2 a, Vector2 b, int weight) {
    Vector2 c1, c2;
    EdgeCP(a, b, &c1, &c2);

    DrawSplineSegmentBezierCubic(a, c1, c2, b, 9.5f, C_ROAD_CASE);
    DrawSplineSegmentBezierCubic(a, c1, c2, b, 5.5f, C_ROAD_MID);
    DrawSplineSegmentBezierCubic(a, c1, c2, b, 1.8f, C_ROAD_CTR);

    Vector2 dir = {b.x - c2.x, b.y - c2.y};
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);

    if (len > 0.001f) {
        float dx = dir.x / len;
        float dy = dir.y / len;

        float offset = NODE_SZ * 0.5f + 6.0f;
        Vector2 arrowTip = {b.x - dx * offset, b.y - dy * offset};

        float arrowSize = 14.0f;
        Vector2 wingLeft = {
            arrowTip.x - (dx - dy) * arrowSize * 0.5f,
            arrowTip.y - (dy + dx) * arrowSize * 0.5f
        };
        Vector2 wingRight = {
            arrowTip.x - (dx + dy) * arrowSize * 0.5f,
            arrowTip.y - (dy - dx) * arrowSize * 0.5f
        };

        Color graphArrowColor = (Color){0, 220, 255, 240};
        DrawLineEx(arrowTip, wingLeft, 3.0f, graphArrowColor);
        DrawLineEx(arrowTip, wingRight, 3.0f, graphArrowColor);
    }

    Vector2 mid = BezPt(a, c1, c2, b, 0.5f);
    char buf[8];
    snprintf(buf, sizeof buf, "%d", weight);
    int tw = MeasureText(buf, 10);
    int bw = tw + 11, bh = 15;
    Rectangle br = {mid.x - bw * 0.5f, mid.y - bh * 0.5f, (float) bw, (float) bh};

    DrawRectangleRounded(
        (Rectangle){br.x - 1, br.y - 1, br.width + 2, br.height + 2}, 0.5f, 5,
        C_BADGE_BD);
    DrawRectangleRounded(br, 0.5f, 5, C_BADGE_BG);
    DrawText(buf, (int) (mid.x - tw * 0.5f), (int) (mid.y - 5), 10, C_BADGE_TXT);
}

void DrawEdges(RenderCtx *ctx, Graph *g) {
    for (int i = 0; i < g->num_nodes; i++)
        for (Node *e = g->adj[i]; e; e = e->next)
            DrawEdge(ctx->positions[i], ctx->positions[e->id], e->weight);
}

static int GetEdgeWeight(Graph *g, int from, int to) {
    for (Node *e = g->adj[from]; e; e = e->next)
        if (e->id == to)
            return e->weight;
    return 1;
}

void UpdateCar(Car *car, RenderCtx *ctx, Graph *g, float dt) {
    if (car->state == CAR_ARRIVED || car->state == CAR_IDLE || !car->path)
        return;

    if (car->state == CAR_NODE_WAIT) {
        car->timer -= dt;
        if (car->timer <= 0.0f) {
            car->path_idx++;
            if (car->path_idx >= car->path_len - 1) {
                car->state = CAR_ARRIVED;
                int last = car->path[car->path_len - 1];
                car->x = ctx->positions[last].x;
                car->y = ctx->positions[last].y;
            } else {
                car->state = CAR_MOVING;
                car->t = 0.0f;
            }
        }
        return;
    }

    int from = car->path[car->path_idx];
    int to = car->path[car->path_idx + 1];
    int w = GetEdgeWeight(g, from, to);
    car->t += (car->speed / (float) w) * dt;
    if (car->t >= 1.0f) {
        int ni = car->path[car->path_idx + 1];
        car->x = ctx->positions[ni].x;
        car->y = ctx->positions[ni].y;
        car->t = 1.0f;
        car->state = CAR_NODE_WAIT;
        car->timer = 0.10f;
    } else {
        Vector2 c1, c2;
        EdgeCP(ctx->positions[from], ctx->positions[to], &c1, &c2);
        Vector2 pos =
                BezPt(ctx->positions[from], c1, c2, ctx->positions[to], car->t);
        car->x = pos.x;
        car->y = pos.y;
    }
}

void UpdateCars(RenderCtx *ctx, Graph *g, float dt) {
    if (ctx->paused || !ctx->running)
        return;
    bool all = true;
    for (int i = 0; i < ctx->numCars; i++) {
        UpdateCar(&ctx->cars[i], ctx, g, dt);
        Car *c = &ctx->cars[i];
        if (c->state == CAR_ARRIVED && !c->notified) {
            c->notified = true;
            char label[32];
            snprintf(label, sizeof label, "Traveler %d", c->id + 1);
            FireToast(ctx, label, c->color);
        }
        if (c->state != CAR_ARRIVED)
            all = false;
    }
    if (all) {
        ctx->all_arrived = true;
        ctx->running = false;
    }
}

void DrawSingleCar(Car *car, RenderCtx *ctx) {
    if (car->state == CAR_IDLE)
        return;
    float cx = car->x, cy = car->y;
    float ca = car->last_ca, sa = car->last_sa;
    if (ca == 0.0f && sa == 0.0f) {
        ca = 1.0f;
        sa = 0.0f;
    }

    if (car->state == CAR_MOVING && car->path &&
        car->path_idx < car->path_len - 1) {
        Vector2 a = ctx->positions[car->path[car->path_idx]],
                b = ctx->positions[car->path[car->path_idx + 1]];
        Vector2 c1, c2;
        EdgeCP(a, b, &c1, &c2);
        Vector2 tan = BezTan(a, c1, c2, b, car->t);
        float tl = sqrtf(tan.x * tan.x + tan.y * tan.y);
        if (tl > 0.001f) {
            ca = tan.x / tl;
            sa = tan.y / tl;
            car->last_ca = ca;
            car->last_sa = sa;
        }
    }

    Color glow = car->color;
    glow.a = 45;
    DrawCircleV((Vector2){cx, cy}, CAR_SZ + 5.5f, glow);

    if (car->state == CAR_MOVING && car->path &&
        car->path_idx < car->path_len - 1) {
        Vector2 a = ctx->positions[car->path[car->path_idx]],
                b = ctx->positions[car->path[car->path_idx + 1]];
        Vector2 c1, c2;
        EdgeCP(a, b, &c1, &c2);
        float t1 = car->t - 0.045f, t2 = car->t - 0.09f;
        if (t1 > 0.0f) {
            Color tc = car->color;
            tc.a = 70;
            DrawCircleV(BezPt(a, c1, c2, b, t1), CAR_SZ * 0.48f, tc);
        }
        if (t2 > 0.0f) {
            Color tc = car->color;
            tc.a = 28;
            DrawCircleV(BezPt(a, c1, c2, b, t2), CAR_SZ * 0.3f, tc);
        }
    }
    DrawCarShape(cx, cy, ca, sa, CAR_SZ, car->color);
    DrawCircleV((Vector2){cx, cy}, 2.2f, (Color){255, 255, 255, 190});
}

void DrawArrivedBanner(void) {
    float alpha = (sinf(s_time * 2.8f) + 1.0f) * 0.5f;
    int by = WIN_H / 2 - 54, bw = GRAPH_W, bh = 108;
    DrawRectangle(0, by, bw, bh, C_SUCCESS_BG);
    DrawRectangle(0, by, bw, 2, C_SUCCESS_LINE);
    DrawRectangle(0, by + bh - 2, bw, 2, C_SUCCESS_LINE);

    int x1 = bw / 2 - MeasureText("ALL TRAVELERS ARRIVED", 30) / 2,
            x2 =
                    bw / 2 -
                    MeasureText("Simulation Complete  —  Press RESTART to replay", 13) /
                    2;
    for (int d = 7; d >= 1; d--)
        DrawText("ALL TRAVELERS ARRIVED", x1 + d / 2, by + 18 + d / 2, 30,
                 (Color){0, 200, 110, (unsigned char) ((35 + 30 * alpha) / d)});
    DrawText("ALL TRAVELERS ARRIVED", x1, by + 18, 30, C_SUCCESS_TXT);
    DrawText("Simulation Complete  —  Press RESTART to replay", x2, by + 62, 13,
             (Color){100, 200, 155, 210});
}

void DrawPlayOverlay(RenderCtx *ctx) {
    float cx = GRAPH_W * 0.5f, cy = WIN_H * 0.5f, r = 50.0f,
            pulse = (sinf(s_time * 1.9f) + 1.0f) * 0.5f;
    DrawRectangle(0, 0, GRAPH_W, WIN_H, (Color){0, 0, 0, 60});
    DrawCircleV((Vector2){cx, cy}, r + 14.0f + pulse * 6.0f,
                (Color){0, 200, 255, (unsigned char) (28 + 22 * pulse)});
    DrawCircleV((Vector2){cx, cy}, r, (Color){10, 18, 42, 235});
    DrawCircleLines((int) cx, (int) cy, r,
                    (Color){0, 190, 255, (unsigned char) (170 + 70 * pulse)});
    DrawTriangle((Vector2){cx + 16.0f, cy}, (Vector2){cx - 8.0f, cy - 16.0f},
                 (Vector2){cx - 8.0f, cy + 16.0f}, (Color){0, 210, 255, 230});
    DrawText("PRESS PLAY TO START",
             (int) (cx - MeasureText("PRESS PLAY TO START", 12) * 0.5f),
             (int) (cy + r + 14.0f), 12,
             (Color){0, 180, 255, (unsigned char) (160 + 70 * pulse)});

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(GetMousePosition(), (Vector2){cx, cy}, r)) {
        ctx->running = true;
        ctx->paused = false;
    }
}

RenderCtx *InitRenderer(int num_nodes, Vector2 *positions, int num_cars) {
    RenderCtx *ctx = calloc(1, sizeof *ctx);
    ctx->node_count = num_nodes;
    ctx->numCars = num_cars;
    ctx->positions = malloc(num_nodes * sizeof(Vector2));
    ctx->cars = calloc(num_cars, sizeof(Car));
    for (int i = 0; i < num_nodes; i++)
        ctx->positions[i] = positions[i];

    char filePath[32];
    for (int i = 0; i < NUM_STATION_TYPES; i++) {
        snprintf(filePath, sizeof(filePath), "data/station%d.png", i);
        ctx->stationTextures[i] = LoadTexture(filePath);

        GenTextureMipmaps(&ctx->stationTextures[i]);
        SetTextureFilter(ctx->stationTextures[i], TEXTURE_FILTER_TRILINEAR);
    }
    return ctx;
}

static bool DrawButton(Rectangle r, const char *label, Color bg, Color hov) {
    Vector2 mp = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mp, r);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color fill = hovered ? hov : bg;
    DrawRectangleRounded(
        (Rectangle){r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 0.35f, 8,
        C_BTN_BORD);
    DrawRectangleRounded(r, 0.35f, 8, fill);
    int tw = MeasureText(label, 14);
    DrawText(label, (int) (r.x + r.width * 0.5f - tw * 0.5f),
             (int) (r.y + r.height * 0.5f - 7), 14, C_BTN_TXT);
    return clicked;
}

void DrawPanel(RenderCtx *ctx) {
    DrawRectangle(PANEL_X, 0, PANEL_W, WIN_H, C_PANEL_BG);
    DrawRectangle(PANEL_X, 0, 1, WIN_H, C_PANEL_SEP);
    int y = 18;

    DrawText("TRAFFIC SIM", PANEL_X + 16, y, 19, C_PANEL_TITLE);
    y += 22;
    DrawText("OS Course  —  Graph Visualizer", PANEL_X + 16, y, 10,
             (Color){85, 125, 180, 190});
    y += 16;
    DrawRectangle(PANEL_X + 12, y, PANEL_W - 24, 1, C_PANEL_SEP);
    y += 10;

    const char *status = ctx->all_arrived
                             ? "COMPLETE"
                             : ctx->paused
                                   ? "PAUSED"
                                   : ctx->running
                                         ? "RUNNING"
                                         : "READY";
    Color sc = ctx->all_arrived
                   ? C_SUCCESS_TXT
                   : ctx->paused
                         ? (Color){255, 195, 50, 255}
                         : C_PANEL_TITLE;

    DrawText("STATUS", PANEL_X + 16, y, 10, (Color){75, 115, 165, 200});
    int stw = MeasureText(status, 11);
    DrawText(status, PANEL_X + PANEL_W - 16 - stw, y, 11, sc);
    y += 18;
    DrawRectangle(PANEL_X + 12, y, PANEL_W - 24, 1, C_PANEL_SEP);
    y += 10;

    DrawText("TRAVELERS", PANEL_X + 16, y, 11, C_PANEL_TXT);
    y += 16;

    for (int i = 0; i < ctx->numCars; i++) {
        Car *c = &ctx->cars[i];
        int ry = y + i * 56;
        float bx = (float) (PANEL_X + 14);
        float bw = (float) (PANEL_W - 28);

        DrawRectangleRounded((Rectangle){bx, (float) ry, 11, 11}, 0.4f, 4,
                             c->color);
        char lbl[24];
        snprintf(lbl, sizeof lbl, "Traveler %d", c->id + 1);
        DrawText(lbl, PANEL_X + 30, ry, 12, C_PANEL_TXT);

        const char *st = c->state == CAR_ARRIVED
                             ? "ARRIVED"
                             : c->state == CAR_MOVING
                                   ? "MOVING"
                                   : c->state == CAR_NODE_WAIT
                                         ? "WAITING"
                                         : "IDLE";
        Color stc = c->state == CAR_ARRIVED
                        ? C_SUCCESS_TXT
                        : c->state == CAR_MOVING
                              ? C_PANEL_TITLE
                              : (Color){180, 140, 60, 220};
        int sw = MeasureText(st, 10);
        DrawText(st, PANEL_X + PANEL_W - 14 - sw, ry, 10, stc);

        float prog = 0.0f;
        if (c->path_len > 1) {
            prog = ((float) c->path_idx + c->t) / (float) (c->path_len - 1);
            if (prog > 1.0f)
                prog = 1.0f;
        } else if (c->state == CAR_ARRIVED) {
            prog = 1.0f;
        }

        DrawRectangle((int) bx, ry + 16, (int) bw, 4, (Color){18, 30, 58, 255});
        DrawRectangle((int) bx, ry + 16, (int) (bw * prog), 4, c->color);

        char ps[52];
        int plen = (int) strlen(c->path_str);
        if (plen > 46) {
            strncpy(ps, c->path_str, 43);
            ps[43] = '.';
            ps[44] = '.';
            ps[45] = '.';
            ps[46] = '\0';
        } else {
            strncpy(ps, c->path_str, sizeof ps - 1);
            ps[sizeof ps - 1] = '\0';
        }
        DrawText(ps, (int) bx, ry + 24, 9, (Color){90, 130, 175, 200});

        DrawRectangle(PANEL_X + 12, ry + 40, PANEL_W - 24, 1,
                      (Color){18, 30, 55, 180});
    }

    y += ctx->numCars * 56 + 8;
    DrawRectangle(PANEL_X + 12, y, PANEL_W - 24, 1, C_PANEL_SEP);
    y += 14;

    float p_bx = (float) (PANEL_X + BTN_MX);
    float p_bw = (float) (PANEL_W - BTN_MX * 2);

    const char *play_lbl = (!ctx->running && !ctx->paused)
                               ? "PLAY"
                               : ctx->running
                                     ? "PAUSE"
                                     : "RESUME";
    Color play_bg = (!ctx->running && !ctx->paused)
                        ? C_BTN_PLAY
                        : ctx->running
                              ? C_BTN_IDLE
                              : C_BTN_PLAY;
    Color play_hov = (!ctx->running && !ctx->paused)
                         ? C_BTN_PLAY_HOV
                         : ctx->running
                               ? C_BTN_HOVER
                               : C_BTN_PLAY_HOV;

    if (DrawButton((Rectangle){p_bx, (float) y, p_bw, BTN_H}, play_lbl, play_bg,
                   play_hov)) {
        if (!ctx->all_arrived) {
            if (!ctx->running && !ctx->paused)
                ctx->running = true;
            else if (ctx->running) {
                ctx->running = false;
                ctx->paused = true;
            } else {
                ctx->running = true;
                ctx->paused = false;
            }
        }
    }
    y += BTN_H + 9;

    if (DrawButton((Rectangle){p_bx, (float) y, p_bw, BTN_H}, "RESTART",
                   C_BTN_IDLE, C_BTN_HOVER)) {
        for (int i = 0; i < ctx->numCars; i++) {
            Car *c = &ctx->cars[i];
            if (c->path) {
                free(c->path);
                c->path = NULL;
            }
            c->path_idx = 0;
            c->path_len = 0;
            c->t = 0.0f;
            c->timer = 0.0f;
            c->notified = false;
            c->state = CAR_IDLE;
            c->path_str[0] = '\0';
        }
        memset(ctx->toasts, 0, sizeof(ctx->toasts));
        ctx->all_arrived = false;
        ctx->running = false;
        ctx->paused = false;
    }
    y += BTN_H + 14;

    DrawRectangle(PANEL_X + 12, y, PANEL_W - 24, 1, C_PANEL_SEP);
    y += 10;
    char info[64];
    snprintf(info, sizeof info, "Nodes: %d     Travelers: %d", ctx->node_count,
             ctx->numCars);
    DrawText(info, PANEL_X + 14, y, 10, (Color){65, 98, 148, 190});
}

void DrawCarShape(float cx, float cy, float ca, float sa, float sz, Color col) {
    float cp = -sa, sp = ca;
    Vector2 front = {cx + ca * sz * 1.65f, cy + sa * sz * 1.65f};
    Vector2 bl = {
        cx - ca * sz * 0.75f + cp * sz * 0.95f,
        cy - sa * sz * 0.75f + sp * sz * 0.95f
    };
    Vector2 br = {
        cx - ca * sz * 0.75f - cp * sz * 0.95f,
        cy - sa * sz * 0.75f - sp * sz * 0.95f
    };

    DrawTriangle((Vector2){front.x + 2.5f, front.y + 2.5f},
                 (Vector2){br.x + 2.5f, br.y + 2.5f},
                 (Vector2){bl.x + 2.5f, bl.y + 2.5f}, C_CAR_SHADOW);
    DrawTriangle(front, br, bl, col);

    Vector2 hf = {cx + ca * sz * 0.7f, cy + sa * sz * 0.7f};
    Vector2 hl = {
        cx - ca * sz * 0.1f + cp * sz * 0.45f,
        cy - sa * sz * 0.1f + sp * sz * 0.45f
    };
    Vector2 hr = {
        cx - ca * sz * 0.1f - cp * sz * 0.45f,
        cy - sa * sz * 0.1f - sp * sz * 0.45f
    };
    DrawTriangle(hf, hr, hl, (Color){255, 255, 255, 55});
}

void RenderFrame(RenderCtx *ctx, Graph *g, float dt) {
    s_time += dt;
    UpdateCars(ctx, g, dt);
    UpdateToasts(ctx, dt);
    DrawBackground();
    DrawEdges(ctx, g);
    DrawNodes(ctx);
    for (int i = 0; i < ctx->numCars; i++)
        DrawSingleCar(&ctx->cars[i], ctx);
    DrawPanel(ctx);
    if (!ctx->running && !ctx->paused && !ctx->all_arrived)
        DrawPlayOverlay(ctx);
    if (ctx->all_arrived)
        DrawArrivedBanner();
    DrawToasts(ctx);
    DrawText(TextFormat("FPS %d", GetFPS()), GRAPH_W - 58, WIN_H - 18, 10,
             C_FPS_TXT);
}

void ApplyTravelerUpdate(RenderCtx *ctx, int traveler_idx, int current_node, int next_node) {
    if (!ctx || traveler_idx < 0 || traveler_idx >= ctx->numCars) return;
    Car *c = &ctx->cars[traveler_idx];

    if (next_node == -1) {
        c->x = ctx->positions[current_node].x;
        c->y = ctx->positions[current_node].y;
        c->state = CAR_ARRIVED;
        if (c->path) {
            free(c->path);
            c->path = NULL;
        }
        c->path_len = 0;
        c->path_idx = 0;
        c->t = 0.0f;
        return;
    }

    if (c->path) free(c->path);
    c->path = malloc(2 * sizeof(int));
    if (!c->path) return;
    c->path[0] = current_node;
    c->path[1] = next_node;
    c->path_len = 2;
    c->path_idx = 0;
    c->t = 0.0f;
    c->state = CAR_MOVING;
    c->x = ctx->positions[current_node].x;
    c->y = ctx->positions[current_node].y;

    int off = (int) strlen(c->path_str);
    if (off == 0)
        off += snprintf(c->path_str, sizeof(c->path_str), "%d", current_node);
    snprintf(c->path_str + off, sizeof(c->path_str) - (size_t) off, "->%d", next_node);
}

void readTravelerPathFromSharedMemory(RenderCtx *ctx, TravelerMsg *shared_mem, int count) {
    for (int i = 0; i < count; i++) {
        if (ctx->cars[i].state == CAR_IDLE) {
            if (sem_trywait(&shared_mem[i].sem_ready_to_read) == 0) {
                int pid = shared_mem[i].pid;
                int curr = shared_mem[i].current_node;
                int next = shared_mem[i].next_node;
                if (next == -1) {
                    printf("[PID=%d] arrived at node %d | DESTINATION\n", pid, curr);
                    printf("[PID=%d] finished\n", pid);
                } else {
                    printf("[PID=%d] arrived at node %d | next node: %d\n", pid, curr, next);
                }
                fflush(stdout);
                ApplyTravelerUpdate(ctx, i, curr, next);
                sem_post(&shared_mem[i].sem_ready_to_write);

            }
        }
    }
}

void FreeRenderer(RenderCtx *ctx) {
    if (ctx) {
        for (int i = 0; i < NUM_STATION_TYPES; i++) {
            UnloadTexture(ctx->stationTextures[i]);
        }
        free(ctx->positions);
        free(ctx->cars);
        free(ctx);
    }
}
