/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "entity.h"
#include "game_util.h"

inline Entity * GetEntity(int i)
{
    Entity * entity = &gameState->entities[i];
    if (!entity->active) entity = nullptr;
    return entity;
}

inline Entity * GetPlayer()
{
    return GetEntity(gameState->playerEntityIndex);
}

inline Rectangle GetEntityRect(Entity * entity)
{
    Rectangle rect = { entity->pivot.x, entity->pivot.y, entity->tileSize, entity->tileSize };
    return rect;
}

inline bool8 IsSlime(Entity * entity)
{
    return entity->type == ENTITY_TYPE_CLONE || entity->type == ENTITY_TYPE_PLAYER;
}


inline AddEntityResult
AddEntity(EntityType type, IVec2 tilePos, SpriteID spriteID, Color color = WHITE, int tileSize = 32)
{
    AddEntityResult result;

    Entity entity = {};
    entity.type = type;
    entity.tilePos = tilePos;
    entity.spriteID = spriteID;
    entity.sprite = GetSprite(spriteID);
    entity.color = color;
    entity.active = true;
    entity.tileSize = (float)tileSize;
    entity.pivot = GetTilePivot(tilePos, (float)tileSize);    
        
    result.entityIndex = gameState->entities.Add(entity);
    result.entity = &gameState->entities[result.entityIndex];

    result.entity->entityIndex = result.entityIndex;

    return result;
    
}


inline AddEntityResult
AddCable(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_CONNECT;
    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    Cable_Indices.Add(entityResult.entityIndex);

    return entityResult;
}


inline AddEntityResult
AddDoor(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);

    entityResult.entity->movable = true;
    entityResult.entity->mass = 100;
    
    entityResult.entity->conductive = false;
    entityResult.entity->open = false;
    entityResult.entity->cableType = CABLE_TYPE_DOOR;

    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    Door_Indices.Add(Cable_Indices.Add(entityResult.entityIndex));

    return entityResult;
}

inline AddEntityResult
AddSource(IVec2 tilePos, SpriteID spriteID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_SOURCE;
    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    Source_Indices.Add(Cable_Indices.Add(entityResult.entityIndex));
    
    return entityResult;
}

inline AddEntityResult
AddConnection(IVec2 tilePos, SpriteID spriteID)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, spriteID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_CONNECTION_POINT;
    entityResult.entity->left = true;
    entityResult.entity->right = true;
    entityResult.entity->up = true;
    entityResult.entity->down = true;

    CP_Indices.Add(Cable_Indices.Add(entityResult.entityIndex));

    return entityResult;
}

inline void DeleteEntity(Entity * entity)
{
    entity->active = false;
    entity->type = ENTITY_TYPE_NULL;
    entity->tweenController.Reset();
}

inline void SetEntityPosition(Entity * entity, Entity * touchingEntity, IVec2 tilePos)
{
    SM_ASSERT(entity->active, "entity does not exist");
    SM_ASSERT(entity->movable, "entity cannot be moved");

    // entity->positionSetMarker = true;
    
    entity->tilePos = tilePos;
    // entity->pivot = GetTilePivot(entity);

    if (touchingEntity && touchingEntity && (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE))
    {
        IVec2 dir = (touchingEntity->tilePos - entity->tilePos);

        dir.x = dir.x == 0 ? 0 : Sign(dir.x);
        dir.y = dir.y == 0 ? 0 : Sign(dir.y);
                
        SM_ASSERT(dir.SqrMagnitude() == 1, "Invalid bounce direction");

        SetAttach(entity, touchingEntity, dir);
    }
    else
    {
        entity->attach = false;
    }

}

