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

#include "Game.h"
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

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================
long long levelsTimestamp;


//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================

void LoadGameObject(Record & record, int id, IVec2 tilePos)
{
    if (id == WALL)
    {
        Wall wall1 = { 0 };
        wall1.tile = tilePos;

        wall1.sprite = GetSprite(SPRITE_WALL);
                                
        record.walls.Add(wall1);

        SM_TRACE("Wall generated (tile location: %i, %i)", wall1.tile.x, wall1.tile.y);
    }
    else if (id == BLOCK)
    {
        Block block1 = { 0 };
        block1.tile = tilePos;

        block1.color = WHITE;

        block1.sprite = GetSprite(SPRITE_BLOCK);
    
        record.blocks.Add(block1);

        SM_TRACE("Block generated (tile location: %i, %i)", block1.tile.x, block1.tile.y);

    }
    else if (id == GLASS)
    {
        Glass glass = { 0 };
        glass.tile = tilePos;
        glass.sprite = GetSprite(SPRITE_GLASS);
        glass.id = SPRITE_GLASS;
        
        record.glasses.Add(glass);
    }
    else if (id == PLAYER_1)
    {
        Slime slime = { };

        slime.tile = tilePos;
        slime.mass = 1;
                                
        slime.sprite = GetSprite(SPRITE_SLIME_1);
        slime.pivot = GetTilePivot(slime.tile);
                        
        record.player.slimes.Add(slime);
                                
        SM_TRACE("Slime generated (tile location: %i, %i)", slime.tile.x, slime.tile.y);

    }
    else if (id >= DOOR_LEFT && id <= DOOR_DOWN)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.id = (SpriteID)(SPRITE_DOOR_LEFT_CLOSE + (id - DOOR_LEFT));
        cable.type = CABLE_TYPE_DOOR;
        cable.tile = tilePos;
        cable.open = false;
        cable.sprite = GetSprite(cable.id);

        if (id == DOOR_LEFT || id == DOOR_RIGHT)
            cable.left = cable.right = true;
        else if (id == DOOR_UP || id == DOOR_DOWN)
            cable.up = cable.down = true;

        record.electricDoorSystem.doorIndexes.Add(record.electricDoorSystem.cables.Add(cable));
        
    }
    else if (id == CABLE_DOWN_RIGHT || (id - 50) == CABLE_DOWN_RIGHT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_DOWN_RIGHT_OFF);
        cable.id = SPRITE_CABLE_DOWN_RIGHT_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.down = cable.right = true;

        record.electricDoorSystem.cables.Add(cable);
        
    }
    else if (id == CABLE_H || (id - 50) == CABLE_H)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_H_OFF);
        cable.id = SPRITE_CABLE_H_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.left = cable.right = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else if (id == CABLE_DOWN_LEFT || (id - 50) == CABLE_DOWN_LEFT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_DOWN_LEFT_OFF);
        cable.id = SPRITE_CABLE_DOWN_LEFT_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.down = cable.left = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else if (id == CABLE_V || (id - 50) == CABLE_V)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_V_OFF);
        cable.id = SPRITE_CABLE_V_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.up = cable.down = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else if (id == CABLE_UP_RIGHT || (id - 50) == CABLE_UP_RIGHT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_UP_RIGHT_OFF);
        cable.id = SPRITE_CABLE_UP_RIGHT_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.right = cable.up = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else if (id == CABLE_UP_LEFT || (id - 50) == CABLE_UP_LEFT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_UP_LEFT_OFF);
        cable.id = SPRITE_CABLE_UP_LEFT_OFF;
        cable.type = CABLE_TYPE_CONNECT;
        cable.tile = tilePos;
        cable.up = cable.left = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else if (id == SOURCE_RIGHT || (id - 50) == SOURCE_RIGHT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_RIGHT_OFF);
        cable.id = SPRITE_SOURCE_RIGHT_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.right = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_LEFT || (id - 50) == SOURCE_LEFT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_LEFT_OFF);
        cable.id = SPRITE_SOURCE_LEFT_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.left = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_DOWN || (id - 50) == SOURCE_DOWN)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_DOWN_OFF);
        cable.id = SPRITE_SOURCE_DOWN_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.down = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_UP || (id - 50) == SOURCE_UP)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_UP_OFF);
        cable.id = SPRITE_SOURCE_UP_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.up = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }

    else if (id == SOURCE_H || (id - 50) == SOURCE_H)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_H_OFF);
        cable.id = SPRITE_SOURCE_H_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.left = cable.right = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_V || (id - 50) == SOURCE_V)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_V_OFF);
        cable.id = SPRITE_SOURCE_V_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.up = cable.down = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_UP_RIGHT || (id - 50) == SOURCE_UP_RIGHT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_UP_RIGHT_OFF);
        cable.id = SPRITE_SOURCE_UP_RIGHT_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.up = cable.right = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_UP_LEFT || (id - 50) == SOURCE_UP_LEFT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_UP_LEFT_OFF);
        cable.id = SPRITE_SOURCE_UP_LEFT_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.up = cable.left = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_DOWN_RIGHT || (id - 50) == SOURCE_DOWN_RIGHT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_DOWN_RIGHT_OFF);
        cable.id = SPRITE_SOURCE_DOWN_RIGHT_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.down = cable.right = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == SOURCE_DOWN_LEFT || (id - 50) == SOURCE_DOWN_LEFT)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_SOURCE_DOWN_LEFT_OFF);
        cable.id = SPRITE_SOURCE_UP_OFF;
        cable.type = CABLE_TYPE_SOURCE;
        cable.tile = tilePos;
        cable.down = cable.left = true;

        record.electricDoorSystem.sourceIndexes.Add(record.electricDoorSystem.cables.Add(cable));
    }
    else if (id == CABLE_CONNECTION)
    {
        Cable cable = { 0 };
        cable.conductive = false;
        cable.sprite = GetSprite(SPRITE_CABLE_CONNECTION);
        cable.id = SPRITE_CABLE_CONNECTION;
        cable.type = CABLE_TYPE_CONNECTION_POINT;
        cable.tile = tilePos;
        cable.left = cable.right = cable.up = cable.down = true;

        record.electricDoorSystem.cables.Add(cable);
    }
    else
    {
        SM_ASSERT(false, "Unable to register ID");
    }
}

