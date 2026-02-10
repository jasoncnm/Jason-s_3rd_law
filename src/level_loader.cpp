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

inline AddEntityResult LoadGameObject(GameState & state, int32 id, IVec2 tilePos)
{
    AddEntityResult entityResult = { 0 };
    if (id == PIT)
    {
        entityResult = AddEntity(ENTITY_TYPE_PIT, tilePos, SPRITE_PIT);

        SM_TRACE("Pit generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == WALL_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, SPRITE_WALL_1);

        
        SM_TRACE("Wall generated (tile location: %i, %i)", entityResult.entity->tilePos.x, entityResult.entity->tilePos.y);
    }
    else if (id == WALL_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_WALL, tilePos, SPRITE_WALL_2);
        
        
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
    else if (id == TUT_1)
    {
        entityResult = AddEntity(ENTITY_TYPE_TUT_PORTAL, tilePos, SPRITE_TUT_1);
    }
    else if (id == TUT_2)
    {
        entityResult = AddEntity(ENTITY_TYPE_TUT_PORTAL, tilePos, SPRITE_TUT_2);
    }
    else if (id == MAIN_PORTAL)
    {
        entityResult = AddEntity(ENTITY_TYPE_MAIN_PORTAL, tilePos, SPRITE_MAIN_PORTAL);
    }
    else if (id == SLIME_PORTAL)
    {
        entityResult = AddEntity(ENTITY_TYPE_SLIME_PORTAL, tilePos, SPRITE_SLIME_PORTAL);
    }
    else
    {
        SM_ASSERT(false, "Unable to register ID (%d)", id);
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
                case ENTITY_TYPE_LOCK:
                {
                    state.entityTable[LAYER_KEY_LOCK].Add(entity->entityIndex);
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
        for (int32 i = 0; i < state.tileMapCount; i++)
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
            
            state.tileMaps[index] = tileMap;
            
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
                                int32 tileId = a[col + row * mapWidth];
                                
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
                                    else if (name == "Key")
                                    {
                                        result = AddEntity(ENTITY_TYPE_KEY, tilePos, SPRITE_KEY);
                                        
                                        Entity * key = result.entity;
                                        
                                        json properties = layer["properties"];
                                        for (auto & prop : properties)
                                        {
                                            if (prop["type"] == "file")
                                            {
                                                std::string fname = prop["value"];
                                                
                                                // const char * npath = GetDirectoryPath(path.c_str());
                                                
                                                std::string lockMapPath = "/" + fname;
                                                lockMapPath = GetDirectoryPath(path.c_str()) + lockMapPath;
                                                std::ifstream file(lockMapPath);
                                                json lockMap = json::parse(file);
                                                
                                                json lockMapMeta;
                                                for (uint32 mapIdx = 0; mapIdx < tileMaps.size(); mapIdx++)
                                                {
                                                    std::string p = tileMaps[mapIdx]["fileName"];
                                                    if (fname == FindFileNameFromPath(p))
                                                    {
                                                        lockMapMeta = tileMaps[mapIdx];
                                                        break;
                                                    }
                                                }
                                                int32 lockMapWidth =  (int32)lockMapMeta["width"] / tileWidth;
                                                int32 lockMapHeight = (int32)lockMapMeta["height"] / tileWidth;
                                                int32 lockStartPosX = (int32)lockMapMeta["x"] / tileWidth;
                                                int32 lockStartPosY = (int32)lockMapMeta["y"] / tileWidth;
                                                IVec2 lockMapStartPos = { lockStartPosX, lockStartPosY };
                                                for (auto & layer : lockMap["layers"])
                                                {
                                                    if (layer["name"] == "Lock")
                                                    {
                                                        std::vector<int32> lData = layer["data"];
                                                        for (int32 lrow = 0; lrow < lockMapWidth; lrow++)
                                                        {
                                                            for (int32 lcol = 0; lcol < lockMapHeight; lcol++)
                                                            {
                                                                int32 ltileId = lData[lcol + lrow * lockMapWidth];
                                                                if (ltileId == LOCK)
                                                                {
                                                                    IVec2 lOffset = { lcol, lrow };
                                                                    IVec2 lTilePos = lockMapStartPos + lOffset;
                                                                    
                                                                    AddEntityResult lResult = AddEntity(ENTITY_TYPE_LOCK, lTilePos, SPRITE_LOCK);
                                                                    
                                                                    lResult.entity->open = false;
                                                                    
                                                                    key->unlockEntityIndex = lResult.entity->entityIndex;
                                                                    
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                        break;
                                                    }
                                                    }
                                                
                                                break;
                                            }
                                        }
                                    }
                                    else if (name != "Lock")
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
    SetUpElectricDoor();
}
 