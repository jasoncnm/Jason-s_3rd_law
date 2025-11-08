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

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas.png"
#define MAX_TRANSFORM 1000

//  ========================================================================
//              NOTE: Render Structs
//  ========================================================================


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
                // NOTE: Draw tiles from id (and tile borders)
                DrawRectangle(
                    x * tileSize,
                    y * tileSize,
                    tileSize,
                    tileSize,
                    tileColor);

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

#define RENDER_INTERFACE_H
#endif