inline void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir)
{
    SM_ASSERT((attacher->type == ENTITY_TYPE_PLAYER || attacher->type == ENTITY_TYPE_CLONE), "entity is not attachable");
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

inline void SetActionState(Entity * entity, ActionState state)
{
    SM_ASSERT(entity->active, "entity does not exist");
    entity->actionState = state;    
}

inline void SetGlassBeBroken(Entity * glass)
{
    SM_ASSERT(glass->active, "entity does not exist");
    
    glass->broken = true;
    glass->sprite = GetSprite(SPRITE_GLASS_BROKEN);
}

inline float GetSlimeSize(int mass)
{
    return mass == 1 ? 0.5f * MAP_TILE_SIZE : 0.8f * MAP_TILE_SIZE;
}

inline float GetSlimeSize(Entity * slime)
{
    return GetSlimeSize(slime->mass);
}

inline Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime)
{
    SM_ASSERT(mergeSlime->active && mergedSlime->active, "entity does not exist");
    SM_ASSERT(mergeSlime != mergedSlime, "Entity cannot merge itself");

    if (mergedSlime->entityIndex == gameState->playerEntityIndex)
    {
        mergeSlime->type = ENTITY_TYPE_PLAYER;
        gameState->playerEntityIndex = mergeSlime->entityIndex;
        mergeSlime->color = WHITE;
    }

    // mergeSlime->tileSize = endSize;
    // mergeSlime->pivot = GetTilePivot(mergeSlime);
    {
        // Merge Slime Tween

        float startSize = GetSlimeSize(mergeSlime);
        Vector2 startPivot = GetTilePivot(mergeSlime);
        mergeSlime->mass++;
        float endSize = GetSlimeSize(mergeSlime);
        Vector2 endPivot = GetTilePivot(mergeSlime->tilePos, endSize, mergeSlime->attachDir);

        TweenParams params2 = {};
        params2.paramType = PARAM_TYPE_VECTOR2;
        params2.startVec2 = startPivot;
        params2.endVec2 = endPivot;
        params2.realVec2 = &mergeSlime->pivot;
        AddTween(mergeSlime->tweenController, CreateTween(params2), 1);
        
        TweenParams params1 = {};
        params1.paramType = PARAM_TYPE_FLOAT;
        params1.startF = startSize;
        params1.endF = endSize;
        params1.realF = &mergeSlime->tileSize;
        AddTween(mergeSlime->tweenController, CreateTween(params1), 0);
    }

    {
        // Merged Slime Tween
        
        mergedSlime->tweenController.Reset();
        TweenParams params = {};
        params.paramType = PARAM_TYPE_VECTOR2;
        params.startVec2 = mergedSlime->pivot;
        params.endVec2 = mergeSlime->pivot;
        params.realVec2 = &mergedSlime->pivot;
        
        float dist = Vector2Distance(params.startVec2, params.endVec2);
        float iDist = dist / MAP_TILE_SIZE;
        AddTween(mergedSlime->tweenController, CreateTween(params, nullptr, BOUNCE_SPEED, iDist));

        TweenEvent & endEvent = mergedSlime->tweenController.endEvent;
        endEvent.controller = &mergeSlime->tweenController;
        endEvent.OnPlayFunc = OnPlayEvent;
        endEvent.deleteEntity = mergedSlime;
        endEvent.OnDeleteFunc = OnDeleteEvent;

        OnPlayEvent(&mergedSlime->tweenController);
        
    }
    // DeleteEntity(mergedSlime);

    return mergeSlime;
    
}


inline bool8 AttachSlime(Entity * slime, IVec2 dir)
{
    SM_ASSERT((slime->type == ENTITY_TYPE_PLAYER || slime->type == ENTITY_TYPE_CLONE), "entity is not attachable");

    IVec2 pos = slime->tilePos + dir;

    bool8 attach = false;

    Entity * attachEntity = nullptr;

    for (uint32 j = 0; j < gameState->entities.count; j++)
    {
        attachEntity = GetEntity(j);

        if (attachEntity && attachEntity->tilePos == pos)
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

inline FindAttachableResult FindAttachable(IVec2 tilePos, IVec2 attachDir)
{
    FindAttachableResult result = { nullptr, false };
    
    bool8 has = false;
    Entity * entity = nullptr;

    for (uint32 i = 0; i < gameState->entities.count; i++)
    {
        entity = GetEntity(i);
        if (entity && entity->tilePos == tilePos)
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
                        has = DoorBlocked(entity, attachDir);
                    }
                    break;
                }
                case ENTITY_TYPE_PIT:
                {
                    has = false;
                    goto EndLoop;     
                }
            }
            
            if (has) break;
        }
    } EndLoop:;

    result.has = has; 
    result.entity = entity;
    return result;
}


