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

Entity * AddEntityToMap(json & tileData, IVec2 tilePos, uint32 tileID)
{
    uint32 dataID = tileData["id"];
    SM_ASSERT(dataID == tileID - 1, "unmatched tileID");
    
    Entity addEntity = {};
    auto & tileProperties = tileData["properties"];
    
    for (uint32 pIdx = 0; pIdx < tileProperties.size(); pIdx++)
    {
        auto & prop = tileProperties[pIdx];
        std::string propName = prop["name"];
        
        if (propName == "type")
        {
            std::string typeName = prop["value"];
            addEntity.type = entityTypeMap[typeName];
            }
        else if (propName == "mass")
        {
            int8 mass = prop["value"];
            addEntity.mass = mass;
        }
        else if (propName == "left")
        {
            bool8 value = prop["value"];
            addEntity.left = value;
        }
        else if (propName == "right")
        {
            bool8 value = prop["value"];
            addEntity.right = value;
        }
        else if (propName == "up")
        {
            bool8 value = prop["value"];
            addEntity.up = value;
        }
        else if (propName == "down")
        {
            bool8 value = prop["value"];
            addEntity.down = value;
        }
        else if (propName == "main")
        {
            bool8 value = prop["value"];
            addEntity.mainCable = value;
        }
        else if (propName == "open_dir_x")
        {
             int32 value = prop["value"];
            addEntity.openDir.x = value;
        }
        else if (propName == "open_dir_y")
        {
             int32 value = prop["value"];
            addEntity.openDir.y = value;
        }
        
    }
    
    Entity * result = nullptr;
    
    if (addEntity.type != ENTITY_TYPE_NULL)
    {
    addEntity.tilePos = tilePos;
    addEntity.sprite = GetSprite(tileID);
        addEntity.active = true;
        addEntity.tileSize = DEFAULT_TILE_SIZE;
        addEntity.pivot = GetTilePivot(tilePos, DEFAULT_TILE_SIZE);
        
        addEntity.color = WHITE;
        
        if (IsCable(&addEntity))
        {
            addEntity.color = GRAY;
        }
    
        uint32 entityIndex = gameState->entities.Add(addEntity);
        result = &gameState->entities[entityIndex];
        result->entityIndex = entityIndex;
    }
    
    return result;
}

void GenerateTileMap(json & tilesData, json & layerData, 
                     IVec2 startPos, uint32 width, uint32 height)
{
    SM_ASSERT(layerData["type"] == "tilelayer", 
              "cannot generate tile map from non tile layer");
    
    std::string layerName = layerData["name"];
    std::vector<int32> mapData = layerData["data"];
    
    for (uint32 row = 0; row < height; row++)
    {
        for (uint32 col = 0; col < width; col++)
        {
            uint32 tileId = (uint32)mapData[col + row * width];
            
            if (tileId > 0)
            {
                json & tileData = tilesData[tileId - 1];
                
                uint32 dataID = tileData["id"];
                SM_TRACE("dataID: %d, tileId %d", dataID, tileId - 1);
                
                IVec2 tilePos = startPos + IVec2 { (int32)col, (int32)row };
                Entity * addEntity = AddEntityToMap(tileData, tilePos, tileId);
                
                if (!addEntity) continue;
                
                if (addEntity->type == ENTITY_TYPE_SLIME)
                {
                    if (layerName == "Player")
                    {
                        gameState->playerEntityIndex = addEntity->entityIndex;
                        SM_TRACE("Player generated at tile pos (%d, %d)", addEntity->tilePos.x, addEntity->tilePos.y);
                    }
                    else
                    {
                        addEntity->color = GRAY;
                    }
                    addEntity->tileSize = GetSlimeSize(addEntity->mass);
                    addEntity->pivot = GetTilePivot(addEntity);
                }
                else if (addEntity->type == ENTITY_TYPE_LOCK && layerName == "Lock")
                {
                    auto & mapProps = layerData["properties"];
                    for (auto & prop : mapProps)
                    {
                        if (prop["type"] == "int")
                        {
                            addEntity->unlockCount = prop["value"];
                        }
                    }
                }
            }
            
            }
    }
}

