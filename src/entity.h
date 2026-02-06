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
    LAYER_KEY_LOCK,
    
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
    
     uint16 entityIndex;
    int attachedEntityIndex;
    
    int unlockEntityIndex;
    
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
    bool8 changed = false;
    
    bool8 active = false;
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


#define ENTITY_H
#endif
