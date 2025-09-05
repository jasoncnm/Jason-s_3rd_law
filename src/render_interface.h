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

//  ========================================================================
//              NOTE: Render Structs
//  ========================================================================


//  ========================================================================
//              NOTE: Render Globals
//  ========================================================================
Texture2D texture;

//  ========================================================================
//              NOTE: Render Functions
//  ========================================================================
void InitTexture()
{
    texture = LoadTexture(TEXTURE_PATH);
}

void DeInitTexture()
{
    UnloadTexture(texture);
}

void DrawTileMap(int tilesX, int tilesY, int tileSize, Color tileColor, Color gridColor)
{
    // NOTE: Draw Tile Maps
    for (unsigned int y = 0; y < tilesY; y++)
    {
        for (unsigned int x = 0; x < tilesX; x++)
        {
            // NOTE: Draw tiles from id (and tile borders)
            DrawRectangle(
                x * tileSize,
                y * tileSize,
                tileSize,
                tileSize,
                tileColor);
                    
            DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, gridColor);
                    
        }
    }
}

void DrawSprite(Texture2D & texture, Sprite & sprite, Vector2 topLeft, Color color = WHITE)
{
            
    Rectangle source = {
        (float)sprite.altasOffset.x, (float)sprite.altasOffset.y,
        (float)sprite.spriteSize.x, (float)sprite.spriteSize.y
    };
            
    DrawTextureRec(texture, source, topLeft, color);
}

#define RENDER_INTERFACE_H
#endif
