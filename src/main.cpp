/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


#include <cassert>
#include <stack>   
#include <sstream>
#include <iostream>

#include "Game.cpp"

#include "raylib.h"

// NOTE: Program main entry point
int main(void)
{

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");

    GameState gameState;

    InitTexture();
    
    // NOTE: Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // NOTE: Update
        GameUpdate(gameState);        
    
        // NOTE: Draw
        BeginDrawing();

        ClearBackground(GRAY);

        GameRender(gameState);

        EndDrawing();
        
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    DeInitTexture();

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
