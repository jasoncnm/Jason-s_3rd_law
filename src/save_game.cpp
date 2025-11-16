/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "save_game.h"

void SaveGame(GameState & state)
{
    SaveData saveData = {};
    saveData.worldMin = state.tileMin;
    saveData.worldMax = state.tileMax;

    uint32 entityTableCount[LAYER_COUNT];
    for (uint32 layer = 0; layer < LAYER_COUNT; layer++)
    {
        entityTableCount[layer] = state.entityTable[layer].count;
    }
    
    EntityLayer saveLayers[] = { LAYER_DOOR, LAYER_CABLE, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK };
    uint32 saveEntityCount = 0;
    for (uint32 i = 0; i < ArrayCount(saveLayers); i++)
    {
        saveEntityCount += state.entityTable[saveLayers[i]].count;
    }

    saveData.saveEntities = (Entity *)BumpAllocArray(gameMemory->transientStorage, saveEntityCount, sizeof(Entity));

    uint32 index = 0;
    for (uint32 layerIndex = 0; layerIndex < ArrayCount(saveLayers); layerIndex++)
    {
        auto & layer = state.entityTable[saveLayers[layerIndex]];
        for (uint32 i = 0; i < layer.count; i++)
        {
            SM_ASSERT(index < saveEntityCount, "Trying to write outside of allocated memory");
            Entity entity = state.entities[layer[i]];
            saveData.saveEntities[index++] = entity;
        }
    }

    
    
};
