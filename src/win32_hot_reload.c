#include "WinDef.h"
#include "winbase.h"
#include "libloaderapi.h"

#if 0
typedef void HotReload(GameState *gameStateIn); // called when you recompile the program while its running
typedef void HotUnload(GameState *gameStateIn); // called before the dynamic libraries getswapped
void HotReloadStub(GameState * gameStateIn){}
void HotUnloadStub(GameState *gameStateIn){} 
#endif

// NOTE: prototypes for function pointers
typedef void Initialize(GameState *gameStateIn, Texture2D * textureIn); // called at the beginning of the app
typedef void UpdateAndRender(GameState * gameStateIn, Memory * gameMemoryIn, Texture2D * textureIn); // called on every frame

// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
void InitializeStub(GameState * gameStateIn, Texture2D * textureIn){}
void UpdateAndRenderStub(GameState * gameStateIn, Memory * gameMemoryIn, Texture2D * textureIn){}
typedef struct GameCode
{
#if 0
    HotReload *hotReload;
    HotUnload *hotUnload;
#endif
    
    HMODULE library;
    long lastDllWriteTime;
    bool isValid;

    // NOTE: pointers to functions from the dll
    Initialize *initialize;
    UpdateAndRender *updateAndRender;
} GameCode;

// Creates a copy of the main dll, and loads that copy
// if load fails it substitutes the loaded function with a stub(empty function)
static GameCode
GameCodeLoad(char *mainDllPath, char *tempDllPath, char *lockFilePath)
{
    // Load only after the lockfile gets deleted!
    // That way we won't load half compiled code. 
    while(FileExists(lockFilePath)) 
    {
        // You need to provide Sleep if you are porting to Linux
        // takes in milliseconds
        Sleep(50);
    }

    GameCode result;
    result.lastDllWriteTime = GetFileModTime(mainDllPath);

    // NOTE: Prevent locking the main dll by making a copy and loading that copy
    CopyFileA((LPCSTR)mainDllPath, (LPCSTR)tempDllPath, FALSE);
    
    result.library = LoadLibraryA(tempDllPath);
    result.isValid = 1;

    // NOTE: Load the functions from the game dll
    if (result.library)
    {
#if 0
        result.hotReload = (HotReload *)GetProcAddress(result.library, "HotReload");
        result.hotUnload = (HotUnload *)GetProcAddress(result.library, "HotUnload");
#endif
        
        result.initialize = (Initialize *)GetProcAddress(result.library, "Initialize");
        result.updateAndRender = (UpdateAndRender *)GetProcAddress(result.library, "UpdateAndRender");

        result.isValid = (result.updateAndRender != 0) && (result.initialize != 0);
    }

    // NOTE: if functions failed to load, load the empty functions
    if (result.isValid == 0)
    {

#if 0
        result.hotReload = HotReloadStub;
        result.hotUnload = HotUnloadStub;
#endif
        
        result.updateAndRender = UpdateAndRenderStub;
        result.initialize = InitializeStub;
        
        TraceLog(LOG_ERROR, "FAILED TO LOAD LIBRARY");
    }

    TraceLog(LOG_DEBUG, "GameCode valid? = %d", result.isValid);
    return result;
}

/* Unloads the dll and nulls the pointers to functions from the dll */
static void
GameCodeUnload(GameCode *GameCode)
{
    if (GameCode->library)
    {
#if 0
        GameCode->hotReload = HotReloadStub;
        GameCode->hotUnload = HotUnloadStub;
#endif
        FreeLibrary(GameCode->library);
        GameCode->library = 0;
        GameCode->initialize = InitializeStub;
        GameCode->updateAndRender = UpdateAndRenderStub;
        TraceLog(LOG_DEBUG, "Unload game code");
    }

    GameCode->isValid = false;
}
