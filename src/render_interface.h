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

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas.png"

//  ========================================================================
//              NOTE: Render Structs
//  ========================================================================

enum PostShaderType
{
    FX_GRAYSCALE = 0,
    FX_POSTERIZATION,
    FX_DREAM_VISION,
    FX_PIXELIZER,
    FX_CROSS_HATCHING,
    FX_CROSS_STITCHING,
    FX_PREDATOR_VIEW,
    FX_SCANLINES,
    FX_FISHEYE,
    FX_SOBEL,
    FX_BLOOM,
    FX_BLUR,
    FX_VIGNETTE,
    FX_JASON,
    FX_COUNT,
    //FX_FXAA
};

struct PostFX
{
    Shader shader;
    int frameBufferSizeLoc;
    long fileWriteTime;
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

static const char * shaderPaths[FX_COUNT] = 
{
        "Assets/Shaders/grayscale.fs",
        "Assets/Shaders/posterization.fs",
        "Assets/Shaders/dream_vision.fs",
        "Assets/Shaders/pixelizer.fs",
        "Assets/Shaders/cross_hatching.fs",
        "Assets/Shaders/cross_stitching.fs",
        "Assets/Shaders/predator.fs",
        "Assets/Shaders/scanlines.fs",
        "Assets/Shaders/fisheye.fs",
        "Assets/Shaders/sobel.fs",
    "Assets/Shaders/bloom.fs",
    "Assets/Shaders/blur.fs",
    "Assets/Shaders/vignette.fs",
    "Assets/Shaders/jason.fs",
};

//  ========================================================================
//              NOTE: Render Functions
//  ========================================================================

Rectangle GetCameraRect(Camera2D camera)
{
    Vector2 offset = Vector2Scale(camera.offset, 1.0f / camera.zoom);        
    Vector2 topleft = Vector2Subtract(camera.target, offset);

    Rectangle result = { topleft.x, topleft.y, offset.x * 2, offset.y * 2 };

    return result;
}

void DebugDrawPlayerActionState(ActionState state, int x, int y, int fontSize, Color color)
{
    char * stateTable[] = { 
        "NULL_STATE",
        "MOVE_STATE",
        "SPLIT_STATE",
        "ANIMATE_STATE",
        "FREEZE_STATE"
    };

    DrawText(TextFormat("Player Action State: %s", stateTable[state]), x, y, fontSize, color);
    
}

void DrawTileMap(Camera2D camera, IVec2 startPos, IVec2 dim, Color tileColor, Color gridColor)
{
    int tileSize = MAP_TILE_SIZE;

    IVec2 endPos = startPos + dim;    
    
    // NOTE: Draw Tile Maps
    for (int y = startPos.y; y < endPos.y; y++)
    {
        for (int x = startPos.x; x < endPos.x; x++)
        {
            Rectangle source = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };

            if (CheckCollisionRecs(source, GetCameraRect(camera)))
            {

                DrawRectangleLinesEx(source, .5f, gridColor);
                // DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, gridColor);
            }                     
        }
    }
}

void DrawSprite(Camera2D camera, Texture2D texture, Sprite & sprite, Vector2 topLeft, float tileSize = 32, Color color = WHITE)
{

    SM_ASSERT(IsTextureValid(texture), "Texture is not valid");

    float offset = 0.1f;
            
    Rectangle source =
        {
            // (float)sprite.altasOffset.x + 1, (float)sprite.altasOffset.y + 1,
            // (float)sprite.spriteSize.x - 2, (float)sprite.spriteSize.y -2
            (float)sprite.altasOffset.x + offset, (float)sprite.altasOffset.y + offset,
            (float)sprite.spriteSize.x - 2 * offset, (float)sprite.spriteSize.y - 2 * offset
        };

    Rectangle dest =
        {
            //topLeft.x + tileSize, topLeft.y + tileSize,
            topLeft.x, topLeft.y,
            tileSize, tileSize
        };

    if (CheckCollisionRecs(dest, GetCameraRect(camera)))
    {
        // Draw a part of a texture defined by a rectangle with 'pro' parameters
        DrawTexturePro(texture,  source,  dest, { 0, 0 }, 0, color);
        }
}

