/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600


#include <cassert>
#include <stack>   
#include <sstream>
#include <iostream>

#include "game.cpp"

#include "raylib.h"

// NOTE: Program main entry point
int main(void)
{
    if (IsWindowState(FLAG_VSYNC_HINT)) ClearWindowState(FLAG_VSYNC_HINT);
    else SetWindowState(FLAG_VSYNC_HINT);

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");
    MaximizeWindow();

    InitTexture();
    
    BumpAllocator transientStorage = MakeBumpAllocator(MB(64));
    BumpAllocator persistentStorage = MakeBumpAllocator(MB(64));

    Memory gameMemory = { &transientStorage,  &persistentStorage };

    gameState = (GameState *)BumpAlloc(&persistentStorage, sizeof(GameState));
    if (!gameState)
    {
        SM_ERROR("Failed to allocate gameState");
        return -1;
    }
    
    // NOTE: Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        transientStorage.used = 0;
        // NOTE: Update
        GameUpdateAndRender(gameState, &gameMemory);        
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    DeInitTexture();

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
