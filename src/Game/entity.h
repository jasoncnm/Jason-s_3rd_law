#if !defined(ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "move_animation.h"

enum EntityLayer
{
 
    LAYER_PLAYER,
    LAYER_BLOCKS,
    LAYER_OVERLAP,
    LAYER_GROUND,
    
    LAYER_COUNT,
};

enum EntityType
{
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_CLONE,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_GLASS,
    ENTITY_TYPE_ELECTRIC_DOOR,
    ENTITY_TYPE_PIT,

    ENTITY_TYPE_COUNT,
};

enum ActionState
{
    NULL_STATE,
    MOVE_STATE,
    SPLIT_STATE,
    FREEZE_STATE,
};


struct Entity
{
    
    EntityType type;
    CableType cableType;

    ActionState actionState = MOVE_STATE;
    
    Entity * interact;
    MoveAnimationQueue moveAniQueue;

    
    Sprite sprite;
    SpriteID spriteID;
    IVec2 tilePos;
    IVec2 attachDir = {0,0};
    Vec4 color;

    Vec2 pivot;
    
    int entityIndex;
    int attachedEntityIndex;

    int sourceIndex = -1;
    int rightIndex = -1, leftIndex = -1, upIndex = -1, downIndex = -1;
    
    float tileSize = 32.0f;

    int mass = 1;
    int maxMass = 3;
    
    bool movable = false;
    
    bool show = true;

    bool split = false;
    bool attach = false;

    bool broken = false;

    bool open = false;
    bool conductive = false;
    bool left = false, right = false, up = false, down = false;

    bool hover = false;
    
    bool active = false;
    
};

struct AddEntityResult
{
    Entity *entity;
    int entityIndex;
};

struct FindAttachableResult
{
    Entity * entity;
    bool has;
};

// NOTE: Get a pointer of the entity by entities array index 
inline Entity * GetEntity(int i);

inline AddEntityResult
AddEntity(EntityType type, IVec2 tilePos, SpriteID spriteID, Vec4 color, int tileSize);

inline AddEntityResult
AddCable(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down);

inline AddEntityResult
AddDoor(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down);

inline AddEntityResult
AddSource(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down);

inline AddEntityResult
AddConnection(IVec2 tilePos, SpriteID spriteID);

inline void DeleteEntity(Entity * entity);

inline void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir);

inline void SetActionState(Entity * entity, ActionState state);

inline void SetGlassBeBroken(Entity * glass);

inline Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime);

inline bool AttachSlime(Entity * slime, IVec2 dir);

inline FindAttachableResult FindAttachable(IVec2 tilePos, IVec2 attachDir);

inline Entity * FindEntityByLocationAndLayer(IVec2 pos, EntityLayer layer);

inline Entity * FindEntity(IVec2 pos);

inline Entity * FindSlime(IVec2 pos);

inline void UpdateSlimes();

inline void MoveTowardsUntilBlocked(Entity * entity, IVec2 dest, IVec2 dir);

void SetEntityPosition(Entity * entity, Entity * touchingEntity, IVec2 tilePos);

inline bool HasPit(IVec2 tilePos);

inline bool IsSlime(Entity * entity);


#define ENTITY_H
#endif
