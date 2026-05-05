#include "gui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const Color NODE_ACCENTS[] = {
    { 210, 80, 70, 255 }, { 60, 155, 155, 255 },
    { 50, 65, 90, 255 }, { 200, 150, 60, 255 }
};



static void DrawWeightBadge(Vector2 s, Vector2 d, int w, bool on_path) {
    // 1. حساب نقطة المنتصف بين النود البداية والنهاية
    Vector2 mid = { (s.x + d.x) * 0.5f, (s.y + d.y) * 0.5f };

    // 2. تجهيز النص وحساب عرضه ليتوسط الدائرة
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", w);
    int tw = MeasureText(buf, 15);

    // 3. رسم الدائرة: حمراء للمسار المختار، وداكنة للطرق العادية
    DrawCircleV(mid, 14, on_path ? MM_HEART : (Color){45, 52, 70, 200});

    // 4. رسم الرقم باللون الأبيض
    DrawText(buf, (int)(mid.x - tw/2), (int)(mid.y - 7), 15, WHITE);
}

static void DrawHeart(float cx, float cy) {
    float s = 11.0f;
    DrawCircle(cx - s/2, cy - s/2, s/2, MM_HEART);
    DrawCircle(cx + s/2, cy - s/2, s/2, MM_HEART);
    DrawTriangle((Vector2){cx - s - 1, cy - s/2 + 1}, (Vector2){cx, cy + s + 2}, (Vector2){cx + s + 1, cy - s/2 + 1}, MM_HEART);
}

static void DrawArrow(Vector2 start, Vector2 end, Color c) {
    float dx = end.x - start.x, dy = end.y - start.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 5.0f) return;
    Vector2 unit = { dx/len, dy/len };
    Vector2 tip = { end.x - unit.x * (NODE_SIZE * 0.52f), end.y - unit.y * (NODE_SIZE * 0.52f) };
    float aLen = 22.0f; float aHalf = 18.0f;
    Vector2 base = { tip.x - unit.x * aLen, tip.y - unit.y * aLen };
    DrawTriangle(tip, (Vector2){base.x + unit.y * aHalf, base.y - unit.x * aHalf}, (Vector2){base.x - unit.y * aHalf, base.y + unit.x * aHalf}, c);
}

static void DrawRoad(Vector2 a, Vector2 b, float thick, Color c) {
    DrawLineEx(a, b, thick, c);
    DrawCircleV(a, thick * 0.5f, c);
    DrawCircleV(b, thick * 0.5f, c);
    DrawArrow(a, b, c);
}

static void DrawNodeTile(RenderCtx* ctx, int idx) {
    Vector2 p = ctx->positions[idx];
    Rectangle r = { p.x - NODE_SIZE/2, p.y - NODE_SIZE/2, NODE_SIZE, NODE_SIZE };
    DrawRectangleRounded(r, 0.25f, 8, MM_NODE_BG);
    Color c = (idx == ctx->src) ? (Color){210, 80, 70, 255} : (idx == ctx->dst) ? (Color){60, 155, 155, 255} : ctx->accents[idx % 4];
    DrawRectangleRounded((Rectangle){ r.x, r.y, NODE_SIZE, NODE_SIZE * 0.5f }, 0.25f, 6, c);
    DrawText(TextFormat("%d", idx), (int)(p.x - 5), (int)(p.y + 8), 14, MM_TEXT_DARK);
}

