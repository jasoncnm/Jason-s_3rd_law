#if !defined(GAME_UTIL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "game.h"
#include "engine_lib.h"

enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

Vec2 TilePositionToPixelPosition(float tileX, float tileY)
{

    Vec2 result;
    result.x = (float)tileX * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;
    result.y = (float)tileY * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;

    return result;

}

Vec2 TilePositionToPixelPosition(Vec2 tilePos)
{
    return TilePositionToPixelPosition(tilePos.x, tilePos.y);
}


Vec2 GetTilePivot(IVec2 tilePos, float tileSize, IVec2 attachDir = { 0, 0 })
{
    Vec2 playerPos = TilePositionToPixelPosition(tilePos.x, tilePos.y);
    Vec2 topLeft = Vec2Subtract(playerPos, Vec2Scale(Vec2One(), tileSize * 0.5f));

    float dist = (MAP_TILE_SIZE - tileSize) * 0.5f;
    Vec2 offset = Vec2Scale({ (float)attachDir.x, (float)attachDir.y }, dist);
    topLeft = Vec2Add(topLeft, offset);                

    return topLeft;
}

Vec2 GetTilePivot(Entity * entity)
{
    Vec2 topLeft = GetTilePivot(entity->tilePos, entity->tileSize);
    
    if (IsSlime(entity) && entity->attach)
    {
        topLeft = GetTilePivot(entity->tilePos, entity->tileSize, entity->attachDir);
    }

    return topLeft;
}

#define GAME_UTIL_H
#endif
