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

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas-10x.png"
#define FG_PATH "Assets/Texture/Backgrounds/5.png"
#define VS_PATH "Assets/Shaders/jason.vs"
#define BG_PATH "Assets/Texture/gradient.png"

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

void DebugDrawPlayerActionState(ActionState state, int32 x, int32 y, int32 fontSize, Color color)
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
    int32 tileSize = MAP_TILE_SIZE;

    IVec2 endPos = startPos + dim;    
    
    // NOTE: Draw Tile Maps
    for (int32 y = startPos.y; y < endPos.y; y++)
    {
        for (int32 x = startPos.x; x < endPos.x; x++)
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
        10.0f * ((float)sprite.altasOffset.x + offset), 10.0f * ((float)sprite.altasOffset.y + offset),
        10.0f * ((float)sprite.spriteSize.x - 2 * offset), 10.0f * ((float)sprite.spriteSize.y - 2 * offset)
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

void UpdateStarField(Vector3 * stars, Vector2 * starsScreenPos, Vector2 moveDir, float flySpeed, float dt,
                     int32 screenWidth, int32 screenHeight, int32 offsetX, int32 offsetY,
                     uint32 start, uint32 end)
{
    float moveSpeed = 2000;
    for (uint32 i = start; i < end; i++)
    {
        stars[i].x += moveDir.x * dt * moveSpeed;
        stars[i].y += moveDir.y * dt * moveSpeed;
    // Update star's timer
    stars[i].z -= dt * flySpeed;
    // Calculate the screen position
        starsScreenPos[i] =
    {
        screenWidth  * 0.5f + stars[i].x/stars[i].z,
        screenHeight * 0.5f + stars[i].y/stars[i].z,
    };
    
        // If the star is too old, or offscreen, it dies and we make a new random one
        if ((stars[i].z < 0.0f) || (starsScreenPos[i].x < 0) || (starsScreenPos[i].y < 0.0f) ||
            (starsScreenPos[i].x > screenWidth) || (starsScreenPos[i].y > screenHeight))
        {
            stars[i].x = (float)GetRandomValue(-screenWidth / 2 + offsetX,
                                               screenWidth / 2 - offsetX);
            stars[i].y = (float)GetRandomValue(-screenHeight / 2 + offsetY,
                                               screenHeight / 2 - offsetY);
            stars[i].z = 1.0f;
        }
        
    }
    }

void UpdateAndDrawStarFieldBG(StarFields * starFields, int32 offsetX = 0, int32 offsetY = 0, 
                              Vector2 moveDir = {0, 0})
{
    if (!starFields->initialized)
    {
        starFields->initialized = true;
        // Speed at which we fly forward
        starFields->flySpeed = 0.1f;
        // Setup the stars with a random position
        for (int32 i = 0; i < STAR_COUNT; i++)
        {
            starFields->stars[i].x = (float)GetRandomValue(-GetScreenWidth() / 2 + offsetX,
                                                           GetScreenWidth() / 2 - offsetX);
            starFields->stars[i].y = (float)GetRandomValue(-GetScreenHeight() / 2 + offsetY,
                                                           GetScreenHeight() / 2  - offsetY);
            starFields->stars[i].z = (float)GetRandomValue(0, INT_MAX) / INT_MAX;
        }
    }
    
    starFields->flySpeed = 0.2f;
    
    Vector3 * stars = starFields->stars;
    float flySpeed = starFields->flySpeed;
    
    int32 screenWidth = GetScreenWidth();
    int32 screenHeight = GetScreenHeight();
    static float time = 0;    
    float dt = GetFrameTime();
    time += dt;
    
    uint32 chunck = STAR_COUNT / 4;
    
    Vector2 sp[STAR_COUNT] = {};
    Vector2 * starsScreenPos = sp;
    
    for (uint32 tid = 0; tid < 4; tid++)
    {
        int32 start = tid * chunck;
        int32 end = (tid + 1) * chunck;
        if (end > STAR_COUNT) end = STAR_COUNT;
        
        UpdateStarField(stars, starsScreenPos, moveDir, flySpeed,  dt, screenWidth, screenHeight, offsetX, offsetY,
                        start, end);

    }
    
for (uint32 i = 0; i < STAR_COUNT; i++)
    {
        float radius = Lerp(stars[i].z, 1, 5);
        Color color = ColorLerp(DARKPURPLE, SKYBLUE, stars[i].z);
        DrawCircleV(starsScreenPos[i], radius * 1.5f, color);
    }
    
}

void UpdateAndRenderWithShader(RenderTexture2D & renderTarget, PostFX * postFX, 
                               int32 shaderType, int32 screenWidth, int32 screenHeight,
                                 int32 shake, float time)
{
    
    if (GetFileModTime(shaderPaths[shaderType]) != 
        postFX[shaderType].fsWriteTime ||
        GetFileModTime(VS_PATH) != postFX[shaderType].vsWriteTime)
    {
        UnloadShader(postFX[shaderType].shader);
        postFX[shaderType].shader =
            LoadShader(VS_PATH, shaderPaths[shaderType]);
        if (!IsShaderValid(postFX[shaderType].shader))
        {
            SM_ERROR(false, "Unable to load shader file (%s)", 
                     "Assets/Shaders/bloom.fs");
        }
        postFX[shaderType].fsWriteTime = GetFileModTime(shaderPaths[shaderType]);
        postFX[shaderType].vsWriteTime = GetFileModTime(VS_PATH);
        
    }
    
    float mn = (float)Min(screenWidth, screenHeight);
    
    float size[2] =
    { 
        (float)screenWidth, (float)screenHeight
    };
    
    int32 shakeLoc = GetShaderLocation(postFX[shaderType].shader, "shake");
    int32 timeLoc = GetShaderLocation(postFX[shaderType].shader, "time");
    int32 frameBufferSizeLoc = GetShaderLocation(postFX[shaderType].shader, "u_frameSize");
    int32 offsetLoc = GetShaderLocation(postFX[shaderType].shader, "offset");
    static float val = 0; 
    val -= GetFrameTime() * 0.0015f;
    
    
    SetShaderValue(postFX[shaderType].shader, frameBufferSizeLoc, size, SHADER_UNIFORM_VEC2);
    SetShaderValue(postFX[shaderType].shader, offsetLoc, &val, SHADER_UNIFORM_FLOAT);
    SetShaderValue(postFX[FX_JASON].shader, shakeLoc, &shake, SHADER_UNIFORM_INT);
    SetShaderValue(postFX[FX_JASON].shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
    
    
    BeginShaderMode(postFX[shaderType].shader);
    
    Rectangle source =
    {
        0, 0, (real32)screenWidth, -(real32)screenHeight
        //(screenWidth - mn) / 2, (screenHeight - mn) / 2, (float)mn, (float)-mn
    };
    
    Vector2 position = 
    {
        0, 0,
        //0.5f * (screenWidth - mn), 0.5f * (screenHeight - mn)
    };
    
    DrawTextureRec(renderTarget.texture, source, position, WHITE);
                   
    EndShaderMode();
    
}

// NOTE:
// Code from https://github.com/raysan5/raylib/blob/master/examples/textures/textures_tiled_drawing.c
// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;
    
    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                       Rectangle{dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, Rectangle{dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }
        
        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                           Rectangle{dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, Rectangle{dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }
        
        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                           Rectangle{dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }
            
            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                               Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }
        
        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                               Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }
            
            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                               Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}



#define RENDER_INTERFACE_H
#endif
