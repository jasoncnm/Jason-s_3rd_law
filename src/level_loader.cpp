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

        state.entityTable[LAYER_PIT].Add(entityResult.entityIndex);

        SM_TRACE("Pit generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == WALL)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, SPRITE_WALL);

        state.entityTable[LAYER_WALL].Add(entityResult.entityIndex);
        
        SM_TRACE("Wall generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == BLOCK_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, SPRITE_BLOCK_2);
        entityResult.entity->mass = 2;
        entityResult.entity->movable = true;

        state.entityTable[LAYER_BLOCK].Add(entityResult.entityIndex);

        SM_TRACE("BLOCK 2 generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
        
    }
    else if (id == BLOCK)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, SPRITE_BLOCK);
        entityResult.entity->mass = 1;
        entityResult.entity->movable = true;

        state.entityTable[LAYER_BLOCK].Add(entityResult.entityIndex);

        SM_TRACE("BLOCK 1 generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == GLASS)
    {
        entityResult = AddEntity(ENTITY_TYPE_GLASS, tilePos, SPRITE_GLASS);

        state.entityTable[LAYER_GLASS].Add(entityResult.entityIndex);

        SM_TRACE("GLASS generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == PLAYER_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_CLONE, tilePos, SPRITE_SLIME_1);
        entityResult.entity->mass = 1;
        entityResult.entity->tileSize = GetSlimeSize(entityResult.entity);
        entityResult.entity->color = GRAY;

        entityResult.entity->movable = true;
        state.entityTable[LAYER_SLIME].Add(entityResult.entityIndex);
                                
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

        state.entityTable[LAYER_DOOR].Add(entityResult.entityIndex);
        
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

        state.entityTable[LAYER_DOOR].Add(entityResult.entityIndex);
        
        SM_TRACE("DOOR generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
        
    }
    else if (id == CABLE_DOWN_RIGHT || (id - 50) == CABLE_DOWN_RIGHT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_DOWN_RIGHT_OFF, false, true, false, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_H || (id - 50) == CABLE_H)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_H_OFF, true, true, false, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_DOWN_LEFT || (id - 50) == CABLE_DOWN_LEFT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_DOWN_LEFT_OFF, true, false, false, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_V || (id - 50) == CABLE_V)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_V_OFF, false, false, true, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_UP_RIGHT || (id - 50) == CABLE_UP_RIGHT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_UP_RIGHT_OFF, false, true, true, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_UP_LEFT || (id - 50) == CABLE_UP_LEFT)
    {
        entityResult = AddCable(tilePos, SPRITE_CABLE_UP_LEFT_OFF, true, false, true, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("CABLE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_RIGHT || (id - 50) == SOURCE_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_RIGHT_OFF, false, true, false, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_LEFT || (id - 50) == SOURCE_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_LEFT_OFF, true, false, false, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN || (id - 50) == SOURCE_DOWN)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_OFF, false, false, false, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP || (id - 50) == SOURCE_UP)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_OFF, false, false, true, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_H || (id - 50) == SOURCE_H)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_H_OFF, true, true, false, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_V || (id - 50) == SOURCE_V)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_V_OFF, false, false, true, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP_RIGHT || (id - 50) == SOURCE_UP_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_RIGHT_OFF, false, true, true, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_UP_LEFT || (id - 50) == SOURCE_UP_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_UP_LEFT_OFF, true, false, true, false);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN_RIGHT || (id - 50) == SOURCE_DOWN_RIGHT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_RIGHT_OFF, false, true, false, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == SOURCE_DOWN_LEFT || (id - 50) == SOURCE_DOWN_LEFT)
    {
        entityResult = AddSource(tilePos, SPRITE_SOURCE_DOWN_LEFT_OFF, true, false, false, true);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == CABLE_CONNECTION)
    {
        entityResult = AddConnection(tilePos, SPRITE_CABLE_CONNECTION);

        state.entityTable[LAYER_CABLE].Add(entityResult.entityIndex);
        
        SM_TRACE("SOURCE generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else
    {
        SM_ASSERT(false, "Unable to register ID (%d)", id);
    }
    return entityResult;
}

inline void GenerateTileMap(GameState & state, std::string fileName, IVec2 startPos, int width, int height)
{
    TileMapSrc & currentSrc = tileMapSources.last();
    
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
                        IVec2 offset = { col, row };
                        IVec2 tilePos = startPos + offset;

                        if (name == "Player")
                        {
                            result = AddEntity(ENTITY_TYPE_PLAYER, tilePos, SPRITE_SLIME_1);
                            result.entity->mass = 1;
                            result.entity->tileSize = GetSlimeSize(result.entity); 
                            result.entity->movable = true;

                            state.playerEntityIndex = result.entityIndex;

                            state.entityTable[LAYER_SLIME].Add(result.entityIndex);

                            currentSrc.mapEntitiesIndex.Add(result.entityIndex);
                                
                            SM_TRACE("Player generated (tile location: %i, %i)", result.entity->tilePos.x, result.entity->tilePos.y);
                        }
                        else
                        {
                            result = LoadGameObject(state, tileId, tilePos);
                            currentSrc.mapEntitiesIndex.Add(result.entityIndex);
                        }
                    }
                }
            }
            SM_TRACE("%s layer loading done", name.data());
        }

        SM_TRACE("Level width: %i, Level height: %i", width, height);
    }
}

void LoadLevelToGameState(GameState & state)
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
#if 0
        state.tileMaps = (Map *)BumpAllocArray(gameMemory->persistentStorage, state.tileMapCount, sizeof(Map));
#endif
        for (int index = 0; index < state.tileMapCount; index++)
        {
            json map = tileMaps[index];
            std::string fileName =  map["fileName"];
            
            int mapWidth = (int)map["width"] / MAP_TILE_SIZE;
            int mapHeight = (int)map["height"] / MAP_TILE_SIZE;
            int startPosX = (int)map["x"] / MAP_TILE_SIZE + 1;
            int startPosY = (int)map["y"] / MAP_TILE_SIZE + 1;

            Map tileMap = {};
            tileMap.tilePos = { startPosX, startPosY };
            tileMap.width = mapWidth, tileMap.height =  mapHeight;

            int id = state.tileMapCount;
            state.tileMaps[index] = tileMap;
            
            if (fileName == LEVEL_2_ROOM_NAME)
            {
                state.lv2Map = &state.tileMaps[index];
            }
            
            std::string path = LEVELS_PATH + fileName;
            char * c_str = (char *)path.data();
            tileMapSources.Add({id, mapWidth, mapHeight, c_str, GetTimestamp(c_str)});

            IVec2 startPos = { startPosX + 1, startPosY + 1 };

            GenerateTileMap(state, path, startPos, mapWidth, mapHeight);

            if (min.x > startPos.x) min.x = startPos.x;
            if (min.y > startPos.y) min.y = startPos.y;

            IVec2 dim = { mapWidth, mapHeight };

            IVec2 endPos = startPos + dim;
            
            if (max.x < endPos.x) max.x = endPos.x;
            if (max.y < endPos.y) max.y = endPos.y;
            
        }

    }

    //animationPlaying = false;
    //animateSlimeCount = 0;

    {
        state.tileMin = min;
        state.tileMax = max;
    } 

    // NOTE: Attatch slimes
    {
        auto & slimeEntityIndices = state.entityTable[LAYER_SLIME];
        
        for (int i = 0; i < slimeEntityIndices.count; i++)
        {
            int index = slimeEntityIndices[i];
            Entity * entity = GetEntity(index);
            SM_ASSERT(entity, "Entity is just created but not activate");
            
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

            for (int j = 0; j < 4; j++)
            {
                if (AttachSlime(entity, directions[j])) break;
            }
        }
        
    }
    
}
 