inline Entity * FindEntityByLocationAndLayers(IVec2 pos, EntityLayer * layers, int arrayCount)
{
    for (int layerIndex = 0; layerIndex < arrayCount; layerIndex++)
    {
        int layer = layers[layerIndex];
        auto & entityIndeices = gameState->entityTable[layer];
        for (uint32 i = 0; i < entityIndeices.count; i++)
        {
            Entity * entity = GetEntity(entityIndeices[i]);
            if (entity && entity->tilePos == pos)
            {
                return entity;
                break;
            }
        }
    }
    
    return nullptr;
}

inline MoveSlimeUntilBlockResult SlimeMoveTowardsUntilBlocked(Entity * entity, IVec2 dest, IVec2 dir)
{
    MoveSlimeUntilBlockResult result = { false };
    
    IVec2 start = entity->tilePos + dir;
    bool8 isSlime = (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE);

    SM_ASSERT(isSlime, "For now entity can only be slime");

    for (IVec2 pos = start; pos.IsBetween(start, dest); pos += dir)
    {
        for (uint32 i = 0; i < gameState->entities.count; i++)
        {
            Entity * target = GetEntity(i);
            if (target && target->tilePos == pos)
            {
                switch(target->type)
                {
                    case ENTITY_TYPE_PIT:
                    {
                        SetEntityPosition(entity, nullptr, pos - dir);
                        return result;
                    }
                    case ENTITY_TYPE_BLOCK:
                    case ENTITY_TYPE_WALL:
                    {
                        SetEntityPosition(entity, target, pos - dir);
                        return result;
                    }
                    case ENTITY_TYPE_GLASS:
                    {
                        if (!target->broken)
                        {
                            SetEntityPosition(entity, target, pos - dir);
                            return result;
                        }
                        break;
                    }
                    case ENTITY_TYPE_PLAYER:
                    case ENTITY_TYPE_CLONE:
                    {
                        if (isSlime)
                        {
                            entity = MergeSlimes(target, entity);
                            result.merged = true;
                            return result;
                        }

                        break;
                    }
                    case ENTITY_TYPE_ELECTRIC_DOOR:
                    {
                        switch(target->cableType)
                        {
                            case CABLE_TYPE_DOOR:
                            {
                                if (SameSide(target, pos, dir))
                                {
                                    SetEntityPosition(entity, target, pos - dir);
                                    return result;
                                }
                                break;
                            }
                            
                        }
                        break;
                    }
                }
            }
        }
    }
    
    Entity * attach = nullptr;
    if (entity->attach)
    {
        attach = GetEntity(entity->attachedEntityIndex);
    }
    SetEntityPosition(entity, attach, dest);

    return result;
    
}

