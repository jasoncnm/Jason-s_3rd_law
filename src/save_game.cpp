/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "save_game.h"
#include "vendor/Json/json.hpp"

using json = nlohmann::json;

void SaveGame(GameState & state, const char * savePath)
{
    EntityLayer saveLayers[] = { LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK };
    int32 saveEntityCount = 0;
    for (int32 i = 0; i < ArrayCount(saveLayers); i++)
    {
        saveEntityCount += state.entityTable[saveLayers[i]].count;
    }
    
    Entity * saveEntities = (Entity *)BumpAllocArray(gameMemory->transientStorage, saveEntityCount, sizeof(Entity));
    
    int32 index = 0;
    for (int32 layerIndex = 0; layerIndex < ArrayCount(saveLayers); layerIndex++)
    {
        auto & layer = state.entityTable[saveLayers[layerIndex]];
        for (uint32 i = 0; i < layer.count; i++)
        {
            SM_ASSERT(index < saveEntityCount, "Trying to write outside of allocated memory");
            Entity entity = state.entities[layer[i]];
            saveEntities[index++] = entity;
        }
    }
    
    for (uint32 saveIndex = 0; ; saveIndex++)
    {
        char saveName[10];
        sprintf(saveName, "Save_%d", saveIndex);
        
        char fileName[100];                
        CatStrings(GAME_SAVE_PATH, StringLength(GAME_SAVE_PATH),
                   saveName, StringLength(saveName),
                   fileName, 100);
        
        if (!FileExists(fileName))
        {
            // Save data to file from byte array (write), returns true on success
            if (!SaveFileData(fileName, (void *) saveEntities, sizeof(Entity) * saveEntityCount))
            {
                SM_ASSERT(false, "fail to save game state");
            }
            break;
        }
    } 
}

void LoadGame(SaveData & data)
{
    
}