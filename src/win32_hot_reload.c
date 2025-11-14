#include "platform.h"

#include "WinDef.h"
#include "winbase.h"
#include "libloaderapi.h"

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
        
        result.updateAndRender = (update_and_render *)GetProcAddress((HMODULE)result.library, "UpdateAndRender");

        result.isValid = (result.updateAndRender != 0);
    }

    // NOTE: if functions failed to load, load the empty functions
    if (result.isValid == 0)
    {
        
        result.updateAndRender = UpdateAndRenderStub;
        
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
        FreeLibrary((HMODULE)GameCode->library);
        GameCode->library = 0;
        GameCode->updateAndRender = UpdateAndRenderStub;
        TraceLog(LOG_DEBUG, "Unload game code");
    }

    GameCode->isValid = false;
}