void GenerateTileMap(Record & currentRecord, std::string fileName, IVec2 startPos, int width, int height)
{
    // NOTE: Offsets start position
    startPos = startPos;
    
    // NOTE: Generate tile map    
    {
        std::ifstream f(fileName);
        json data = json::parse(f);

        json array = data["layers"];

        for (json::iterator it = array.begin(); it != array.end(); ++it)
        {
            json & layer = *it;

            if (!layer["visible"]) continue;
            
            std::string name = layer["name"];

            // tileCountX = width;
            // tileCountY = height;

            std::vector<int> a = layer["data"];

            SM_TRACE("Loading Layer: %s", name.data());

            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    int tileId = a[col + row * width];

                    if (tileId > 0)
                    {
                        IVec2 tilePos = startPos + (IVec2){ col, row };
                        if (name == "Player")
                        {
                            Slime mother = { };

                            mother.tile = tilePos;
                            mother.mass = 1;
                                
                            mother.sprite = GetSprite(SPRITE_SLIME_1);
                            mother.pivot = GetTilePivot(mother.tile);
                        
                            currentRecord.player.motherIndex = currentRecord.player.slimes.Add(mother);
                                
                            SM_TRACE("Player generated (tile location: %i, %i)", mother.tile.x, mother.tile.y);

                        }
                        else
                        {
                            LoadGameObject(currentRecord, tileId, tilePos);
                        }
                    }
                }
            }
            SM_TRACE("%s layer loading done", name.data());
        }

        SM_TRACE("Level width: %i, Level height: %i", width, height);
    }
}

