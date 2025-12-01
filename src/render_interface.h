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

struct StarFields
{
    bool8 initialized = false;
    Vector3 stars[STAR_COUNT];
    Vector2 starsScreenPos[STAR_COUNT];
    float flySpeed = 0.1f;    
};

//  ========================================================================
//              NOTE: Render Globals
//  ========================================================================

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
#if 0
                // NOTE: Draw tiles from id (and tile borders)
                DrawRectangle(
                    x * tileSize,
                    y * tileSize,
                    tileSize,
                    tileSize,
                    tileColor);
#endif
                
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

// TODO: parallelize the operation
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
    
    Vector3 * stars = starFields->stars;
    Vector2 * starsScreenPos = starFields->starsScreenPos;
    uint32 starCount = STAR_COUNT;
    float flySpeed = starFields->flySpeed;
        
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    static float time = 0;    
    float dt = GetFrameTime();
    time += dt;
    for (uint32 i = 0; i < starCount; i++)
    {
        // Update star's timer
        stars[i].z -= dt * flySpeed;
#if 0
        int random = GetRandomValue(INT_MIN, INT_MAX);

        if (random % 2 == 0)
        {
            stars[i].x += 0.5f * cosf(time);
        }
        else
        {
            stars[i].x += 0.5f * sinf(time);
        }
        
        random = GetRandomValue(INT_MIN, INT_MAX);

        if (random % 2 == 0)
        {
            stars[i].y += 0.5f * sinf(time);
        }
        else
        {
            stars[i].y += 0.5f * cosf(time);
        }
#endif
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

    for (uint32 i = 0; i < starCount; i++)
    {
        // Make the radius grow as the star ages
        float radius = Lerp(stars[i].z, 1.0f, 5.0f);
        Color color = ColorLerp(SKYBLUE, DARKPURPLE, stars[i].z);
        // Draw the circle
        DrawCircleV(starsScreenPos[i], radius, color);
    } 

}

#define RENDER_INTERFACE_H
#endif
