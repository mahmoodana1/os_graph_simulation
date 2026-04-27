#include <math.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "raylib.h"


const int screenWidth = 800;
const int screenHeight = 600;

void startGui(Graph* g)
{
    // Screen size and frame settings
    InitWindow(screenWidth, screenHeight, "Graph Visualization - Milestone 2");
    SetTargetFPS(60);

        while (!WindowShouldClose())
        {
            // Cleaning and preparing the screen
            BeginDrawing();
            ClearBackground(RAYWHITE);
            float centerX = screenWidth / 2;
            float centerY = screenHeight / 2;
            float radius = 200;

            for (int i = 0; i < g->num_nodes; i++)
            {
                Node* temp = g->adj[i];
                while (temp != NULL)
                {
                    // int target = temp->id;
                    int weight = temp->weight;



                    float angle = i * (2 * PI / g->num_nodes);
                    float x = centerX + radius * cosf(angle);
                    float y = centerY + radius * sinf(angle);

                    // Draw Circle
                    DrawCircle(x, y, 25,BLUE);
                    DrawText(TextFormat("%d", weight), x - 5, y - 8, 20,BLACK);

                    temp = temp->next;
                }
            }


            EndDrawing();
        }
        CloseWindow();





}

