/*
  NOTE:
  GAME_INTERNAL
  1 - Build for developer only
  0 - Build for pubilc release

*/



#if TEST
#include "test/test.cpp"
int main(void)
{
    test();
    return 0;
}
#else

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
    
    memset(gameState, 0, sizeof(GameState));
    
    Memory memory = { &transientStorage,  &persistentStorage };
    //--------------------------------------------------------------------------------------
    // NOTE: Initialization
    //--------------------------------------------------------------------------------------
    {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetWindowMonitor(0);
        int fps = GetMonitorRefreshRate(0);
        SetTargetFPS(fps);
#if GAME_INTERNAL
// SetWindowState(FLAG_WINDOW_TOPMOST);
        SetExitKey(KEY_F4);  // IMPORTANT: DEBUG ONLY !!
        MaximizeWindow();
#else
        SetExitKey(0);
        ToggleBorderlessWindowed();
#endif
        
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
        
        gameState->soundData = LoadSoundData(&persistentStorage);
        
        gameState->textureAltas = LoadTexture(TEXTURE_PATH); // Initialize Texture
        if (!IsTextureValid(gameState->textureAltas))
        {
            SM_ERROR("Unable to load file (%s) to texture", TEXTURE_PATH);
            return -1;
        }
        SetTextureFilter(gameState->textureAltas, TEXTURE_FILTER_POINT);
        
        gameState->bgTexture = LoadTexture(BACKGROUND_PATH);
        if (!IsTextureValid(gameState->bgTexture))
        {
            SM_ERROR("Unable to load file (%s) to texture", BACKGROUND_PATH);
            return -1;
        }
        SetTextureFilter(gameState->bgTexture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(gameState->bgTexture, TEXTURE_WRAP_REPEAT);
        
        if (!LoadShaderInfo(&gameState->postShader, POST_VS_PATH, POST_FS_PATH))
        {
            SM_ERROR(false, "Unable to load post shader");
            }
        
        if (!LoadShaderInfo(&gameState->movableShader, MOVE_VS_PATH, MOVE_FS_PATH))
        {
            SM_ERROR(false, "Unable to load player shader");
        }
        
        if (!LoadShaderInfo(&gameState->portalShader, BASE_VS_PATH, PORTAL_FS_PATH))
        {
            SM_ERROR(false, "Unable to load portal shader");
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
        
        gameState->fog = { 0 };
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
        CloseWindow();
        
        CloseAudioDevice();
        UnloadShaderInfo(&gameState->postShader);
        UnloadShaderInfo(&gameState->movableShader);
        UnloadShaderInfo(&gameState->portalShader);
        
        UnloadTexture(gameState->textureAltas);
        UnloadRenderTexture(gameState->starFields.starTexture);
        UnloadRenderTexture(gameState->fog.fogRenderTex);
    }
}
#endif
