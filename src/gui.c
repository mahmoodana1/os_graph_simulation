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
/// /// /// //// /// / / // /  /





//////////////////
