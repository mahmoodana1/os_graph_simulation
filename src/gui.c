#include "gui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const Color NODE_ACCENTS[] = {
    { 210, 80, 70, 255 }, { 60, 155, 155, 255 },
    { 50, 65, 90, 255 }, { 200, 150, 60, 255 }
};

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
/// /// /// //// /// / / // /  /





//////////////////
