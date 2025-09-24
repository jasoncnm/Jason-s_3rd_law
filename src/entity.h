#if !defined(ENTITY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

enum EntityType
{
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_CLONE,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_GLASS,
    ENTITY_TYPE_ELECTRIC_DOOR,

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

    Sprite sprite;
    SpriteID spriteID;
    IVec2 tilePos;
    IVec2 attachDir;
    Color color;

    Vector2 pivot;
    
    int entityIndex;
    int attachedEntityIndex;

    int sourceIndex = -1;
    int rightIndex = -1, leftIndex = -1, upIndex = -1, downIndex = -1;
    
    int tileSize = 32;

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
    
    bool active = false;

    bool positionSetMarker = false;
    
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

#define ENTITY_H
#endif
