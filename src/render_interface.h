#if !defined(RENDER_INTERFACE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "assets.h"
#include "engine_lib.h"
#include "entity.h"

#define STAR_COUNT 100

#define BACKGROUND_PATH "Assets/Texture/background.png"

#define BASE_VS_PATH "Assets/Shaders/base.vs"
#define BASE_FS_PATH "Assets/Shaders/base.fs"
#define POST_VS_PATH "Assets/Shaders/PostShaders/post.vs"
#define POST_FS_PATH "Assets/Shaders/PostShaders/post.fs"
#define MOVE_VS_PATH "Assets/Shaders/EntityShaders/moveable.vs"
#define MOVE_FS_PATH "Assets/Shaders/EntityShaders/moveable.fs"
#define PORTAL_FS_PATH "Assets/Shaders/EntityShaders/portal.fs"

//  ========================================================================
//              NOTE: Render Structs
//  ========================================================================

struct ShaderInfo
{
    char vsPath[100];
     char fsPath[100];
    Shader shader;
    long fsWriteTime;
    long vsWriteTime;
};

struct StarFields
{
    RenderTexture starTexture;
    bool8 initialized = false;
    Vector3 stars[STAR_COUNT];
    float flySpeed = 0.1f;    
};

struct Particle
{
    Vector2 position;       // Particle position on screen
    Vector2 velocity;       // Particle current speed and direction
     real32 radius;           // Particle radius
    Color color;            // Particle color
    
     real32 lifeTime;         // Particle life time
    bool8 alive;             // Particle alive: inside screen and life time
};

struct CircularBuffer {
     real32 head;               // Index for the next write
     real32 tail;               // Index for the next read
    Particle *buffer;       // Particle buffer array
};
//  ========================================================================
//              NOTE: Render Globals
//  ========================================================================

//  ========================================================================
//              NOTE: Render Functions
//  ========================================================================

bool LoadShaderInfo(ShaderInfo * shaderInfo, const char * vsPath, const char * fsPath)
{
    if (FileExists(vsPath) && FileExists(fsPath))
    {
        TextCopy(shaderInfo->vsPath, vsPath);
        TextCopy(shaderInfo->fsPath, fsPath);
        shaderInfo->shader = LoadShader(vsPath, fsPath);
        if (!IsShaderValid(shaderInfo->shader)) return false;
        shaderInfo->vsWriteTime = GetFileModTime(vsPath);
        shaderInfo->fsWriteTime = GetFileModTime(fsPath);
        
        return true;
    }
    
    return false;
}

void UnloadShaderInfo(ShaderInfo * shaderInfo)
{
    if (IsShaderValid(shaderInfo->shader)) UnloadShader(shaderInfo->shader);
}


#define RENDER_INTERFACE_H
#endif
