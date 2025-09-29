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

template<typename T, int N>
bool CheckTiles(IVec2 tilePos, Array<T, N> arr)
{
    bool result = false;

    for (unsigned int index = 0; index < arr.count; index++)
    {
        if (arr[index].tile == tilePos)
        {
            result = true;
            break;
        }
    }

    return result;
}

template<typename T, int N>
int GetTileIndex(IVec2 tilePos, Array<T, N> tiles)
{
    for (unsigned int index = 0; index < tiles.count; index++)
    {
        if (tilePos == tiles[index].tile)
        {
            return index;
        }
    }
    return -1;
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


Vector2 GetTilePivot(int tileX, int tileY, float tileSize)
{
    Vector2 playerPos = TilePositionToPixelPosition(tileX, tileY);            

    return Vector2Subtract(playerPos, Vector2Scale(Vector2One(), tileSize * 0.5f));
}

Vector2 GetTilePivot(IVec2 tilePos, float tileSize)
{
    return GetTilePivot(tilePos.x, tilePos.y, tileSize);
}

#define GAME_UTIL_H
#endif
