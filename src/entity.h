#if !defined(ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "tween_controller.h"

enum EntityLayer
{
    LAYER_WALL,
    LAYER_DOOR,
    LAYER_CABLE,
    LAYER_GLASS,
    LAYER_SLIME,
    LAYER_BLOCK,
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

    TweenController tweenController;
    
    Sprite sprite;
    SpriteID spriteID;
    Color color;

    IVec2 tilePos;
    IVec2 attachDir = {0,0};

    Vector2 pivot;

    float tileSize = 32.0f;
    
    int entityIndex;
    int attachedEntityIndex;

    int sourceIndex = -1;
    int rightIndex = -1, leftIndex = -1, upIndex = -1, downIndex = -1;

    int mass = 1;
    int maxMass = 2;
    
    bool8 movable = false;

    bool8 attach = false;

    bool8 broken = false;

    bool8 open = false;
    bool8 conductive = false;
    bool8 left = false, right = false, up = false, down = false;
    bool8 sourceLit = false;
    bool8 hasPower = false;
    
    bool8 active = false;
};

struct AddEntityResult
{
    Entity *entity;
    int entityIndex;
};

struct FindAttachableResult
{
    Entity * entity;
    bool8 has;
};

struct MoveSlimeUntilBlockResult
{
    bool8 merged = false;
};

struct BounceEntityResult
{
    enum BounceState
    {
        STOP_AT_CONTACT_POS,
        PASS_THROUGHT_CONTACT_POS,
        PASS_THROUGHT_CONTACT_POS_AND_BROKE_GLASSES,
    };
    BounceState bounceState;
    
    IVec2 contactPos;
    IVec2 finalPos;

    Entity * contactEntity;

    Array<Entity *, 20> brokenGlasses;
};

void BounceEntity(Entity * entity, IVec2 dir);

void ShiftEntities(IVec2 startPos, IVec2 bounceDir);

inline void DeleteEntity(Entity * entity);

inline void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir);

inline void SetActionState(Entity * entity, ActionState state);

inline void SetGlassBeBroken(Entity * glass);

inline void UpdateSlimes();

inline void SetEntityPosition(Entity * entity, Entity * touchingEntity, IVec2 tilePos);

inline Entity * GetEntity(int i);

inline Entity * GetPlayer();

inline Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime);

inline Entity * FindEntityByLocationAndLayers(IVec2 pos, EntityLayer * layers, int arrayCount);

inline Entity * CreateSlimeClone(IVec2 tilePos);

inline FindAttachableResult FindAttachable(IVec2 tilePos, IVec2 attachDir);

inline bool8 AttachSlime(Entity * slime, IVec2 dir);

inline bool8 IsSlime(Entity * entity);

inline bool8 CheckBounce(IVec2 tilePos, IVec2 pushDir);

inline Rectangle GetEntityRect(Entity * entity);

inline AddEntityResult
AddEntity(EntityType type, IVec2 tilePos, SpriteID spriteID, Color color, int tileSize);

inline AddEntityResult
AddCable(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down);

inline AddEntityResult
AddDoor(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down);

inline AddEntityResult
AddSource(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down);

inline AddEntityResult
AddConnection(IVec2 tilePos, SpriteID spriteID);


#define ENTITY_H
#endif