void LoadLevelToGameState(GameState & state, State loadState)
{
    
    levelsTimestamp = GetTimestamp(LEVELS_PATH);

    Record & currentRecord = state.currentRecord;

    unsigned int tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };

    // NOTE: initilized player
    {
        Player player = { 0 };
        player.moveSteps = 1;
        player.color = WHITE;
            
        currentRecord.player = player;
    }

    // NOTE: Retrive TileMaps from world
    {
        
        std::ifstream f(WORLD_PATH);
        json worldData = json::parse(f);

        json tileMaps = worldData["maps"];

        for (json::iterator it = tileMaps.begin(); it != tileMaps.end(); ++it)
        {
            json map = *it;
            std::string fileName =  map["fileName"];
            int mapWidth = (int)map["width"] / MAP_TILE_SIZE;
            int mapHeight = (int)map["height"] / MAP_TILE_SIZE;
            int startPosX = (int)map["x"] / MAP_TILE_SIZE + 1;
            int startPosY = (int)map["y"] / MAP_TILE_SIZE + 1;

            Map tileMap = {};
            tileMap.tilePos = { startPosX, startPosY };
            tileMap.width = mapWidth, tileMap.height =  mapHeight;
            state.tileMaps.Add(tileMap);

            std::string path = LEVELS_PATH + fileName;
   
            IVec2 startPos = (IVec2){ startPosX + 1, startPosY + 1 };

            GenerateTileMap(currentRecord, path, startPos, mapWidth, mapHeight);

            if (min.x > startPos.x) min.x = startPos.x;
            if (min.y > startPos.y) min.y = startPos.y;

            IVec2 endPos = startPos + (IVec2){ mapWidth, mapHeight };
            
            if (max.x < endPos.x) max.x = endPos.x;
            if (max.y < endPos.y) max.y = endPos.y;
            
        }

    }

    animationPlaying = false;
    animateSlimeCount = 0;

    {
        state.tileMin = min;
        state.tileMax = max;
        
        // TODO: Need change

        // NOTE: Camera Setup

        Vector2 pos = TilePositionToPixelPosition(6, 6);
        
        state.camera.target = { pos.x, pos.y };
        state.camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        state.camera.rotation = 0.0f;
        state.camera.zoom = 3.5f;
    } 

    // NOTE: Attatch slimes
    {
        IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        for (int i = 0; i < 4; i++)
        {
            IVec2 pos = currentRecord.player.slimes[currentRecord.player.motherIndex].tile + directions[i];
            if (currentRecord.CheckWalls(pos) ||
                CheckTiles(pos, currentRecord.blocks))
            {
                currentRecord.player.slimes[currentRecord.player.motherIndex].attach = true;
                currentRecord.player.slimes[currentRecord.player.motherIndex].attachDir = directions[i];
                break;
            }
        }


        for (int i = 0; i < currentRecord.player.slimes.count; i++)
        {
            Slime & child = currentRecord.player.slimes[i];

            for (int j = 0; j < 4; j++)
            {
                IVec2 dir = directions[j];
                IVec2 pos = child.tile + dir;

                if (CheckTiles(pos, currentRecord.walls) ||
                    CheckTiles(pos, currentRecord.blocks))
                {
                    child.attach = true;
                    child.attachDir = dir;
                }
            }
            
        }
    }
    
    // NOTE: SetUp Electric Door
    currentRecord.electricDoorSystem.SetUp();

}

 

        
#if 0
        // NOTE: Create Level with string
        int size;
        unsigned char * fileData = LoadFileData(LEVELS_PATH, &size);
        levelsTimestamp = GetTimestamp(LEVELS_PATH);

        
        const char * level;

        state.state = loadState;
        bool loading = false;
        std::string l = "";

        int col = 0, row = 0;
            
        for (int i = 0; i < size; i++)
        {
            char c = fileData[i];

            if (loading)
            {
                if (c == ';') break;

                
                col++;

                switch(c)
                {
                    case '#':
                    {
                        // NOTE: Set Wall Inital States
                        currentRecord.wallSize = MAP_TILE_SIZE;
                        Wall wall1 = { 0 };
                        wall1.wallX = col;
                        wall1.wallY = row + 1;
                        currentRecord.walls[currentRecord.wallCount++] = wall1;
                        break;
                    }
                    case '@':
                    {
                        // NOTE: Set Player Inital States
                        Slime mother = { 0 };
                        
                        mother.tileX = col;
                        mother.tileY = row + 1;
                        mother.mass = 1;
                        mother.pivot = GetTilePivot(mother.tileX, mother.tileY, mother.tileSize);
                        
                        currentRecord.player.mother = mother;
                        
                        break;
                    }
                    case '1':
                    {

                        Slime child = { 0 };

                        child.tileX = col;
                        child.tileY = row + 1;
                        child.mass = 1;
                        child.tileSize = GetSlimeTileSize(currentRecord.player.maxTileSize, child.mass);
                        child.pivot = GetTilePivot(child.tileX, child.tileY, child.tileSize);

                        SM_ASSERT(currentRecord.player.childrenCount < (MAX_SLIME - 1), "Out of child capacity!!");
                        currentRecord.player.children[currentRecord.player.childrenCount++] = child;
                        
                        break;
                    }
                    case 'g':
                    {
                        
                        // NOTE:Set Goals Inital States
                        currentRecord.goalSize = MAP_TILE_SIZE / 4;

                        Goal goal = { 0 };
                        goal.goalX = col;
                        goal.goalY = row + 1;

                        currentRecord.goals[currentRecord.goalCount++] = goal;
                        break;

                    }
                    case 't':
                    {

                        // NOTE:Set Goals Inital States
                        currentRecord.goalSize = MAP_TILE_SIZE / 2;

                        Goal goal = { 0 };
                        goal.goalX = col;
                        goal.goalY = row + 1;
                        goal.cover = true;
                        currentRecord.goals[currentRecord.goalCount++] = goal;
                        
                        // NOTE: Set Block Inital States
                        currentRecord.blockSize = MAP_TILE_SIZE;

                        Block block1 = { 0 };
                        block1.blockX = col;
                        block1.blockY = row + 1;
                        block1.color = DARKGREEN;
    
                        currentRecord.blocks[currentRecord.blockCount++] = block1;
                        break;
                    }
                    case 'b':
                    {
                        // NOTE: Set Block Inital States
                        currentRecord.blockSize = MAP_TILE_SIZE;

                        Block block1 = { 0 };
                        block1.blockX = col;
                        block1.blockY = row + 1;

                        block1.color = GREEN;
    
                        currentRecord.blocks[currentRecord.blockCount++] = block1;
                        break;
                    }
                    case '\r':
                    case '\n':
                    {
                        if (fileData[i + 1] == '\n') i++; 
                        tileCountX = col - 1;
                        tileCountY++;
                        row++;
                        col = 0;
                        break;
                    }
                }
                            
            }
            else
            {
                if (c == '\r')
                {
                    switch(loadState)
                    {
                        case STATE_TEST_LEVEL:
                        {
                            if (l == "TestLevel")
                            {
                                loading = true;
                            }
                            break;
                        }
                    }
                    if (fileData[i + 1] == '\n') i++; 
                    l = "";
                }
                else
                {
                    l += c;
                }
            }
        }

        UnloadFileData(fileData);
#endif


#define LEVEL_LOADER_H
#endif
