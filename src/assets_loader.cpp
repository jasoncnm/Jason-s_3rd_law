/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "assets_loader.h"

//  ========================================================================
//              NOTE: Level Functions
//  ========================================================================

inline AddEntityResult LoadGameObject(GameState & state, TileID id, IVec2 tilePos)
{
    AddEntityResult entityResult = { 0 };
    if (id == PIT)
    {
        entityResult = AddEntity(ENTITY_TYPE_PIT, tilePos, id);

        }
    else if (id == WALL_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, id);

        }
    else if (id == WALL_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, id);
        }
    else if (id == BRIDGE_RIGHT_A || id == BRIDGE_RIGHT_B || id == BRIDGE_LEFT_A || id == BRIDGE_LEFT_B)
    {
        entityResult = AddDoor(tilePos, id, true, true, false, false);
    }
    else if (id == BRIDGE_UP_A || id == BRIDGE_UP_B || id == BRIDGE_DOWN_A || id == BRIDGE_DOWN_B)
    {
        entityResult = AddDoor(tilePos, id, false, false, true, true);
        }
    else if (id == BLOCK_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, id);
        entityResult.entity->mass = 2;
        entityResult.entity->movable = true;

    }
    else if (id == BLOCK)
    {
        entityResult = AddEntity(ENTITY_TYPE_BLOCK, tilePos, id);
        entityResult.entity->mass = 1;
        entityResult.entity->movable = true;

    }
    else if (id == GLASS)
    {
        entityResult = AddEntity(ENTITY_TYPE_GLASS, tilePos, id);
}
    else if (id == PLAYER_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_CLONE, tilePos, PLAYER_IDLE);
        entityResult.entity->mass = 1;
        entityResult.entity->tileSize = GetSlimeSize(entityResult.entity);
        entityResult.entity->color = GRAY;
entityResult.entity->movable = true;
                                
    }
    else if (id >= DOOR_LEFT && id <= DOOR_DOWN)
    {
        if (id == DOOR_LEFT || id == DOOR_RIGHT)
        {
            entityResult = AddDoor(tilePos, id, true, true, false, false);
        }
        else if (id == DOOR_UP || id == DOOR_DOWN)
        {
            entityResult = AddDoor(tilePos, id, false, false, true, true);
        }
        else
        {
            SM_ASSERT(false, "Possible Door id miss match (id %d)", id);
        }
        
        }
    else if (id >= DOOR_LEFT_R && id <= DOOR_DOWN_R)
    {
        if (id == DOOR_LEFT_R || id == DOOR_RIGHT_R)
        {
            entityResult = AddDoor(tilePos, id, true, true, false, false);
        }
        else if (id == DOOR_UP_R || id == DOOR_DOWN_R)
        {
            entityResult = AddDoor(tilePos, id, false, false, true, true);
        }
        else
        {
            SM_ASSERT(false, "Possible Door id miss match (id %d)", id);
        }
        }
    else if (id == CABLE_DOWN_RIGHT || (id - 50) == CABLE_DOWN_RIGHT)
    {
        entityResult = AddCable(tilePos, (TileID)(CABLE_DOWN_RIGHT + 50), false, true, false, true);
}
    else if (id == CABLE_H || (id - 50) == CABLE_H)
    {
        entityResult = AddCable(tilePos,(TileID)(CABLE_H + 50), true, true, false, false);
}
    else if (id == CABLE_DOWN_LEFT || (id - 50) == CABLE_DOWN_LEFT)
    {
        entityResult = AddCable(tilePos,(TileID)(CABLE_DOWN_LEFT + 50), true, false, false, true);
}
    else if (id == CABLE_V || (id - 50) == CABLE_V)
    {
        entityResult = AddCable(tilePos,(TileID)(CABLE_V + 50), false, false, true, true);
}
    else if (id == CABLE_UP_RIGHT || (id - 50) == CABLE_UP_RIGHT)
    {
        entityResult = AddCable(tilePos,(TileID)(CABLE_UP_RIGHT + 50), false, true, true, false);
}
    else if (id == CABLE_UP_LEFT || (id - 50) == CABLE_UP_LEFT)
    {
        entityResult = AddCable(tilePos,(TileID)(CABLE_UP_LEFT + 50), true, false, true, false);

    }
    else if (id == SOURCE_RIGHT || (id - 50) == SOURCE_RIGHT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_RIGHT + 50), false, true, false, false);

    }
    else if (id == SOURCE_LEFT || (id - 50) == SOURCE_LEFT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_LEFT + 50), true, false, false, false);

    }
    else if (id == SOURCE_DOWN || (id - 50) == SOURCE_DOWN)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_DOWN + 50), false, false, false, true);

    }
    else if (id == SOURCE_UP || (id - 50) == SOURCE_UP)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_UP + 50), false, false, true, false);
        
    }
    else if (id == SOURCE_H || (id - 50) == SOURCE_H)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_H + 50), true, true, false, false);
        
    }
    else if (id == SOURCE_V || (id - 50) == SOURCE_V)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_V + 50), false, false, true, true);
        
    }
    else if (id == SOURCE_UP_RIGHT || (id - 50) == SOURCE_UP_RIGHT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_UP_RIGHT + 50), false, true, true, false);
        
    }
    else if (id == SOURCE_UP_LEFT || (id - 50) == SOURCE_UP_LEFT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_UP_LEFT + 50), true, false, true, false);
        
        
        
    }
    else if (id == SOURCE_DOWN_RIGHT || (id - 50) == SOURCE_DOWN_RIGHT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_DOWN_RIGHT + 50), false, true, false, true);
        
        
    }
    else if (id == SOURCE_DOWN_LEFT || (id - 50) == SOURCE_DOWN_LEFT)
    {
        entityResult = AddSource(tilePos, (TileID)(SOURCE_DOWN_LEFT + 50), true, false, false, true);
        }
    else if (id == CABLE_CONNECTION)
    {
        entityResult = AddConnection(tilePos, CABLE_CONNECTION);
        }
    else if (id == TUT_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_TUT_PORTAL, tilePos, TUT_1);
    }
    else if (id == TUT_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_TUT_PORTAL, tilePos, TUT_2);
    }
    else if (id == MAIN_PORTAL)
    {
        entityResult = AddEntity(ENTITY_TYPE_MAIN_PORTAL, tilePos, MAIN_PORTAL);
    }
    else if (id == SLIME_PORTAL)
    {
        entityResult = AddEntity(ENTITY_TYPE_SLIME_PORTAL, tilePos, SLIME_PORTAL);
    }
    else if (id == KEY)
    {
        entityResult = AddEntity(ENTITY_TYPE_KEY, tilePos, id);
    }
    else if (id != LOCK)
    {
        SM_WARN("Unknown ID (%d)", id);
    }
    return entityResult;
}

