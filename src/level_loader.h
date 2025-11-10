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
#include "vendor/Json/json.hpp"

using json = nlohmann::json;

#define WORLD_PATH "Assets/Level_Editor/maps.world"
#define LEVELS_PATH "Assets/Level_Editor/"
#define LEVEL_2_ROOM_NAME "TileMap/Room_24.tmj"

//  ========================================================================
//              NOTE: Level structs
//  ========================================================================
enum TileID
{
    PLAYER_1 = 32,
    BLOCK    = 33,
    WALL     = 34,
    BLOCK_2  = 12,
    GLASS    = 46,
    PIT      = 35,
    
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
    CABLE_V          = 16,
    CABLE_UP_RIGHT   = 26,
    CABLE_UP_LEFT    = 28,

    SOURCE_RIGHT = 36,
    SOURCE_LEFT  = 37,
    SOURCE_DOWN  = 38,
    SOURCE_UP    = 48,
    
    SOURCE_H          = 30,
    SOURCE_V          = 29,
    SOURCE_UP_RIGHT   = 19,
    SOURCE_UP_LEFT     = 20,
    SOURCE_DOWN_RIGHT = 9,
    SOURCE_DOWN_LEFT  = 10,
    
    
    CABLE_CONNECTION = 72,
};

struct TileMapSrc
{
    int mapID;

    int width, height;

    char * fileName;
    long long timestamp;

    Array<int, 500> mapEntitiesIndex;
};

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================
static Array<TileMapSrc, 100> tileMapSources;


//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================
inline AddEntityResult LoadGameObject(int id, IVec2 tilePos);

inline void GenerateTileMap(std::string fileName, IVec2 startPos, int width, int height);

void ReloadTileMap(TileMapSrc & mapSrc);

void LoadLevelToGameState(GameState & state);


#define LEVEL_LOADER_H
#endif
