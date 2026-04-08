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
