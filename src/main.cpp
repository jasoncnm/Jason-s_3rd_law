/*
  NOTE:
  GAME_INTERNAL
  1 - Build for developer only
  0 - Build for pubilc release

*/



#if GAME_INTERNAL
#include "game.h"

#include "WinDef.h"
#include "winbase.h"
#include "platform.h"

#if defined _WIN32
#include "win32_hot_reload.cpp"
#elif defined __linux__
#error Linux build not supported
#elif defined __APPLE__
#error Apple build not supported
#endif

#else

#include "game.cpp"

#endif


#if defined _WIN32
#include "win32_memory.cpp"
#endif


#define PATH_SIZE 260

// NOTE: This file should be cross-compatible, one thing you need to provide
// if you want to do a linux version of this is providing a "Sleep(time)" function

int main(int argumentCount, char *argumentArray[])
{
#if GAME_INTERNAL

    SetTraceLogLevel(LOG_ALL);

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

#else
    SetTraceLogLevel(LOG_NONE);
#endif
    
    //--------------------------------------------------------------------------------------
    // NOTE: Memory Allocation
    //--------------------------------------------------------------------------------------
    
#if GAME_INTERNAL
     void * baseAddress = (void *)TB(2);

    size_t transientStorageSize = MB(500);
    size_t perminentStorageSize = GB(1);
    
    BumpAllocator persistentStorage = MakeBumpAllocator(baseAddress, perminentStorageSize);
    BumpAllocator transientStorage =
        MakeBumpAllocator((uint8 *)persistentStorage.memory + persistentStorage.capacity, transientStorageSize);
    #else
    
    BumpAllocator persistentStorage = MakeBumpAllocator(MB(500));
    BumpAllocator transientStorage = MakeBumpAllocator(MB(500));
    
#endif
    
    size_t gameStateSize = sizeof(GameState);
    
    gameState = (GameState *)BumpAlloc(&persistentStorage, gameStateSize);
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
        #if GAME_INTERNAL
        
        // if (IsWindowState(FLAG_VSYNC_HINT)) ClearWindowState(FLAG_VSYNC_HINT);
        // else SetWindowState(FLAG_VSYNC_HINT);
// SetWindowState(FLAG_WINDOW_TOPMOST);
        
        // SetTargetFPS(30);
#endif
        
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetWindowMonitor(0);
        SetExitKey(KEY_F4);  // IMPORTANT: DEBUG ONLY !!
         // MaximizeWindow();

        Image icon = LoadImage("Assets/ICON/ICON.png");
        if (IsImageValid(icon))
        {
            SetWindowIcon(icon); 
        } 
        UnloadImage(icon);
        
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
        
        for (uint32 shaderType = 0; shaderType < FX_COUNT; shaderType++)
        {
            if (FileExists(shaderPaths[shaderType]))
            {
                // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
                gameState->postFX[shaderType].shader =
                    LoadShader(VS_PATH, shaderPaths[shaderType]);
                if (!IsShaderValid(gameState->postFX[shaderType].shader))
                {
                    SM_ERROR("Unable to load shader file (%s)", 
                             shaderPaths[shaderType]);
                }
                gameState->postFX[shaderType].fsWriteTime = GetFileModTime(shaderPaths[shaderType]);
                gameState->postFX[shaderType].vsWriteTime = GetFileModTime(VS_PATH);
                }
        }
        
        gameState->renderTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
        if (!IsRenderTextureValid(gameState->renderTarget))
        {
            SM_ERROR("Unable to Load Render Target");
            }
        
        gameState->starFields.starTexture = LoadRenderTexture(24,24);
        if (!IsRenderTextureValid(gameState->starFields.starTexture))
        {
            SM_ERROR("Unable to Load star texture");
        }
        BeginTextureMode(gameState->starFields.starTexture);
        DrawCircle(12, 12, 10, WHITE);
        EndTextureMode();
        
         GenTextureMipmaps(&gameState->texture);
        SetTextureFilter(gameState->texture, TEXTURE_FILTER_POINT);
        
        gameState->currentScreen = TITLE_SCREEN;
        
    }

    bool8 running = true;

    //--------------------------------------------------------------------------------------
    // NOTE: Update Loop
    //--------------------------------------------------------------------------------------
    while(running)
    {
        running = !WindowShouldClose();
        memory.transientStorage->used = 0;
#if GAME_INTERNAL
        // NOTE: Check if the code got recompiled
        long dllFileWriteTime = GetFileModTime(mainDllPath);
        if (dllFileWriteTime != gameCode.lastDllWriteTime)
        {
            GameCodeUnload(&gameCode);
            gameCode = GameCodeLoad(mainDllPath, tempDllPath, lockFilePath);
        }
        gameCode.updateAndRender(gameState, &memory, &running);
#else
        UpdateAndRender(gameState, &memory, &running);

#endif
    }

    
    //--------------------------------------------------------------------------------------
    // NOTE: De-Initialization
    //--------------------------------------------------------------------------------------
    {
        CloseAudioDevice();
        CloseWindow();
        UnloadTexture(gameState->texture);
        UnloadRenderTexture(gameState->starFields.starTexture);
        } 
}
