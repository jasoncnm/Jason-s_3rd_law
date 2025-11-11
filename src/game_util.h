#if !defined(GAME_UTIL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "engine_lib.h"

//  ========================================================================
//              NOTE: Game Util Structs
//  ========================================================================
enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

//  ========================================================================
//              NOTE: Game Util Functions
//  ========================================================================
inline bool CheckOutOfBound(int tileX, int tileY)
{
    bool result =
    (tileX < gameState->tileMin.x)
        || (tileX > gameState->tileMax.x)
        || (tileY < gameState->tileMin.y)
        || (tileY > gameState->tileMax.y);
    
    return result;
}

inline bool CheckOutOfBound(IVec2 tilePos)
{
    return CheckOutOfBound(tilePos.x, tilePos.y);
}

Vector2 TilePositionToPixelPosition(float tileX, float tileY)
{

    Vector2 result;
    result.x = (float)tileX * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;
    result.y = (float)tileY * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;

    return result;

}

Vector2 TilePositionToPixelPosition(Vector2 tilePos)
{
    return TilePositionToPixelPosition(tilePos.x, tilePos.y);
}


Vector2 GetTilePivot(IVec2 tilePos, float tileSize, IVec2 attachDir = { 0, 0 })
{
    Vector2 playerPos = TilePositionToPixelPosition((float)tilePos.x, (float)tilePos.y);
    Vector2 topLeft = Vector2Subtract(playerPos, Vector2Scale(Vector2One(), tileSize * 0.5f));

    float dist = (MAP_TILE_SIZE - tileSize) * 0.5f;
    Vector2 offset = Vector2Scale({ (float)attachDir.x, (float)attachDir.y }, dist);
    topLeft = Vector2Add(topLeft, offset);                

    return topLeft;
}


Vector2 GetTilePivot(Entity * entity)
{
    Vector2 topLeft = GetTilePivot(entity->tilePos, entity->tileSize);
    
    if (IsSlime(entity) && entity->attach)
    {
        topLeft = GetTilePivot(entity->tilePos, entity->tileSize, entity->attachDir);
    }

    return topLeft;
}

Color IntToRGBA(unsigned int val)
{

    unsigned char red   = (val >> 16) & 0xFF;
    unsigned char green = (val >> 8) & 0xFF;
    unsigned char blue  = (val >> 0) & 0xFF;

    Color color = { red, green, blue, 0xFF };

    return color;
}

#define GAME_UTIL_H
#endif
