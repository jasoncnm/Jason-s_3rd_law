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

// TODO: separate world into test levels and main levels
#define MAIN_PATH "Assets/Level_Editor/main.world"
#define TEST_PATH "Assets/Level_Editor/TestLevels/test.world"

#define LEVEL_2_ROOM_NAME "TileMap/Room_59.tmj"

//  ========================================================================
//              NOTE: Level structs
//  ========================================================================

enum LoadOption
{
    LOAD_MAIN_LEVEL,
    LOAD_TEST_LEVEL_1,
    LOAD_TEST_LEVEL_2,
};

struct TileMapSrc
{
    int32 mapID;

    int32 width, height;

    char * fileName;
    long long timestamp;

    Array<int32, 500> mapEntitiesIndex;
};

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================


//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================

void ReloadTileMap(TileMapSrc & mapSrc);

void LoadTileMapsAndEntities(GameState & state, char * worldPath);


#define LEVEL_LOADER_H
#endif
