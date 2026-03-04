#if !defined(ASSETS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#include "engine_lib.h"

#define TileSetCols 10
//  ========================================================================
//              NOTE: Assets Constants
//  ========================================================================

enum TileID
{
    PLAYER_1 = 32,
    
    PLAYER_IDLE  = 102,
    PLAYER_UP    = 103,
    PLAYER_DOWN  = 104,
    PLAYER_LEFT  = 105,
    PLAYER_RIGHT = 106,
    
    WALL_1   = 34,
    WALL_2   = 35,
    
    BACKGROUND_1 = 141,
    
    BLOCK    = 33,
    BLOCK_2  = 12,
    GLASS    = 46,
    GLASS_BROKEN = 47,
    
    PIT      = 41,
    TUT_1    = 13,
    TUT_2    = 14,
    MAIN_PORTAL = 15,
    SLIME_PORTAL = 11,
    KEY = 21,
    LOCK = 31,
    
    BRIDGE_LEFT_A = 107,
    BRIDGE_LEFT_B = 117,
    BRIDGE_RIGHT_A = 108,
    BRIDGE_RIGHT_B = 118,
    BRIDGE_UP_A = 109,
    BRIDGE_UP_B = 110,
    BRIDGE_DOWN_A = 119,
    BRIDGE_DOWN_B = 120,
    
    
    DOOR_LEFT  = 82,
    DOOR_RIGHT = 83,
    DOOR_UP    = 84,
    DOOR_DOWN  = 85,
    
    DOOR_LEFT_R  = 92,
    DOOR_RIGHT_R = 93,
    DOOR_UP_R    = 94,
    DOOR_DOWN_R  = 95,
    
    CABLE_DOWN_RIGHT = 6,
    CABLE_H          = 7,
    CABLE_DOWN_LEFT  = 8,
    SOURCE_DOWN_RIGHT = 9,
    SOURCE_DOWN_LEFT  = 10,
    
    CABLE_V          = 16,
    SOURCE_UP_RIGHT   = 19,
    SOURCE_UP_LEFT     = 20,
    CABLE_UP_RIGHT   = 26,
    CABLE_UP_LEFT    = 28,
    SOURCE_V          = 29,
    SOURCE_H          = 30,
    SOURCE_RIGHT = 36,
    SOURCE_LEFT  = 37,
    SOURCE_DOWN  = 38,
    SOURCE_UP    = 48,
    CABLE_CONNECTION = 72,
    
};


//  ========================================================================
//              NOTE: Assets Structs
//  ========================================================================

struct Sprite
{
    IVec2 altasOffset;
    IVec2 spriteSize;
};

//  ========================================================================
//              NOTE: Assets Functions
//  ========================================================================

TileID GetCablePowerOnID(TileID tileID)
{
    if (tileID >= (50 + CABLE_DOWN_RIGHT))
    {
        return (TileID)(tileID - 50);
    }
    return tileID;
}

TileID GetCablePowerOffID(TileID tileID)
{
    if ((tileID) <= SOURCE_UP)
    {
        return (TileID)(tileID + 50);
    }
    return tileID;
}

Sprite GetSprite(TileID tileID)
{
    int id = tileID - 1;
     int32 row = id / TileSetCols;
     int32 col = id % TileSetCols;
    
    Sprite sprite = { 0 };
    sprite.altasOffset = IVec2 { col * 32, row * 32 };
    sprite.spriteSize = IVec2 { 32, 32 };
    
    return sprite;
}


#define ASSETS_H
#endif
