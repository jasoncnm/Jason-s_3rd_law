#if !defined(ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "animation_controller.h"

enum EntityLayer
{
    LAYER_SLIME,
    LAYER_BLOCK,
    LAYER_WALL,
    LAYER_CABLE,
    LAYER_DOOR,
    LAYER_GLASS,
    LAYER_PIT,
    
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
    ANIMATE_STATE,
    FREEZE_STATE,
};


struct Entity
{
    
    EntityType type;
    CableType cableType;

    ActionState actionState = MOVE_STATE;

    AnimationController aniController;
    
    Sprite sprite;
    SpriteID spriteID;
    IVec2 tilePos;
    IVec2 attachDir = {0,0};
    Color color;

    Vector2 pivot;
    
    int entityIndex;
    int attachedEntityIndex;

    int sourceIndex = -1;
    int rightIndex = -1, leftIndex = -1, upIndex = -1, downIndex = -1;
    
    float tileSize = 32.0f;

    int mass = 1;
    int maxMass = 2;
    
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
AddEntity(EntityType type, IVec2 tilePos, SpriteID spriteID, Color color, int tileSize);

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

inline void UpdateSlimes();

inline bool AttachSlime(Entity * slime, IVec2 dir);

inline void SlimeMoveTowardsUntilBlocked(Entity * entity, IVec2 dest, IVec2 dir);

inline Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime);

inline FindAttachableResult FindAttachable(IVec2 tilePos, IVec2 attachDir);

inline Entity * FindEntityByLocationAndLayer(IVec2 pos, EntityLayer layer);

inline bool IsSlime(Entity * entity);

inline void SetEntityPosition(Entity * entity, Entity * touchingEntity, IVec2 tilePos);

void ShiftEntities(IVec2 startPos, IVec2 bounceDir);

inline bool CheckBounce(IVec2 tilePos, IVec2 pushDir);

void BounceEntity(Entity * startEntity, Entity * entity, IVec2 dir);

#define ENTITY_H
#endif
