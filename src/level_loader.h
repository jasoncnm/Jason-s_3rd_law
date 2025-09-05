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
#define LEVELS_PATH "Assets/Level/level.tmj"

//  ========================================================================
//              NOTE: Level structs
//  ========================================================================
enum TileID
{
    PLAYER_1 = 17,
    BLOCK = 18,
    WALL = 19,
};

//  ========================================================================
//              NOTE: Level Globals
//  ========================================================================
long long levelsTimestamp;


//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================
void LoadLevelToGameState(GameState & state, State loadState)
{
    levelsTimestamp = GetTimestamp(LEVELS_PATH);

    // NOTE: Generate tile map    
    {

        state.currentRecord = { 0 };
        Record & currentRecord = state.currentRecord;

        // NOTE: initilized player
        {
            Player player = { 0 };
            player.moveSteps = 1;
            player.color = WHITE;
            
            currentRecord.player = player;
        }

        animationPlaying = false;
        animateSlimeCount = 0;
        
        Map map = { 0 };

        unsigned int tileCountX = 0, tileCountY = 0;
        
        
        std::ifstream f(LEVELS_PATH);
        json data = json::parse(f);

        json array = data["layers"];

        for (json::iterator it = array.begin(); it != array.end(); ++it)
        {
            json & layer = *it;

            std::string name = layer["name"];
            
            int width = layer["width"];
            int height = layer["height"];

            tileCountX = width;
            tileCountY = height;

            std::vector<int> a = layer["data"];

            SM_TRACE("Loading Layer: %s", name.data());
            
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    TileID tileId = (TileID)a[col + row * width];

                    if (tileId > 0)
                    {
                        if (name == "Blocks")
                        {
                            if (tileId == WALL)
                            {
            
                                Wall wall1 = { 0 };
                                wall1.tile = { col + 1, row + 1 };

                                wall1.sprite = GetSprite(SPRITE_WALL);
                                
                                currentRecord.walls.Add(wall1);

                                SM_TRACE("Wall generated (tile location: %i, %i)", wall1.tile.x, wall1.tile.y);
                                
                            }
                            else if (tileId == BLOCK)
                            {

                                Block block1 = { 0 };
                                block1.tile = { col + 1, row + 1 };

                                block1.color = WHITE;

                                block1.sprite = GetSprite(SPRITE_BLOCK);
    
                                currentRecord.blocks.Add(block1);

                                SM_TRACE("Block generated (tile location: %i, %i)", block1.tile.x, block1.tile.y);
                                
                            }
                        }
                        else if (name == "Player")
                        {
                            if (tileId == PLAYER_1)
                            {
                                Slime mother = { 0 };

                                mother.tile = { col + 1, row + 1 };
                                mother.mass = 1;
                                
                                mother.sprite = GetSprite(SPRITE_SLIME_1);
                                mother.pivot = GetTilePivot(mother.tile);
                        
                                currentRecord.player.mother = mother;
                                
                                SM_TRACE("Player generated (tile location: %i, %i)", mother.tile.x, mother.tile.y);
               
                            }

                        }
                    } 
                }
            }
            SM_TRACE("%s layer loading done", name.data());
        }

        SM_TRACE("Level width: %i, Level height: %i", tileCountX, tileCountY);
        
        // NOTE: Attatch slimes
        IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

        for (int i = 0; i < 4; i++)
        {
            IVec2 pos = currentRecord.player.mother.tile + directions[i];
            if (CheckTiles(pos, currentRecord.walls) ||
                CheckTiles(pos, currentRecord.blocks))
            {
                currentRecord.player.mother.attach = true;
                currentRecord.player.mother.attachDir = directions[i];
                break;
            }
        }

        for (int i = 0; i < currentRecord.player.children.count; i++)
        {
            Slime & child = currentRecord.player.children[i];

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
        
        map.tilesX = tileCountX;
        map.tilesY = tileCountY;

        state.tileMap = map;
        

        // NOTE: Camera Setup
        
        unsigned int tileX = tileCountX + 1;
        unsigned int tileY = tileCountY + 1;

        Vector2 pos = TilePositionToPixelPosition(tileX / 2.0f, tileY / 2.0f);
        
        state.camera.target = { pos.x, pos.y };
        state.camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        state.camera.rotation = 0.0f;
        state.camera.zoom = 3.0f;

    }
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
