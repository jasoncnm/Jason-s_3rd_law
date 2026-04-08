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
    LAYER_SOURCE,
    LAYER_CONNECTION,
    LAYER_GLASS,
    LAYER_SLIME,
    LAYER_BLOCK,
    LAYER_PIT,
    LAYER_PORTAL,
    LAYER_KEY,
    LAYER_LOCK,
    LAYER_COUNT,
};

enum EntityType
{
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_CLONE,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_BRIDGE,
    ENTITY_TYPE_GLASS,
    ENTITY_TYPE_ELECTRIC_DOOR,
    ENTITY_TYPE_PIT,
    ENTITY_TYPE_TUT_PORTAL,
    ENTITY_TYPE_MAIN_PORTAL,
    ENTITY_TYPE_SLIME_PORTAL,
    ENTITY_TYPE_KEY,
    ENTITY_TYPE_LOCK,
    ENTITY_TYPE_COUNT,
};

enum ActionState
{
    NULL_STATE,
    MOVE_STATE,
    ANIMATE_STATE,
    FREEZE_STATE,
};

enum SpriteType
{
    SPRITE_TYPE_SPRITE = 0,
    SPRITE_TYPE_ANIMATED,
};


struct Entity
{
    EntityType type;
    CableType cableType;
    
    SpriteType spriteType;
    Sprite sprite;
        
    ActionState actionState = MOVE_STATE;
    
    TileID tileID;
    Color color;
    
    IVec2 tilePos;
    IVec2 attachDir = {0,0};
    
    Vector2 pivot;
    Vector2 tileSize;
    
    Vector2 screenStart;
    Vector2 screenEnd;
     
    int32 sourceIndex = -1;
    int32 rightIndex = -1, leftIndex = -1, upIndex = -1, downIndex = -1;
    
    uint32 attachedEntityIndex;
    uint32 unlockEntityIndex;
    uint32 unlockCount = 0;
    uint32 entityIndex;
    
    int8 mass = 1;
    int8 maxMass = 2;
    bool8 attach = false;
    bool8 broken = false;
    bool8 isVisible = false;
    bool8 open = false;
    bool8 conductive = false;
    bool8 left = false, right = false, up = false, down = false;
    bool8 sourceLit = false;
    bool8 hasPower = false;
    bool8 changed = false;
    bool8 active = false;
    bool8 starCollecting = false;
    
    // TODO: These are too big for it to store upfront,
    // Allocate it whenever you need this
    TweenController tweenController;
    AnimatedSprite animatedSprite;
    };

struct AddEntityResult
{
    Entity *entity;
     uint16 entityIndex;
};

struct FindAttachableResult
{
    Entity * entity;
    bool8 has;
};

inline bool8 IsMovable(Entity * entity)
{
    bool8 result = 
        entity->type == ENTITY_TYPE_PLAYER || 
        entity->type == ENTITY_TYPE_CLONE  || 
        entity->type == ENTITY_TYPE_BLOCK;
    
    return result;
}

inline IVec2 GetDoorDirection(Entity * door);
inline bool8 BridgeBlocked(Entity * bridge, IVec2 dir);
inline bool8 DoorBlocked(Entity * door, IVec2 reachDir);
inline bool8 SameSide(Entity * door, IVec2 tilePos, IVec2 reachDir);
inline bool8 IsDoor(Entity * door);
inline Vector2 GetSlimeSize(Entity * slime);
inline Entity * GetEntity(int32 i);
inline void MoveEntity(Entity * entity, Entity * attachedEntity, TweenEvent * playEvent,
                       IVec2 targetPos, real32 (*MoveFunc)(real32), real32 speed, bool8 isStretch = true);

inline Array<Entity *, LAYER_COUNT>
FindAllEntitiesFromLocationAndLayers(IVec2 pos,
                                     EntityLayer * layers, 
                                     uint32 layerCount);


#define ENTITY_H
#endif