void SetupEntityTable(GameState & state)
{
    
    for (int32 layer = 0; layer < LAYER_COUNT; layer++)
    {
        state.entityTable[layer].Clear();
    }
    
    for (uint16 i = 0; i < gameState->entities.count; i++)
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
                    if (entity->cableType == CABLE_TYPE_DOOR)
                    {
                        state.entityTable[LAYER_DOOR].Add(entity->entityIndex);
                    }
                    else if (entity->cableType == CABLE_TYPE_CONNECT)
                    {
                        state.entityTable[LAYER_CABLE].Add(entity->entityIndex);
                    }
                    else if (entity->cableType == CABLE_TYPE_CONNECTION_POINT)
                    {
                        state.entityTable[LAYER_CONNECTION].Add(entity->entityIndex);
                    }
                    else
                    {
                        state.entityTable[LAYER_SOURCE].Add(entity->entityIndex);
                    }
                    break;
                }
                case ENTITY_TYPE_PIT:
                {
                    state.entityTable[LAYER_PIT].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_TUT_PORTAL:
                {
                    state.entityTable[LAYER_BLOCK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_MAIN_PORTAL:
                {
                    state.entityTable[LAYER_PORTAL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_SLIME_PORTAL:
                {
                    state.entityTable[LAYER_PORTAL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_KEY:
                {
                    state.entityTable[LAYER_KEY].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_LOCK:
                {
                    state.entityTable[LAYER_LOCK].Add(entity->entityIndex);
                    break;
                }
                }
        }
    }
    
    
    
}

void LoadTileMapsAndEntities(GameState & state, char * worldPath)
{
    SM_TRACE("worldPath: %s", worldPath);
    uint32 tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };
    
    // NOTE: Retrive TileMaps from world
    {
state.currentMapIndex = -1;
        
        std::ifstream f(worldPath);
        json worldData = json::parse(f);

        auto tileMaps = worldData["maps"];
        
        state.tileMapCount = (int32)tileMaps.size();

        int32 index = 0;
        for (uint32 i = 0; i < state.tileMapCount; i++)
        {
            
            json mapMeta = tileMaps[i];
            std::string fileName = mapMeta["fileName"];
            const char * levelPath = GetDirectoryPath(worldPath);
            std::string path = "/" + fileName;
            path = levelPath + path;
            
            std::ifstream file(path);
            json map = json::parse(file);
            int32 tileWidth = (int32)map["tilewidth"];
            
            int32 mapWidth =  (int32)mapMeta["width"] / tileWidth;
            int32 mapHeight = (int32)mapMeta["height"] / tileWidth;
            int32 startPosX = (int32)mapMeta["x"] / tileWidth;
            int32 startPosY = (int32)mapMeta["y"] / tileWidth;
            
            std::string ID = FindFileNameFromPath(fileName).c_str();
            // Source - https://stackoverflow.com/a
            // Posted by Pixelchemist
            // Retrieved 2026-01-23, License - CC BY-SA 3.0
            std::string::size_type const p(ID.find_last_of('.'));
            std::string mapName = ID.substr(0, p);
            
            Map tileMap = {};
            strcpy(tileMap.mapID, mapName.c_str());
            tileMap.tilePos = { startPosX - 1, startPosY - 1 };
            tileMap.width = mapWidth, tileMap.height =  mapHeight;
            
            if (fileName == LEVEL_2_ROOM_NAME)
            {
                state.lv2Map = &state.tileMaps[index];
            }
            
            
             IVec2 startPos = { startPosX, startPosY };
            
            // NOTE: Generate tile map    
            {
                json array = map["layers"];
                for (json::iterator it = array.begin(); it != array.end(); ++it)
                {
                    json & layer = *it;
                    if (!layer["visible"]) continue;
                    
                    std::string name = layer["name"];
                    
                    if (name == "level_info")
                    {
                        json properties = layer["properties"];
                        for (auto & prop : properties)
                        {
                            if ((prop["name"] == "visible_star_count") &&
                                (prop["type"] == "int"))
                            {
                                 tileMap.visibleStarCount = (int32)prop["value"];
                                break;
                            }
                        }
                        continue;
                    }
                    
                    if (layer["type"] == "tilelayer")
                    {
                        // tileCountX = width;
                        // tileCountY = height;
                        
                        std::vector<int32> a = layer["data"];
                        
                        SM_TRACE("Loading Layer: %s", name.data());
                        
                        for (int32 row = 0; row < mapHeight; row++)
                        {
                            for (int32 col = 0; col < mapWidth; col++)
                            {
                                TileID tileId = (TileID)a[col + row * mapWidth];
                                
                                if (tileId > 0)
                                {
                                    AddEntityResult result;
                                    IVec2 offset = { col, row };
                                    IVec2 tilePos = startPos + offset;
                                    
                                    if (name == "Player")
                                    {
                                        
                                        result = AddEntity(ENTITY_TYPE_PLAYER, tilePos, PLAYER_IDLE);
                                        result.entity->mass = 1;
                                        result.entity->tileSize = GetSlimeSize(result.entity); 
                                        result.entity->movable = true;
                                        
                                        
                                        state.playerEntityIndex = result.entityIndex;
                                        
                                        SM_TRACE("Player generated (tile location: %i, %i)", result.entity->tilePos.x, result.entity->tilePos.y);
                                    }
                                    else if (name == "Lock")
                                    {
                                        result = AddEntity(ENTITY_TYPE_LOCK, tilePos, tileId);
                                        json properties = layer["properties"];
                                        for (auto & prop : properties)
                                        {
                                            if (prop["type"] == "int")
                                            {
                                                result.entity->unlockCount = (int32)prop["value"];
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        result = LoadGameObject(state, tileId, tilePos);
                                    }
                                }
                            }
                        }
                        
                    }
                    SM_TRACE("%s layer loading done", name.data());
                    
                }
                
                SM_TRACE("Level width: %i, Level height: %i", mapWidth, mapHeight);
            }
            state.tileMaps[index] = tileMap;
            
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
        state.starCount = 0;
    }
    
    SetupEntityTable(state);
    SetUpElectricDoor();
}



