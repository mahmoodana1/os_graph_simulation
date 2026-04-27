#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "graph.h"

// open a window and initialize the GUI
void startGui(Graph* g);
void DrawArrowHead(Vector2 start, Vector2 end, Color color);
#endif
