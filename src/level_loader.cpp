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

inline AddEntityResult LoadGameObject(GameState & state, int id, IVec2 tilePos)
{
    AddEntityResult entityResult = { 0 };
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
        entityResult.entity->tileSize = GetSlimeSize(entityResult.entity);
        entityResult.entity->color = GRAY;

        entityResult.entity->movable = true;
                                
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
        else
        {
            SM_ASSERT(false, "Possible Door id miss match (id %d)", id);
        }
        
        SM_TRACE("DOOR generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id >= DOOR_LEFT_R && id <= DOOR_DOWN_R)
    {
        if (id == DOOR_LEFT_R || id == DOOR_RIGHT_R)
        {
            entityResult = AddDoor(tilePos, (SpriteID)(SPRITE_DOOR_LEFT_R_CLOSE + (id - DOOR_LEFT_R)), true, true, false, false);
        }
        else if (id == DOOR_UP_R || id == DOOR_DOWN_R)
        {
            entityResult = AddDoor(tilePos, (SpriteID)(SPRITE_DOOR_LEFT_R_CLOSE + (id - DOOR_LEFT_R)), false, false, true, true);
        }
        else
        {
            SM_ASSERT(false, "Possible Door id miss match (id %d)", id);
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

inline void GenerateTileMap(GameState & state, json & map, IVec2 startPos, int width, int height)
{
        
    // NOTE: Offsets start position
    startPos = startPos;
    
    // NOTE: Generate tile map    
    {
        json array = map["layers"];

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
                        IVec2 offset = { col, row };
                        IVec2 tilePos = startPos + offset;

                        if (name == "Player")
                        {
                            result = AddEntity(ENTITY_TYPE_PLAYER, tilePos, SPRITE_SLIME_1);
                            result.entity->mass = 1;
                            result.entity->tileSize = GetSlimeSize(result.entity); 
                            result.entity->movable = true;

                            state.playerEntityIndex = result.entityIndex;

                                                            
                            SM_TRACE("Player generated (tile location: %i, %i)", result.entity->tilePos.x, result.entity->tilePos.y);
                        }
                        else
                        {
                            result = LoadGameObject(state, tileId, tilePos);
                            
                        }
                    }
                }
            }
            SM_TRACE("%s layer loading done", name.data());
        }

        SM_TRACE("Level width: %i, Level height: %i", width, height);
    }
}

void SetupEntityTable(GameState & state)
{
    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        state.entityTable[layer].Clear();
    }
    
    for (uint32 i = 0; i < gameState->entities.count; i++)
    {
        Entity * entity = GetEntity(i);
        if (entity)
        {
            switch(entity->type)
            {
                case ENTITY_TYPE_PLAYER:
                case ENTITY_TYPE_CLONE:
                {
                    state.entityTable[LAYER_SLIME].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_WALL:
                {
                    state.entityTable[LAYER_WALL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_BLOCK:
                {
                    state.entityTable[LAYER_BLOCK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_GLASS:
                {
                    state.entityTable[LAYER_GLASS].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_ELECTRIC_DOOR:
                {
                    state.entityTable[LAYER_DOOR].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_PIT:
                {
                    state.entityTable[LAYER_PIT].Add(entity->entityIndex);
                    break;
                }
            }
        }
    }
}

void LoadTileMapsAndEntities(GameState & state)
{

    unsigned int tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };

    // NOTE: Retrive TileMaps from world
    {

        state.currentMapIndex = -1;
        
        std::ifstream f(WORLD_PATH);
        json worldData = json::parse(f);

        auto tileMaps = worldData["maps"];

        state.tileMapCount = (int)tileMaps.size();

        int index = 0;
        for (int i = 0; i < state.tileMapCount; i++)
        {
            json map = tileMaps[i];
            std::string fileName =  map["fileName"];
            if (fileName == TEST_LEVEL_ONE_NAME) continue;            
            
            int mapWidth = (int)map["width"] / MAP_TILE_SIZE;
            int mapHeight = (int)map["height"] / MAP_TILE_SIZE;
            int startPosX = (int)map["x"] / MAP_TILE_SIZE + 1;
            int startPosY = (int)map["y"] / MAP_TILE_SIZE + 1;

            Map tileMap = {};
            tileMap.tilePos = { startPosX, startPosY };
            tileMap.width = mapWidth, tileMap.height =  mapHeight;

            state.tileMaps[index] = tileMap;
            
            if (fileName == LEVEL_2_ROOM_NAME)
            {
                state.lv2Map = &state.tileMaps[index];
            }
            
            std::string path = LEVELS_PATH + fileName;
            
            IVec2 startPos = { startPosX + 1, startPosY + 1 };

            std::ifstream file(path);
            json data = json::parse(file);

            GenerateTileMap(state, data, startPos, mapWidth, mapHeight);

            if (min.x > startPos.x) min.x = startPos.x;
            if (min.y > startPos.y) min.y = startPos.y;

            IVec2 dim = { mapWidth, mapHeight };

            IVec2 endPos = startPos + dim;
            
            if (max.x < endPos.x) max.x = endPos.x;
            if (max.y < endPos.y) max.y = endPos.y;

            index++;
        }

    }

    //animationPlaying = false;
    //animateSlimeCount = 0;

    {
        state.tileMin = min;
        state.tileMax = max;
    }
    
    SetupEntityTable(state);
}
 


void LoadTestLevel(GameState & state)
{

    unsigned int tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };

    // NOTE: Retrive TileMaps from world
    {

        state.currentMapIndex = -1;

        state.tileMapCount = 1;

        std::string fileName =  "Assets/Level_Editor/TileMap/Test.tmj";

        std::ifstream f(fileName);
        json map = json::parse(f);
                    
        int mapWidth = (int)map["width"];
        int mapHeight = (int)map["height"];
        int startPosX = 0;
        int startPosY = 0;

        Map tileMap = {};
        tileMap.tilePos = { startPosX, startPosY };
        tileMap.width = mapWidth, tileMap.height =  mapHeight;
        state.tileMaps[0] = tileMap;
        
        IVec2 startPos = { startPosX + 1, startPosY + 1 };

        auto & layers = map["layers"];
        for (int index = 0; index < layers.size(); index++)
        {
            auto & layer = layers[index];
            layer["visible"] = true;
        }
        
        GenerateTileMap(state, map, startPos, mapWidth, mapHeight);

        if (min.x > startPos.x) min.x = startPos.x;
        if (min.y > startPos.y) min.y = startPos.y;

        IVec2 dim = { mapWidth, mapHeight };

        IVec2 endPos = startPos + dim;
            
        if (max.x < endPos.x) max.x = endPos.x;
        if (max.y < endPos.y) max.y = endPos.y;

    }

    {
        state.tileMin = min;
        state.tileMax = max;
    } 

    SetupEntityTable(state);
     
}