inline void UpdateSlimes()
{
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
    for (uint32 i = 0; i < slimeEntityIndices.count; i++)
    {
        Entity * slime = GetEntity(slimeEntityIndices[i]);
        if (slime && slime->attach)
        {
            // slime->tweenController.NoTweens() && 
            Entity * attach = GetEntity(slime->attachedEntityIndex);

            // NOTE: Depends on the levels, there might be the case when the attached entity get destroyed along with the slime
            //       Need to handle that case in here if necessary
            if (attach && attach->type != ENTITY_TYPE_ELECTRIC_DOOR)
            {
                IVec2 oldPos = slime->tilePos;
                IVec2 newPos = attach->tilePos - slime->attachDir;
                if (oldPos != newPos)
                {
                    IVec2 dir = newPos - oldPos;
                    dir.x = dir.x == 0 ? 0 : Sign(dir.x);
                    dir.y = dir.y == 0 ? 0 : Sign(dir.y);
                
                    SM_ASSERT(dir.SqrMagnitude() == 1, "Invalid bounce direction");

                    Vector2 moveStart = GetTilePivot(slime);
                    IVec2 attachDirStart = slime->attachDir;
                    MoveSlimeUntilBlockResult moveResult = SlimeMoveTowardsUntilBlocked(slime, newPos, dir);
      
                    if (moveResult.merged)
                    {
                        return;                        
                    }
                    
                    Vector2 moveEnd = GetTilePivot(slime);
                    IVec2 attachDirEnd = slime->attachDir;

                    // TODO: Play Instanly for now, need to blend the current animation with this animation
                    //       How do I blend two motions
#if 1
                    slime->tweenController.Reset();
#endif
                    if (attachDirStart != attachDirEnd)
                    {
                        Vector2 moveMiddle = GetTilePivot(slime->tilePos, slime->tileSize, attachDirStart);
                        moveMiddle =
                            Vector2Add(moveMiddle,
                                       Vector2Scale({ (float)dir.x, (float)dir.y }, 0.5f *(MAP_TILE_SIZE - slime->tileSize)));

                        float dist = Vector2Distance(moveStart, moveMiddle);
                        float iDist = dist / MAP_TILE_SIZE;
                    
                        TweenParams params1 = {};
                        params1.paramType = PARAM_TYPE_VECTOR2;
                        params1.startVec2 = moveStart;
                        params1.endVec2 = moveMiddle;
                        params1.realVec2  = &slime->pivot;
                        AddTween(slime->tweenController, CreateTween(params1, nullptr,  BOUNCE_SPEED, iDist));
                    
                        TweenParams params2 = {};
                        params2.paramType = PARAM_TYPE_VECTOR2;
                        params2.startVec2 = moveMiddle;
                        params2.endVec2 = moveEnd;
                        params2.realVec2  = &slime->pivot;
                        AddTween(slime->tweenController, CreateTween(params2, nullptr,  BOUNCE_SPEED * 2.0f, iDist));
                        
                    }
                    else
                    {
                        float dist = Vector2Distance(moveStart, moveEnd);
                        float iDist = dist / MAP_TILE_SIZE;
                    
                        TweenParams params = {};
                        params.paramType = PARAM_TYPE_VECTOR2;
                        params.startVec2 = moveStart;
                        params.endVec2 = moveEnd;
                        params.realVec2  = &slime->pivot;
                        AddTween(slime->tweenController, CreateTween(params, nullptr,  BOUNCE_SPEED, iDist));
                    }
                    
                    if (!attach->tweenController.NoTweens())
                    {
                        TweenEvent & startEvent = attach->tweenController.startEvent;
                        startEvent.controller = &slime->tweenController;
                        startEvent.OnPlayFunc = OnPlayEvent;
                    }
                    else
                    {
                        OnPlayEvent(&slime->tweenController);
                    } 
                }
            }
        }
    }

    Entity * player = GetEntity(gameState->playerEntityIndex);
    for (uint32 i = 0; i < slimeEntityIndices.count; i++)
    {
        Entity * slime = GetEntity(slimeEntityIndices[i]);
        if (slime && slime != player && slime->mass > player->mass)
        {
            slime->type = ENTITY_TYPE_PLAYER;
            slime->color = WHITE;

            player->type = ENTITY_TYPE_CLONE;
            player->color = GRAY;

            gameState->playerEntityIndex = slime->entityIndex;
        }
    }
    
}

inline Entity * CreateSlimeClone(IVec2 tilePos)
{
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
    Entity * freeEntity = nullptr;
    for (uint32 i = 0; i < slimeEntityIndices.count; i++)
    {
        Entity * entity = &gameState->entities[slimeEntityIndices[i]];
        if (!entity->active)
        {
            freeEntity = entity;
        }
    }

    SM_ASSERT(freeEntity, "slimes slots are full");

    freeEntity->active = true;
    freeEntity->tilePos = tilePos;
    freeEntity->type = ENTITY_TYPE_CLONE;
    freeEntity->mass = 1;
    freeEntity->tileSize = GetSlimeSize(freeEntity);
    freeEntity->color = GRAY;
    freeEntity->attach = false;    

    return freeEntity;
    
}



void ShiftEntities(IVec2 startPos, IVec2 bounceDir)
{
    Entity * last = nullptr;
    for (IVec2 pos = startPos; ; pos = pos + bounceDir)
    {
        bool8 empty = true;
        for (uint32 i = 0; i < gameState->entities.count; i++)
        {
            Entity * entity = GetEntity(i);
            if (entity && entity != last && entity->movable && entity->tilePos == pos)
            {
                last = entity;
                entity->tilePos += bounceDir;
                empty = false;
                break;
            }
        }

        if (empty)
        {
            break;            
        }
    }
}