void SetupEntityTable()
{
    
    for (int32 layer = 0; layer < LAYER_COUNT; layer++)
    {
        gameState->entityTable[layer].Clear();
    }
    
    for (uint16 i = 0; i < gameState->entities.count; i++)
    {
        Entity * entity = GetEntity(i);
        if (entity)
        {
            switch(entity->type)
            {
                case ENTITY_TYPE_SLIME:
                {
                    gameState->entityTable[LAYER_SLIME].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_BRIDGE:
                case ENTITY_TYPE_WALL:
                {
                    gameState->entityTable[LAYER_WALL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_BLOCK:
                {
                    gameState->entityTable[LAYER_BLOCK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_GLASS:
                {
                    gameState->entityTable[LAYER_GLASS].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_DOOR:
                    {
                    gameState->entityTable[LAYER_DOOR].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_CABLE_WIRE:
                    {
                    gameState->entityTable[LAYER_CABLE].Add(entity->entityIndex);
                    break;
                    }
                case ENTITY_TYPE_CABLE_CONNECT:
                    {
                    gameState->entityTable[LAYER_CONNECTION].Add(entity->entityIndex);
                    break;
                    }
                case ENTITY_TYPE_CABLE_SOURCE:
                    {
                    gameState->entityTable[LAYER_SOURCE].Add(entity->entityIndex);
                    break;
                    }
                case ENTITY_TYPE_PIT:
                {
                    gameState->entityTable[LAYER_PIT].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_TUT_PORTAL:
                {
                    gameState->entityTable[LAYER_BLOCK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_MAIN_PORTAL:
                {
                    gameState->entityTable[LAYER_PORTAL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_SLIME_PORTAL:
                {
                    gameState->entityTable[LAYER_PORTAL].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_STAR:
                {
                    gameState->entityTable[LAYER_STAR].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_LOCK:
                {
                    gameState->entityTable[LAYER_LOCK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_CABLE_LINK:
                {
                    gameState->entityTable[LAYER_LINK].Add(entity->entityIndex);
                    break;
                }
                case ENTITY_TYPE_STAR_DEST:
                {
                    gameState->entityTable[LAYER_STAR_DEST].Add(entity->entityIndex);
                    break;
                }
                
                default:
                {
                    gameState->entityTable[LAYER_NULL].Add(entity->entityIndex);
                }
                }
        }
    }
    }

void LoadTileMapsAndEntities(char * worldPath)
{
    SM_TRACE("worldPath: %s", worldPath);
    uint32 tileCountX = 0, tileCountY = 0;
    IVec2 offset = { 50 - 12, 50 - 6 };

    IVec2 min = { INT_MAX, INT_MAX };
    IVec2 max = { INT_MIN, INT_MIN };
    gameState->currentMapIndex = -1;
    
    // NOTE: Load world
        std::ifstream f(worldPath);
    json worldData = json::parse(f);
    
    // NOTE: Load TileSet
    std::ifstream ts(TILESET_PATH);
    json tileSetData = json::parse(ts);
    auto tilesData = tileSetData["tiles"];
    
    auto tileMaps = worldData["maps"];
    gameState->tileMapCount = (int32)tileMaps.size();
int32 index = 0;
        for (uint32 i = 0; i < gameState->tileMapCount; i++)
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
                gameState->lv2Map = &gameState->tileMaps[index];
        }
        
        if (fileName == LAST_ROOM_NAME)
        {
            gameState->lastMap = &gameState->tileMaps[index];
        }
            
             IVec2 startPos = { startPosX, startPosY };
            
            // NOTE: Generate tile map    
            {
                json array = map["layers"];
                for (json::iterator it = array.begin(); it != array.end(); ++it)
                {
                    json & layer = *it;
                    if (!layer["visible"]) continue;
                    
                    std::string layerName = layer["name"];
                    
                    if (layerName == "level_info")
                    {
                        json properties = layer["properties"];
                        for (auto & prop : properties)
                        {
                            if ((prop["name"] == "visible_star_count") &&
                                (prop["type"] == "int"))
                            {
                                 tileMap.visibleStarCount = (int16)prop["value"];
                                break;
                            }
                        }
                        continue;
                    }
                    
                    if (layer["type"] == "tilelayer")
                    {
                        // tileCountX = width;
                    // tileCountY = height;
                    
                    GenerateTileMap(tilesData, layer, 
                                    startPos, mapWidth, mapHeight);
                }
                
                    SM_TRACE("%s layer loading done", layerName.data());
                    }
                SM_TRACE("Level width: %i, Level height: %i", mapWidth, mapHeight);
            }
            
            if (min.x > startPos.x) min.x = startPos.x;
            if (min.y > startPos.y) min.y = startPos.y;

            IVec2 dim = { mapWidth, mapHeight };

            IVec2 endPos = startPos + dim;
            
            if (max.x < endPos.x) max.x = endPos.x;
            if (max.y < endPos.y) max.y = endPos.y;
            
            gameState->tileMaps[index] = tileMap;
            
            index++;
        }

    // NOTE(Jason): 
    //animationPlaying = false;
    //animateSlimeCount = 0;

    {
        gameState->tileMin = min - IVec2 { 1, 1 };
        gameState->tileMax = max + IVec2 { 1, 1 };
        gameState->starCount = 0;
    }
    
    // Render texture to render fog of war
    // NOTE: To get an automatic smooth-fog effect we use a render texture to render fog
    // at a smaller size (one pixel per tile) and scale it on drawing with bilinear filtering
    Fog & fog = gameState->fog;
    if (!fog.initialized)
    {
        uint32 old_count = fog.dim.x * fog.dim.y;
        
        fog.initialized = true;
        fog.tileMin = gameState->tileMin - IVec2 { 20, 20 };
        fog.tileMax = gameState->tileMax + IVec2 { 20, 20 };
        fog.dim = fog.tileMax - fog.tileMin;
        
    fog.fogRenderTex = LoadRenderTexture(fog.dim.x, fog.dim.y);
    SetTextureFilter(fog.fogRenderTex.texture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(fog.fogRenderTex.texture, TEXTURE_WRAP_CLAMP);
        
        uint32 count = (fog.dim.x) * (fog.dim.y);
        if (!fog.fogPixels || count > old_count)
        {
        fog.fogPixels = (Color *)BumpAllocArray(gameMemory->persistentStorage, count, sizeof(Color));
        }
        
        for (uint32 i = 0; i < count; i++)
        {
            fog.fogPixels[i] = BLACK;
        }
        // memset(fog.fogPixels, BLANK, count * sizeof(*fog.fogPixels));
    }
    
    
    SetupEntityTable();
    SetUpElectricDoor();
    
    
    // NOTE: init stars
    auto starTable = gameState->entityTable[LAYER_STAR];
    if (!gameState->starT || starTable.count > gameState->starTCount)
    {
        gameState->starTCount = starTable.count;
        gameState->starT = (real32 *)BumpAllocArray(gameMemory->persistentStorage, starTable.count, sizeof(real32));
        }
    memset(gameState->starT, 0, starTable.count * sizeof(real32));
    
}



