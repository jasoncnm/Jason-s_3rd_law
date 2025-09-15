/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1200


#include <cassert>
#include <stack>   
#include <sstream>
#include <iostream>

#include "Game.cpp"

#include "raylib.h"

// NOTE: Program main entry point
int main(void)
{
    if (IsWindowState(FLAG_VSYNC_HINT)) ClearWindowState(FLAG_VSYNC_HINT);
    else SetWindowState(FLAG_VSYNC_HINT);
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");

    InitTexture();

    
    BumpAllocator transientStorage = MakeBumpAllocator(MB(50));
    BumpAllocator persistentStorage = MakeBumpAllocator(MB(50));

    gameState = (GameState *)BumpAlloc(&persistentStorage, sizeof(GameState));
    if (!gameState)
    {
        SM_ERROR("Failed to allocate gameState");
        return -1;
    }
    
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
