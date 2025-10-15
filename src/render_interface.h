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

void DrawTileMap(IVec2 startPos, IVec2 dim, Color tileColor, Color gridColor)
{
    int tileSize = MAP_TILE_SIZE;

    IVec2 endPos = startPos + dim;    
    
    // NOTE: Draw Tile Maps
    for (int y = startPos.y; y < endPos.y; y++)
    {
        for (int x = startPos.x; x < endPos.x; x++)
        {
            // NOTE: Draw tiles from id (and tile borders)
            DrawRectangle(
                x * tileSize,
                y * tileSize,
                tileSize,
                tileSize,
                tileColor);

            Rectangle source = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };
            DrawRectangleLinesEx(source, .5f, gridColor);
            // DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, gridColor);
                    
        }
    }
}

void DrawSprite(Texture2D texture, Sprite & sprite, Vector2 topLeft, float tileSize = 32, Color color = WHITE)
{

    SM_ASSERT(IsTextureValid(texture), "Texture is not valid");
            
    Rectangle source =
        {
            (float)sprite.altasOffset.x + 1, (float)sprite.altasOffset.y + 1,
            (float)sprite.spriteSize.x - 2, (float)sprite.spriteSize.y -2
        };

    Rectangle dest =
        {
            topLeft.x + tileSize, topLeft.y + tileSize,
            tileSize, tileSize
        };
        
    // Draw a part of a texture defined by a rectangle with 'pro' parameters
    DrawTexturePro(texture,  source,  dest, { dest.width, dest.height }, 0, color);
}


void DrawError()
{
    DrawText("SOMETHING IS WRONG PLEASE UNDO(Z) OR RESET(R)", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
}

#define RENDER_INTERFACE_H
#endif