void DrawError()
{
    DrawText("SOMETHING IS WRONG PLEASE UNDO(Z) OR RESET(R)", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
}

void UpdateStarField(Vector3 * stars, Vector2 * starsScreenPos, float flySpeed, float dt,
                              int screenWidth, int screenHeight, uint32 start, uint32 end)
{
    for (uint32 i = start; i < end; i++)
    {
    // Update star's timer
    stars[i].z -= dt * flySpeed;
    // Calculate the screen position
        starsScreenPos[i] =
    {
        screenWidth *  0.5f + stars[i].x/stars[i].z,
        screenHeight * 0.5f + stars[i].y/stars[i].z,
    };
    
        // If the star is too old, or offscreen, it dies and we make a new random one
        if ((stars[i].z < 0.0f) || (starsScreenPos[i].x < 0) || (starsScreenPos[i].y < 0.0f) ||
            (starsScreenPos[i].x > screenWidth) || (starsScreenPos[i].y > screenHeight))
        {
            stars[i].x = (float)GetRandomValue(-screenWidth / 2, screenWidth / 2);
            stars[i].y = (float)GetRandomValue(-screenHeight / 2, screenHeight / 2);
            stars[i].z = 1.0f;
        }
        
    }
    }

void UpdateAndDrawStarFieldBG(StarFields * starFields)
{
    if (!starFields->initialized)
    {
        starFields->initialized = true;
        // Speed at which we fly forward
        starFields->flySpeed = 0.1f;
        // Setup the stars with a random position
        for (int i = 0; i < STAR_COUNT; i++)
        {
            starFields->stars[i].x = (float)GetRandomValue(-GetScreenWidth() / 2, GetScreenWidth() / 2);
            starFields->stars[i].y = (float)GetRandomValue(-GetScreenHeight() / 2, GetScreenHeight() / 2);
            starFields->stars[i].z = (float)GetRandomValue(0, INT_MAX) / INT_MAX;
        }
    }
    
    starFields->flySpeed = 0.2f;
    
    Vector3 * stars = starFields->stars;
    float flySpeed = starFields->flySpeed;
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    static float time = 0;    
    float dt = GetFrameTime();
    time += dt;
    
    uint32 chunck = STAR_COUNT / 4;
    
    Vector2 sp[STAR_COUNT] = {};
    Vector2 * starsScreenPos = sp;
    
    for (uint32 tid = 0; tid < 4; tid++)
    {
        int start = tid * chunck;
        int end = (tid + 1) * chunck;
        if (end > STAR_COUNT) end = STAR_COUNT;
        
        UpdateStarField(stars, starsScreenPos, flySpeed,  dt, screenWidth, screenHeight,
                        start, end);

    }
    
for (uint32 i = 0; i < STAR_COUNT; i++)
    {
         float radius = Lerp(stars[i].z, 1, 5);
        Color color = ColorLerp(DARKPURPLE, SKYBLUE, stars[i].z);
        DrawCircleV(starsScreenPos[i], radius, color);
    }
    
}

void UpdateAndRenderPostShader(RenderTexture2D & renderTarget, PostFX * postFX, 
                               int32 shaderType, int32 screenWidth, int32 screenHeight)
{
    
    if (GetFileModTime(shaderPaths[shaderType]) != 
        postFX[shaderType].fileWriteTime)
    {
        UnloadShader(postFX[shaderType].shader);
        postFX[shaderType].shader =
            LoadShader(0, shaderPaths[shaderType]);
        postFX[shaderType].frameBufferSizeLoc = 
            GetShaderLocation(postFX[shaderType].shader, "u_frameSize");
        if (!IsShaderValid(postFX[shaderType].shader))
        {
            SM_ERROR(false, "Unable to load shader file (%s)", 
                     "Assets/Shaders/bloom.fs");
        }
        postFX[shaderType].fileWriteTime = GetFileModTime(shaderPaths[shaderType]);
    }
    
    float size[2] =
    { 
        (float)screenWidth, (float)screenHeight
    };
    SetShaderValue(postFX[shaderType].shader, 
                   postFX[shaderType].frameBufferSizeLoc, 
                   size, SHADER_UNIFORM_VEC2);
    int offsetLoc = GetShaderLocation(postFX[shaderType].shader, "offset");
    
    static float val = 0; 
    
    val -= GetFrameTime() * 0.0015f;
    
    SetShaderValue(postFX[shaderType].shader, offsetLoc, &val, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(postFX[shaderType].shader);
    
    
    #if 0
    int mn = Min(screenWidth, screenHeight);
    DrawTextureRec(renderTarget.texture, 
                   {
                       0.5f * (screenWidth - mn), 0.5f * (screenHeight - mn), (float)mn, (float)-mn
                   }, 
                   {
                       0.5f * (screenWidth - mn), 0.5f * (screenHeight - mn)
                   }, WHITE);
#else
    DrawTextureRec(renderTarget.texture, 
                   {
                       0,
                       0, 
                       (float)renderTarget.texture.width, (float)-renderTarget.texture.height
                   }, 
                   {
                       0,
                       0
                   }, WHITE);
    
    #endif
    EndShaderMode();
    
}

#define RENDER_INTERFACE_H
#endif
