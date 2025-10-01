/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "level_loader.h"

//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================

inline AddEntityResult LoadGameObject(int id, IVec2 tilePos)
{
    AddEntityResult entityResult;
    if (id == PIT)
    {
        entityResult = AddEntity(ENTITY_TYPE_PIT, tilePos, SPRITE_PIT);
        SM_TRACE("Pit generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
        
    }
    else if (id == WALL)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, SPRITE_WALL);
        
        SM_TRACE("Wall generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == BLOCK_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, SPRITE_BLOCK_2);
        entityResult.entity->mass = 2;
        entityResult.entity->movable = true;

        SM_TRACE("BLOCK 2 generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
        
    }
    else if (id == BLOCK)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, SPRITE_BLOCK);
        entityResult.entity->mass = 1;
        entityResult.entity->movable = true;

        SM_TRACE("BLOCK 1 generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == GLASS)
    {
        entityResult = AddEntity(ENTITY_TYPE_GLASS, tilePos, SPRITE_GLASS);

        SM_TRACE("GLASS generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == PLAYER_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_CLONE, tilePos, SPRITE_SLIME_1);
        entityResult.entity->mass = 1;
        entityResult.entity->tileSize = (float)MAP_TILE_SIZE / entityResult.entity->maxMass;
        entityResult.entity->color = GRAY;

        entityResult.entity->movable = true;
        gameState->slimeEntityIndices.Add(entityResult.entityIndex);
                                
        SM_TRACE("CLONE 1 generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id >= DOOR_LEFT && id <= DOOR_DOWN)
    {
        if (id == DOOR_LEFT || id == DOOR_RIGHT)
        {
            entityResult = AddDoor(tilePos, (SpriteID)(SPRITE_DOOR_LEFT_CLOSE + (id - DOOR_LEFT)), true, true, false, false);
        }
        else if (id == DOOR_UP || id == DOOR_DOWN)
        {
            entityResult = AddDoor(tilePos, (SpriteID)(SPRITE_DOOR_LEFT_CLOSE + (id - DOOR_LEFT)), false, false, true, true);
        }
        
        SM_TRACE("DOOR generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_DOWN_RIGHT || (id - 50) == CABLE_DOWN_RIGHT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_DOWN_RIGHT_OFF, false, true, false, true);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_H || (id - 50) == CABLE_H)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_H_OFF, true, true, false, false);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_DOWN_LEFT || (id - 50) == CABLE_DOWN_LEFT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_DOWN_LEFT_OFF, true, false, false, true);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_V || (id - 50) == CABLE_V)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_V_OFF, false, false, true, true);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_UP_RIGHT || (id - 50) == CABLE_UP_RIGHT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_UP_RIGHT_OFF, false, true, true, false);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_UP_LEFT || (id - 50) == CABLE_UP_LEFT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_UP_LEFT_OFF, true, false, true, false);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_RIGHT || (id - 50) == SOURCE_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_RIGHT_OFF, false, true, false, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_LEFT || (id - 50) == SOURCE_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_LEFT_OFF, true, false, false, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN || (id - 50) == SOURCE_DOWN)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_OFF, false, false, false, true);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP || (id - 50) == SOURCE_UP)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_OFF, false, false, true, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_H || (id - 50) == SOURCE_H)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_H_OFF, true, true, false, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_V || (id - 50) == SOURCE_V)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_V_OFF, false, false, true, true);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP_RIGHT || (id - 50) == SOURCE_UP_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_RIGHT_OFF, false, true, true, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP_LEFT || (id - 50) == SOURCE_UP_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_LEFT_OFF, true, false, true, false);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN_RIGHT || (id - 50) == SOURCE_DOWN_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_RIGHT_OFF, false, true, false, true);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN_LEFT || (id - 50) == SOURCE_DOWN_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_LEFT_OFF, true, false, false, true);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_CONNECTION)
    {
        entityResult = AddConnection(tilePos, SPRITE_CABLE_CONNECTION);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else
    {
        SM_ASSERT(false, "Unable to register ID (%d)", id);
    }
    return entityResult;
}

inline void GenerateTileMap(std::string fileName, IVec2 startPos, int width, int height)
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
                        AddEntityResult result;
                        IVec2 tilePos = startPos + (IVec2){ col, row };

                        if (name == "Player")
                        {
                            result = AddEntity(ENTITY_TYPE_PLAYER, tilePos, SPRITE_SLIME_1);
                            result.entity->mass = 1;
                            result.entity->tileSize = (float)MAP_TILE_SIZE / result.entity->maxMass; 
                            result.entity->movable = true;

                            gameState->playerEntityIndex = result.entityIndex;
                            gameState->slimeEntityIndices.Add(result.entityIndex);

                            gameState->entityTable[LAYER_PLAYER].Add(result.entityIndex);
                                
                            SM_TRACE("Player generated (tile location: %i, %i)", result.entity->tilePos.x, result.entity->tilePos.y);
                        }
                        else if (name == "Blocks")
                        {
                            result = LoadGameObject(tileId, tilePos);
                            gameState->entityTable[LAYER_BLOCKS].Add(result.entityIndex);
                        }
                        else if (name == "Overlap")
                        {
                            result = LoadGameObject(tileId, tilePos);
                            gameState->entityTable[LAYER_OVERLAP].Add(result.entityIndex);
                        }
                        else
                        {
                            result = LoadGameObject(tileId, tilePos);
                            gameState->entityTable[LAYER_GROUND].Add(result.entityIndex);
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

    unsigned int tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };

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

            int id = state.tileMapCount;
            state.tileMaps[state.tileMapCount++] = tileMap;
            
            std::string path = LEVELS_PATH + fileName;
            char * c_str = (char *)path.data();
            tileMapSources.Add({id, c_str, GetTimestamp(c_str)});
   
            IVec2 startPos = (IVec2){ startPosX + 1, startPosY + 1 };

            GenerateTileMap(path, startPos, mapWidth, mapHeight);

            if (min.x > startPos.x) min.x = startPos.x;
            if (min.y > startPos.y) min.y = startPos.y;

            IVec2 endPos = startPos + (IVec2){ mapWidth, mapHeight };
            
            if (max.x < endPos.x) max.x = endPos.x;
            if (max.y < endPos.y) max.y = endPos.y;
            
        }

    }

    //animationPlaying = false;
    //animateSlimeCount = 0;

    {
        state.tileMin = min;
        state.tileMax = max;
        
        // NOTE: Camera Setup
        Vector2 pos = TilePositionToPixelPosition(6, 6);
        
        state.camera.target = { pos.x, pos.y };
        state.camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        state.camera.rotation = 0.0f;
        state.camera.zoom = 1.75f;
    } 

    // NOTE: Attatch slimes
    {
        for (int i = 0; i < gameState->slimeEntityIndices.count; i++)
        {
            int index = gameState->slimeEntityIndices[i];
            Entity * entity = GetEntity(index);
            SM_ASSERT(entity, "Entity is just created but not activate");
            
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

            for (int j = 0; j < 4; j++)
            {
                if (AttachSlime(entity, directions[j])) break;
            }
        }
        
    }
    
    // NOTE: SetUp Electric Door
    if (gameState->electricDoorSystem) SetUpElectricDoor();

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

