/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "entity.h"

bool SameSide(Entity * door, IVec2 tilePos, IVec2 reachDir);

inline Entity * GetEntity(int i)
{
    return &gameState->entities[i];
}


AddEntityResult
AddEntity(EntityType type, IVec2 tilePos, SpriteID spriteID, Color color = WHITE)
{
    AddEntityResult result;

    Entity entity = {};
    entity.type = type;
    entity.tilePos = tilePos;
    entity.spriteID = spriteID;
    entity.sprite = GetSprite(spriteID);
    entity.pivot = GetTilePivot(tilePos);
    entity.color = color;
    entity.active = true;
        
    result.entityIndex = gameState->entities.Add(entity);
    result.entity = &gameState->entities[result.entityIndex];

    result.entity->entityIndex = result.entityIndex;

    return result;
    
}


AddEntityResult
AddCable(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_CONNECT;
    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    if (!gameState->electricDoorSystem)
    {
        gameState->electricDoorSystem = (ElectricDoorSystem *)BumpAlloc(gameMemory->persistentStorage, sizeof (ElectricDoorSystem));
    }

    gameState->electricDoorSystem->entityIndices.Add(entityResult.entityIndex);

    return entityResult;
}


AddEntityResult
AddDoor(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->open = false;
    entityResult.entity->cableType = CABLE_TYPE_DOOR;

    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    if (!gameState->electricDoorSystem)
    {
        gameState->electricDoorSystem = (ElectricDoorSystem *)BumpAlloc(gameMemory->persistentStorage, sizeof (ElectricDoorSystem));
    }

    gameState->electricDoorSystem->doorIndices.Add(gameState->electricDoorSystem->entityIndices.Add(entityResult.entityIndex));

    return entityResult;
}

AddEntityResult
AddSource(IVec2 tilePos, SpriteID spriteID, bool left, bool right, bool up, bool down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_SOURCE;
    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    if (!gameState->electricDoorSystem)
    {
        gameState->electricDoorSystem = (ElectricDoorSystem *)BumpAlloc(gameMemory->persistentStorage, sizeof (ElectricDoorSystem));
    }

    gameState->electricDoorSystem->sourceIndices.Add(gameState->electricDoorSystem->entityIndices.Add(entityResult.entityIndex));
    
    return entityResult;
}

AddEntityResult
AddConnection(IVec2 tilePos, SpriteID spriteID)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_CONNECTION_POINT;
    entityResult.entity->left = true;
    entityResult.entity->right = true;
    entityResult.entity->up = true;
    entityResult.entity->down = true;

    if (!gameState->electricDoorSystem)
    {
        gameState->electricDoorSystem = (ElectricDoorSystem *)BumpAlloc(gameMemory->persistentStorage, sizeof (ElectricDoorSystem));
    }

    gameState->electricDoorSystem->connectionPointIndices.Add(gameState->electricDoorSystem->entityIndices.Add(entityResult.entityIndex));

    return entityResult;
}

void DeleteEntity(Entity * entity)
{
    entity->active = false;
    entity->type = ENTITY_TYPE_NULL;
}

void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir);
void SetEntityPosition(Entity * entity, Entity * touchingEntity, IVec2 tilePos)
{
    SM_ASSERT(entity->active, "entity does not exist");
    SM_ASSERT(entity->movable, "entity cannot be moved");


    // entity->positionSetMarker = true;
    
    entity->tilePos = tilePos;
    entity->pivot = GetTilePivot(tilePos);

    if (touchingEntity && (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE))
    {
        IVec2 dir = (touchingEntity->tilePos - entity->tilePos);
        SM_ASSERT(dir.SqrMagnitude() == 1, "Two entity are not near by");

        SetAttach(entity, touchingEntity, dir);
    }
    

#if 0
    if (entity->attach)
    {
        Entity * attachedEntity = GetEntity(entity->attachedEntityIndex);
        if (attachedEntity->movable && !entity->positionSetMarker)
        {
            SetEntityPosition(attachedEntity, tilePos + entity->attachDir);
        }
        else if (attachedEntity->movable)
        {
            attachedEntity->attach = false;
            attachedEntity->attachedEntityIndex = -1;
        }
    }
    
    entity->positionSetMarker = false;

#endif

    
}

