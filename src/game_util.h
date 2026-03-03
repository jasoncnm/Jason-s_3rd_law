#if !defined(GAME_UTIL_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "engine_lib.h"
#define DIR_UP IVec2 { 0, -1 }
#define DIR_DOWN IVec2 { 0 ,1 }
#define DIR_LEFT IVec2 { -1, 0 }
#define DIR_RIGHT IVec2 { 1, 0 }


inline bool8 IsSlime(Entity * entity);

//  ========================================================================
//              NOTE: Game Util Structs
//  ========================================================================

//  ========================================================================
//              NOTE: Game Util Functions
//  ========================================================================
inline bool8 CheckOutOfBound(int32 tileX, int32 tileY)
{
    bool8 result =
    (tileX < gameState->tileMin.x)
        || (tileX > gameState->tileMax.x)
        || (tileY < gameState->tileMin.y)
        || (tileY > gameState->tileMax.y);
    
    return result;
}

inline bool8 CheckOutOfBound(IVec2 tilePos)
{
    return CheckOutOfBound(tilePos.x, tilePos.y);
}

IVec2 PixelPositionToTilePos(float x, float y)
{
    IVec2 result;
    result.x = (int32)((x + MAP_TILE_SIZE/2.0f) / MAP_TILE_SIZE);
    result.y = (int32)((y + MAP_TILE_SIZE/2.0f) / MAP_TILE_SIZE);
    
    return result;
}

IVec2 PixelPositionToTilePos(Vector2 pos)
{
    return PixelPositionToTilePos(pos.x, pos.y);
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
        
        Entity * attach = GetEntity(entity->attachedEntityIndex);
        if (IsDoor(attach) && (entity->tilePos == attach->tilePos))
        {
            topLeft -= Vector2 { (real32)entity->attachDir.x, (real32)entity->attachDir.y } * 5;
        }
    }
    
    return topLeft;
}


void DrawTile(IVec2 tilePos, Color color)
{
    Vector2 pivot = GetTilePivot(tilePos, MAP_TILE_SIZE);
    Rectangle rect = { pivot.x, pivot.y, MAP_TILE_SIZE, MAP_TILE_SIZE };
    DrawRectangleRec(rect, color);
}


IVec2 PivotToTilePos(Vector2 pivot, float tileSize)
{
    Vector2 center = Vector2Add(pivot, { tileSize * 0.5f, tileSize * 0.5f });
    center.x += MAP_TILE_SIZE * (center.x > 0 ? 0.5f : -0.5f);
    center.y += MAP_TILE_SIZE * (center.y > 0 ? 0.5f : -0.5f);
    return PixelPositionToTilePos(center);
}


Color IntToRGBA(uint32 val)
{
    
    unsigned char red   = (val >> 16) & 0xFF;
    unsigned char green = (val >> 8)  & 0xFF;
    unsigned char blue  = (val >> 0)  & 0xFF;
    
    Color color = { red, green, blue, 0xFF };
    
    return color;
}

const char * GetCameraState(MyCamera & camera)
{
    switch (camera.followState)
    {
        case MyCamera::LOCK_TO_MAP:
        {
            return "LOCK_TO_MAP";
        }
        case MyCamera::FOLLOW_WITHIN_MAP:
        {
            return "FOLLOW_WITHIN_MAP";
        }
        case MyCamera::FOLLOW_CENTER:
        {
            return "FOLLOW_CENTER";
        }
        case MyCamera::FOLLOW_ALONG_AXIS:
        {
            return "FOLLOW_ALONG_AXIS";
        }
    }
    
    return "none";
}

const char * GetEntityType(Entity * entity)
{
    if (entity)
    {
    switch (entity->type)
    {
        case ENTITY_TYPE_PLAYER:
        {
            return "ENTITY_TYPE_PLAYER";
        }
        case ENTITY_TYPE_CLONE:
        {
            return "ENTITY_TYPE_CLONE";
        }
        case ENTITY_TYPE_WALL:
        {
            return "ENTITY_TYPE_WALL";
        }
        case ENTITY_TYPE_BLOCK:
        {
            return "ENTITY_TYPE_BLOCK";
        }
        case ENTITY_TYPE_GLASS:
        {
            return "ENTITY_TYPE_GLASS";
        }
        case ENTITY_TYPE_ELECTRIC_DOOR:
        {
            return "ENTITY_TYPE_ELECTRIC_DOOR";
        }
        case ENTITY_TYPE_PIT:
        {
            return "ENTITY_TYPE_PIT";
        }
        case ENTITY_TYPE_TUT_PORTAL:
        {
            return "ENTITY_TYPE_TUT_PORTAL";
        }
        case ENTITY_TYPE_MAIN_PORTAL:
        {
            return "ENTITY_TYPE_MAIN_PORTAL";
        }
        case ENTITY_TYPE_SLIME_PORTAL:
        {
            return "ENTITY_TYPE_SLIME_PORTAL";
        }
        case ENTITY_TYPE_KEY:
        {
            return "ENTITY_TYPE_KEY";
        }
        case ENTITY_TYPE_LOCK:
        {
            return "ENTITY_TYPE_LOCK";
        }
        
        }
    }
    return "NONE";
}

#define GAME_UTIL_H
#endif
