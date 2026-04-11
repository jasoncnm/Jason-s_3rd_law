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

#define TILESET_PATH "Assets/Level_Editor/TileSet/game_tilesets.tsj"

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

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================
static std::unordered_map<std::string, EntityType> 
entityTypeMap
{
    
    {"slime", ENTITY_TYPE_SLIME},
    {"wall", ENTITY_TYPE_WALL},
    {"block", ENTITY_TYPE_BLOCK},
    {"bridge", ENTITY_TYPE_BRIDGE},
    {"glass", ENTITY_TYPE_GLASS},
    {"tut_portal", ENTITY_TYPE_TUT_PORTAL},
    {"main_portal", ENTITY_TYPE_MAIN_PORTAL},
    {"slime_portal", ENTITY_TYPE_SLIME_PORTAL},
    {"star", ENTITY_TYPE_STAR},
    {"lock", ENTITY_TYPE_LOCK},
    {"door", ENTITY_TYPE_DOOR},
    {"cable_wire", ENTITY_TYPE_CABLE_WIRE},
    {"cable_connect", ENTITY_TYPE_CABLE_CONNECT},
    {"cable_source", ENTITY_TYPE_CABLE_SOURCE},
    {"cable_link", ENTITY_TYPE_CABLE_LINK},
    
    {"null", ENTITY_TYPE_NULL},
    {"error", ENTITY_TYPE_NULL},
    };


//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================
void LoadTileMapsAndEntities(GameState & state, char * worldPath);


#define LEVEL_LOADER_H
#endif
