#if !defined(LEVEL_LOADER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include <fstream>
#include <iostream>
#include <vector>

#include "game.h"
#include "json.hpp"

using json = nlohmann::json;

// #define LEVELS_PATH "Assets/Level/levels.lv"
#define WORLD_PATH "Assets/Level_Editor/maps.world"
#define LEVELS_PATH "Assets/Level_Editor/"

//  ========================================================================
//              NOTE: Level structs
//  ========================================================================
enum TileID
{
    PLAYER_1 = 32,
    BLOCK    = 33,
    WALL     = 34,
    GLASS    = 46,
    
    DOOR_LEFT  = 82,
    DOOR_RIGHT = 83,
    DOOR_UP    = 84,
    DOOR_DOWN  = 85,

    CABLE_DOWN_RIGHT = 6,
    CABLE_H          = 7,
    CABLE_DOWN_LEFT  = 8,
    CABLE_V          = 16,
    CABLE_UP_RIGHT   = 26,
    CABLE_UP_LEFT    = 28,

    SOURCE_RIGHT = 36,
    SOURCE_LEFT  = 37,
    SOURCE_DOWN  = 38,
    SOURCE_UP    = 48,
    
    SOURCE_H          = 29,
    SOURCE_V          = 30,
    SOURCE_UP_RIGHT   = 19,
    SOURCE_UP_LEFT     = 20,
    SOURCE_DOWN_RIGHT = 9,
    SOURCE_DOWN_LEFT  = 10,
    
    
    CABLE_CONNECTION = 72,
};

struct TileMapSrc
{
    int mapID;

    char * fileName;
    long long timestamp;
};

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================
static Array<TileMapSrc, 100> tileMapSources;

#define LEVEL_LOADER_H
#endif
