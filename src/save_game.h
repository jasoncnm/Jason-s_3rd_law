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
    enum SaveLoc
    {
        SAVE_MAIN,
        SAVE_TUT,
    };
    
    Entity * mainEntities;
    Entity * tutEntities;
    
    SaveLoc saveLoc;
    
    int mainEntCount;
    int tutEntCount;
    
    int playerIndex;
    int lastTutBlockIndex;
    };


#define SAVE_GAME_H
#endif
