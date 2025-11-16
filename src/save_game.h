#if !defined(SAVE_GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

struct SaveData
{
    IVec2 worldMin, worldMax;
    Entity * saveEntities;
    uint32 * saveEntityTable[LAYER_COUNT];
    Map * saveTileMap;
    uint32 * saveSourceIndices;
    uint32 * saveDoorIndices;
    uint32 * saveConnectionPointIndices;
    uint32 * saveEntityIndices;
};


#define SAVE_GAME_H
#endif
