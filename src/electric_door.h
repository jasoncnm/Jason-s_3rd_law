#if !defined(ELECTRIC_DOOR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_CABLE 1000
struct Entity;

enum CableType
{
    CABLE_TYPE_NULL,
    
    CABLE_TYPE_SOURCE,
    CABLE_TYPE_CONNECT,
    CABLE_TYPE_CONNECTION_POINT,
    CABLE_TYPE_DOOR,

    CABLE_TYPE_COUNT,
};


struct ElectricDoorSystem
{
    Array<int, 100> sourceIndices;
    Array<int, 400> doorIndices;
    Array<int, 400> connectionPointIndices;
    Array<int, MAX_CABLE> entityIndices;

    inline void Update();
    
    inline void SetUp();

    inline bool DoorBlocked(IVec2 tilepos, IVec2 reachDir);
    
    inline bool CheckDoor(IVec2 tilePos);

    void Search(Array<bool, MAX_CABLE> & visited, int currentIndex, int sourceIndex);

    void OnSourcePowerOn(Array<bool, MAX_CABLE> & visited, int currentIndex);

};

inline bool SameSide(Entity * door, IVec2 tilePos, IVec2 reachDir);

inline void PowerOnCable(Entity * cable, bool & end);

inline void UnfreezeSlimes(Entity * door);

inline bool CanFreezeSlime(Entity * connect);

#define ELECTRIC_DOOR_H
#endif