inline bool8 CheckBounce(IVec2 tilePos, IVec2 pushDir)
{
    IVec2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

    bool8 bounce = true;
    
    for (int i = 0; i < 4; i++)
    {
        if (dirs[i] == -pushDir) continue;

        for (uint32 entityIndex = 0; entityIndex < gameState->entities.count; entityIndex++)
        {
            Entity * target = GetEntity(entityIndex);

            if (target && target->tilePos == tilePos + dirs[i])
            {
                switch (target->type)
                {
                    case ENTITY_TYPE_BLOCK:
                    case ENTITY_TYPE_WALL:
                    {
                        return false;
                    }
                    case ENTITY_TYPE_GLASS:
                    {
                        if (!target->broken)
                        {
                            return false;
                        }
                        break;
                    }
                    case ENTITY_TYPE_ELECTRIC_DOOR:
                    {
                        if (target->cableType == CABLE_TYPE_DOOR && SameSide(target, target->tilePos, dirs[i]))
                        {
                            return false; 
                        }
                        break;
                    }
                }
            }
            
        }
    }
    return true;
}


void BounceEntity(Entity * entity, IVec2 dir)
{
    SM_ASSERT(entity->active, "entity does not exists");
    SM_ASSERT(entity->movable, "entitiy is static");
    
    IVec2 start = entity->tilePos + dir;
    
    for (IVec2 pos = start;
         ;
         pos = pos + dir)
    {
        for (uint32 i = 0; i < gameState->entities.count; i++)
        {
            Entity * target = GetEntity(i);
            bool8 isSlime = (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE);

            if (target && target->tilePos == pos)
            {
                switch(target->type)
                {
                    case ENTITY_TYPE_ELECTRIC_DOOR:
                    {
                        if (target->cableType == CABLE_TYPE_DOOR && SameSide(target, pos, dir))
                        {

                            SetEntityPosition(entity, target, pos - dir);
                            return;    
                        }
                        break;
                    }
                    case ENTITY_TYPE_PIT:
                    case ENTITY_TYPE_WALL:
                    {

                        if (isSlime && target->type == ENTITY_TYPE_WALL)
                        {
                            SetAttach(entity, target, dir);
                        }
                        SetEntityPosition(entity, target, pos - dir);
                        return;
                    }
                    case ENTITY_TYPE_GLASS:
                    {
                        if (!target->broken && isSlime)
                        {

                            SetAttach(entity, target, dir);
                            SetEntityPosition(entity, target, pos - dir);
                            return;
                        }


                        SetGlassBeBroken(target);
                        break;                        
                    }
                    case ENTITY_TYPE_BLOCK:
                    {

                        MoveActionResult result = 
                            MoveActionCheck(entity, entity, pos, dir, 0);

                        if (IsSlime(entity))
                        {
                            SetEntityPosition(entity, target, target->tilePos - dir);
                        }
                        else if (result.blocked)
                        {
                            SetEntityPosition(entity, result.blockedEntity, pos - dir);
                        }
                        else
                        {
                            SetEntityPosition(entity, nullptr, pos - dir);
                        }
                        
                        return;
                    }
                    case ENTITY_TYPE_PLAYER:
                    case ENTITY_TYPE_CLONE:
                    {
                        if (!isSlime)
                        {

                            IVec2 attachDir = target->attachDir;
                            MoveActionResult result =
                                MoveActionCheck(entity, entity, pos, dir, 0);

                            if (result.blocked)
                            {
                                if (result.blockedEntity != target)
                                {
                                    SetEntityPosition(target, result.blockedEntity, target->tilePos);
                                }
                            }
                            else
                            {
                                Entity * attachEntity = nullptr;
                                FindAttachableResult result = FindAttachable(target->tilePos + attachDir, attachDir);
                                if (result.has)
                                {
                                    attachEntity = result.entity;
                                }
                                
                                if (attachEntity && attachEntity != target)
                                {
                                    SetEntityPosition(target, attachEntity, target->tilePos);
                                }

                            }
                            
                            SetEntityPosition(entity, nullptr, pos - dir);

                            return;
                        }

                        // if (target->type == ENTITY_TYPE_PLAYER)
                        // {

                        //     entity = MergeSlimes(entity, target);
                        // }
                        // else
                        // {
                        //     entity = MergeSlimes( target, entity);

                        // }
                        entity = MergeSlimes(target, entity);
                                                
                        break;
                    }
                }
                        
                // break;
            }
            
        }
        
        if (CheckOutOfBound(pos))
        {
            entity->tilePos = pos;

            DeleteEntity(entity);
            return;
        }

    }
}
