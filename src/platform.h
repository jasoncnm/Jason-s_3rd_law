#if !defined(PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#if 0
typedef void HotReload(GameState *gameStateIn); // called when you recompile the program while its running
typedef void HotUnload(GameState *gameStateIn); // called before the dynamic libraries getswapped
typedef void Initialize(GameState *gameStateIn, Texture2D * textureIn); // called at the beginning of the app
void HotReloadStub(GameState * gameStateIn){}
void HotUnloadStub(GameState *gameStateIn){} 
void InitializeStub(GameState * gameStateIn, Texture2D * textureIn){}
#endif


// NOTE: prototypes for function pointers
typedef void UpdateAndRender(GameState * gameStateIn, Memory * gameMemoryIn); // called on every frame

// NOTE: empty functions meant to be replacements when
// functions from the dll fail to load
void UpdateAndRenderStub(GameState * gameStateIn, Memory * gameMemoryIn){}
typedef struct GameCode
{
#if 0
    HotReload *hotReload;
    HotUnload *hotUnload;
    Initialize *initialize;
#endif
    
    void * library;
    long lastDllWriteTime;
    bool isValid;

    // NOTE: pointers to functions from the dll
    UpdateAndRender *updateAndRender;
} GameCode;

static GameCode
GameCodeLoad(char *mainDllPath, char *tempDllPath, char *lockFilePath);

static void
GameCodeUnload(GameCode *GameCode);


#define PLATFORM_H
#endif
