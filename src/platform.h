#if !defined(PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
UPDATE_AND_RENDER(UpdateAndRenderStub) {}

typedef struct GameCode
{
    
    void * library;
    long lastDllWriteTime;
    bool8 isValid;

    // NOTE: pointers to functions from the dll
    update_and_render *updateAndRender;
} GameCode;

static GameCode
GameCodeLoad(char *mainDllPath, char *tempDllPath, char *lockFilePath);

static void
GameCodeUnload(GameCode *GameCode);


#define PLATFORM_H
#endif