void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir)
{
    SM_ASSERT((attacher->type == ENTITY_TYPE_PLAYER || attachee->type == ENTITY_TYPE_CLONE), "entity is not attachable");
    SM_ASSERT(attacher->active && attachee->active, "entity does not exist");

    if (attacher->movable)
    {
        attacher->attach = true;
        attacher->attachedEntityIndex = attachee->entityIndex;
        attacher->attachDir = dir;
    }

    if (attachee->movable)
    {
        attachee->attach = true;
        attachee->attachedEntityIndex = attachee->entityIndex;
        attachee->attachDir = -dir;
    }
}

void SetActionState(Entity * entity, ActionState state)
{
    SM_ASSERT(entity->active, "entity does not exist");
    
    entity->actionState = state;    
}

void SetGlassBeBroken(Entity * glass)
{
    SM_ASSERT(glass->active, "entity does not exist");

    
    glass->broken = true;
    glass->sprite = GetSprite(SPRITE_GLASS_BROKEN);
}

Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime)
{
    SM_ASSERT(mergeSlime->active && mergedSlime->active, "entity does not exist");


    mergeSlime->mass += mergedSlime->mass;
    DeleteEntity(mergedSlime);

    return mergeSlime;
    
}


bool AttachSlime(Entity * slime, IVec2 dir)
{
    SM_ASSERT((slime->type == ENTITY_TYPE_PLAYER || slime->type == ENTITY_TYPE_CLONE), "entity is not attachable");

    IVec2 pos = slime->tilePos + dir;

    bool attach = false;

    Entity * attachEntity = nullptr;

    for (int j = 0; j < gameState->entities.count; j++)
    {
        attachEntity = GetEntity(j);

        if (attachEntity->active && attachEntity->tilePos == pos)
        {
            switch (attachEntity->type)
            {
                case ENTITY_TYPE_BLOCK:
                case ENTITY_TYPE_WALL:
                {
                    attach = true;
                    break;
                }
                case ENTITY_TYPE_GLASS:
                {
                    if (!attachEntity->broken)
                    {
                        attach = true;
                    }
                    break;
                }
            }
            
            if (attach) break;
        }
    }
            
    if (attach)
    {
        SetAttach(slime, attachEntity, dir);
    }

    return attach;

}

FindAttachableResult FindAttachable(IVec2 tilePos, IVec2 attachDir)
{
    FindAttachableResult result = { nullptr, false };
    
    bool has = false;
    Entity * entity = nullptr;

    for (int i = 0; i < gameState->entities.count; i++)
    {
        entity = GetEntity(i);
        if (entity->active && entity->tilePos == tilePos)
        {
            switch(entity->type)
            {
                case ENTITY_TYPE_BLOCK:
                case ENTITY_TYPE_WALL:
                {
                    has = true;
                    break;
                }
                case ENTITY_TYPE_GLASS:
                {
                    has = !entity->broken;
                    break;
                }
                case ENTITY_TYPE_ELECTRIC_DOOR:
                {
                    if (entity->cableType == CABLE_TYPE_DOOR) 
                    {
                        has = true;
                    }
                    break;
                }
            }
            
            if (has) break;
        }
    }

    result.has = has;
    result.entity = entity;
    return result;
}

Entity * FindEntityByLocationAndLayer(IVec2 pos, EntityLayer layer)
{
    auto & entityIndeices = gameState->entityTable[layer];
    for (int i = 0; i < entityIndeices.count; i++)
    {
        Entity * entity = GetEntity(entityIndeices[i]);
        if (entity->active && entity->tilePos == pos)
        {
            return entity;
            break;
        }
    }
    
    return nullptr;
}