static void UpdateCar(Car* car, RenderCtx* ctx, Graph* g, float dt) {
    car->timer += dt;
    if (car->state == CAR_IDLE || car->state == CAR_NODE_WAIT) {
        car->x = ctx->positions[car->path[car->seg]].x;
        car->y = ctx->positions[car->path[car->seg]].y;
        float wait = (car->seg == 0 || car->seg + 1 >= car->path_len) ? 0.0f : NODE_WAIT_SEC;
        if (car->timer >= wait) {
            if (car->seg + 1 >= car->path_len) { car->state = CAR_ARRIVED; return; }
            car->timer = 0.0f; car->hop = 0;
            Node* e = g->adj[car->path[car->seg]];
            while(e) { if(e->id == car->path[car->seg+1]) { car->total_hops = e->weight; break; } e = e->next; }
            car->state = CAR_MOVING;
        }
    } else if (car->state == CAR_MOVING) {
        float t = fminf(car->timer / HOP_DURATION_SEC, 1.0f);
        Vector2 s = ctx->positions[car->path[car->seg]], d = ctx->positions[car->path[car->seg+1]];
        float prog = ((float)car->hop + t) / (float)car->total_hops;
        car->x = s.x + prog*(d.x-s.x); car->y = s.y + prog*(d.y-s.y);
        if (car->timer >= HOP_DURATION_SEC) {
            car->timer = 0.0f; car->hop++;
            if (car->hop >= car->total_hops) { car->seg++; car->state = CAR_NODE_WAIT; }
        }
    }
}

RenderCtx* InitRenderer(Graph* g, int src, int dst, int* path, int path_len) {
    RenderCtx* ctx = (RenderCtx*)calloc(1, sizeof(RenderCtx));
    ctx->src = src; ctx->dst = dst; ctx->dijk_len = path_len;
    memcpy(ctx->dijk_path, path, path_len * sizeof(int));
    float cx = SCREEN_W/2, cy = SCREEN_H/2, R = 210;
    for (int i = 0; i < g->num_nodes; i++) {
        float a = (float)i / (float)g->num_nodes * 2.0f * 3.14159f - 1.57f;
        ctx->positions[i] = (Vector2){ cx + R*cosf(a), cy + R*sinf(a) };
        ctx->accents[i] = NODE_ACCENTS[i % 4];
    }
    memcpy(ctx->car.path, path, path_len * sizeof(int));
    ctx->car.path_len = path_len; ctx->car.state = CAR_IDLE;
    return ctx;
}

void FreeRenderer(RenderCtx* ctx) { if(ctx) free(ctx); }

bool RenderFrame(RenderCtx* ctx, Graph* g, float dt) {
    if (WindowShouldClose()) return false;
    if (ctx->playing && ctx->car.state != CAR_ARRIVED) UpdateCar(&ctx->car, ctx, g, dt);
    BeginDrawing(); ClearBackground(MM_BG);

    for (int i = 0; i < g->num_nodes; i++) {
        Node* e = g->adj[i];
        while(e) {
            bool on = false;
            for(int k=0; k+1 < ctx->dijk_len; k++) if(ctx->dijk_path[k]==i && ctx->dijk_path[k+1]==e->id) on=true;
            if(!on) DrawRoad(ctx->positions[i], ctx->positions[e->id], ROAD_THICK, MM_ROAD);
            e = e->next;
        }
    }

    for (int i = 0; i + 1 < ctx->dijk_len; i++) {
        DrawRoad(ctx->positions[ctx->dijk_path[i]], ctx->positions[ctx->dijk_path[i+1]], ROAD_THICK + 6, MM_ROAD_PATH);
    }

    for (int i = 0; i < g->num_nodes; i++) DrawNodeTile(ctx, i);
    if (ctx->playing && ctx->car.state != CAR_ARRIVED) DrawHeart(ctx->car.x, ctx->car.y);

    int bx = SCREEN_W - 116, by = SCREEN_H - 50; Rectangle btn = { (float)bx, (float)by, 100, 36 };
    DrawRectangleRounded(btn, 0.45f, 6, ctx->playing ? MM_BTN_STOP : MM_BTN_PLAY);
    DrawText(ctx->playing ? " Stop" : " Play", bx + 25, by + 10, 16, WHITE);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn)) ctx->playing = !ctx->playing;

    EndDrawing(); return true;
}
/// /// /// //// /// / / // /  /





//////////////////
