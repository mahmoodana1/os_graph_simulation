#include <math.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "raylib.h"


const int screenWidth = 800;
const int screenHeight = 600;


void DrawArrowHead(Vector2 start, Vector2 end, Color color)
{
    // Calculating the angle of inclination of a triangle using the Arc-Attan function
    float angle = atan2(end.y - start.y, end.x - start.x);

    float arrowDist = 30;
    Vector2 tip = {
        end.x - cosf(angle) * arrowDist,
        end.y - sinf(angle) * arrowDist
    };

    DrawPoly(tip, 3,10,(angle * RAD2DEG) + 90,color);
}


void startGui(Graph* g)
{
    if (g == NULL) return;

    // Screen size and frame settings
    InitWindow(screenWidth, screenHeight, "Graph Visualization - Milestone 2");
    SetTargetFPS(60);

    // A vector is a structure in the Raylib library that contains x and y,
    // and its purpose is to define the locations of the nodes
    Vector2 nodePositions[15];

    // Place the drawing in the middle
    float centerX = screenWidth / 2;
    float centerY = screenHeight / 2;
    float radius = 200;

    // Making a circle
    for (int i = 0; i < g->num_nodes; i++)
    {
        // (2 * PI / g->num_nodes) So that the contract is distributed equally
        float calcAngle = i * (2 * PI / g->num_nodes);
        nodePositions[i].x = centerX + radius * cosf(calcAngle);
        // Calculate the horizontal coordinate using the cosine function.
        nodePositions[i].y = centerY + radius * sinf(calcAngle);
        // Calculate the vertical coordinate using the sine function.
    }


    while (!WindowShouldClose())
    {
        // Cleaning and preparing the screen
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < g->num_nodes; i++)
        {
            Node* temp = g->adj[i];
            while (temp != NULL)
            {
                int target = temp->id; // We're going to
                //int weight = temp->weight; // Its weight

                // lins...
                DrawLineEx(nodePositions[i], nodePositions[target], 2, GRAY);

                ///
                DrawArrowHead(nodePositions[i], nodePositions[target], ORANGE);
                ///

                // Weight location calculator (in the mid)
                float midX = (nodePositions[i].x + nodePositions[target].x) / 2.0f;
                float midY = (nodePositions[i].y + nodePositions[target].y) / 2.0f;

                DrawText(TextFormat("%d", temp->weight), midX + 5, midY, 20, RED);

                temp = temp->next;
            }
        }

        for (int i = 0; i < g->num_nodes; i++)
        {
            DrawCircleV(nodePositions[i], 25, BLUE);
            DrawText(TextFormat("%d", i), nodePositions[i].x - 5, nodePositions[i].y - 8, 20, WHITE);
        }

        EndDrawing();
    }
    CloseWindow();
}

/// /// /// //// /// / / // /  /





//////////////////
