/*
  NOTE:
  GAME_INTERNAL
  0 - Build for developer only
  1 - Build for pubilc release

  NOTE:
  BUILD_GAME_STATIC
    - if defined, build game code as a static library
        and link game code manualy to main (i.e #include"game code files")
    - otherwise, build game code as a dynamic library
        and load the game code using OS. 

*/

#if defined BUILD_GAME_STATIC

#include "game.cpp"

#else

#include "game.h"

#include "platform.h"
#if defined _WIN32
#include "win32_hot_reload.c"
#elif defined __linux__
#error Linux build not supported
#elif defined __APPLE__
#error Apple build not supported
#endif

#endif

#define PATH_SIZE 2048

// NOTE: This file should be cross-compatible, one thing you need to provide
// if you want to do a linux version of this is providing a "Sleep(time)" function

int main(int argumentCount, char *argumentArray[])
{
    
#if GAME_INTERNAL
    SetTraceLogLevel(LOG_NONE);
#else
    SetTraceLogLevel(LOG_ALL);
#endif

#if defined BUILD_GAME_STATIC

#else
    //--------------------------------------------------------------------------------------
    // NOTE: Game Code DLL Setup
    //--------------------------------------------------------------------------------------
    // NOTE: first argument of the argumentArray is the relative path
    //      to the executable
    const char *basePath = GetDirectoryPath(argumentArray[0]);
    char mainDllPath[PATH_SIZE];
    char tempDllPath[PATH_SIZE];
    char lockFilePath[PATH_SIZE];

    // NOTE: build paths to our runtime library and the lockfile
    {
        int bytesCopied;
        bytesCopied = TextCopy(mainDllPath, basePath);
        TextAppend(mainDllPath, "/game_code.dll", &bytesCopied);
        bytesCopied = TextCopy(tempDllPath, basePath);
        TextAppend(tempDllPath, "/game_code_temp.dll", &bytesCopied);
        bytesCopied = TextCopy(lockFilePath, basePath);
        TextAppend(lockFilePath, "/lock.file", &bytesCopied);

        TraceLog(LOG_INFO, basePath);
        TraceLog(LOG_INFO, mainDllPath);
        TraceLog(LOG_INFO, tempDllPath);
        TraceLog(LOG_INFO, lockFilePath);
    }
    
    GameCode gameCode = {0};
    gameCode = GameCodeLoad(mainDllPath, tempDllPath, lockFilePath);

#endif
    
    //--------------------------------------------------------------------------------------
    // NOTE: Memory Allocation
    //--------------------------------------------------------------------------------------
    BumpAllocator transientStorage = MakeBumpAllocator(MB(64));
    BumpAllocator persistentStorage = MakeBumpAllocator(MB(64));
    gameState = (GameState *)BumpAlloc(&persistentStorage, sizeof(GameState));
    if (!gameState)
    {
        SM_ERROR("Failed to allocate gameState");
        return -1;
    }
    Memory memory = { &transientStorage,  &persistentStorage };

    //--------------------------------------------------------------------------------------
    // NOTE: Initialization
    //--------------------------------------------------------------------------------------
    {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");

        if (IsWindowState(FLAG_VSYNC_HINT)) ClearWindowState(FLAG_VSYNC_HINT);
        else SetWindowState(FLAG_VSYNC_HINT);

        SetWindowState(FLAG_WINDOW_RESIZABLE);

        SetWindowMonitor(0);
    
        SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

        SetExitKey(KEY_Q);

        Image icon = LoadImage("Assets/ICON/ICON.png");
        if (IsImageValid(icon))
        {
            SetWindowIcon(icon); 
        } 
        // MaximizeWindow();

        InitAudioDevice();      // Initialize audio device
        if (!IsAudioDeviceReady())
        {
            SM_ERROR("Unable to initialize Audio Device");
            return -1;
        }
                
        gameState->texture = LoadTexture(TEXTURE_PATH); // Initialize Texture
        if (!IsTextureValid(gameState->texture))
        {
            SM_ERROR("Unable to load file (%s) to texture", TEXTURE_PATH);
            return -1;
        }

    }


    //--------------------------------------------------------------------------------------
    // NOTE: Update Loop
    //--------------------------------------------------------------------------------------
    while(!WindowShouldClose())
    {

#if defined BUILD_GAME_STATIC
        UpdateAndRender(gameState, &memory);
#else
        
        // NOTE: Check if the code got recompiled
        long dllFileWriteTime = GetFileModTime(mainDllPath);
        if (dllFileWriteTime != gameCode.lastDllWriteTime)
        {
            GameCodeUnload(&gameCode);
            gameCode = GameCodeLoad(mainDllPath, tempDllPath, lockFilePath);
        }
        gameCode.updateAndRender(gameState, &memory);

#endif
    }

    
    //--------------------------------------------------------------------------------------
    // NOTE: De-Initialization
    //--------------------------------------------------------------------------------------
    {
        CloseAudioDevice();
        CloseWindow();
        UnloadTexture(gameState->texture);
    } 
}
