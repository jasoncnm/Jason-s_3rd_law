#include "entity.h"
#include "game_util.h"

// --------------------------------------------------------------------
// NOTE: Entity Actions
// --------------------------------------------------------------------

inline void DeleteEntity(Entity * entity)
{
    entity->changed = true;
    entity->active = false;
    entity->type = ENTITY_TYPE_NULL;
    entity->tweenController.Reset();
}

inline Entity * MergeSlimes(Entity * mergeSlime, Entity * mergedSlime)
{
    SM_ASSERT(mergeSlime->active && mergedSlime->active, "entity does not exist");
    SM_ASSERT(mergeSlime != mergedSlime, "Entity cannot merge itself");
    
    mergedSlime->changed = true;
    mergeSlime->changed = true;
    
    if (mergedSlime->entityIndex == gameState->playerEntityIndex)
    {
        gameState->playerEntityIndex = mergeSlime->entityIndex;
        mergeSlime->color = WHITE;
    }
    
    // mergeSlime->tileSize = endSize;
    // mergeSlime->pivot = GetTilePivot(mergeSlime);
    {
        // Merge Slime Tween
        
         Vector2 startSize = GetSlimeSize(mergeSlime);
        Vector2 startPivot = GetTilePivot(mergeSlime);
        mergeSlime->mass++;
         Vector2 endSize = GetSlimeSize(mergeSlime);
        Vector2 endPivot = GetTilePivot(mergeSlime->tilePos, endSize, mergeSlime->attachDir);
        
        TweenParams params2 = {};
        params2.paramType = PARAM_TYPE_VECTOR2;
        params2.startVec2 = startPivot;
        params2.endVec2 = endPivot;
        params2.realVec2 = &mergeSlime->pivot;
        AddTweenUnique(mergeSlime->tweenController, CreateTween(params2));
        
        TweenParams params1 = {};
        params1.paramType = PARAM_TYPE_VECTOR2;
        params1.startVec2 = startSize;
        params1.endVec2 = endSize;
        params1.realVec2 = &mergeSlime->tileSize;
        AddTweenUnique(mergeSlime->tweenController, CreateTween(params1));
    }
    
    {
        // Merged Slime Tween
        mergedSlime->tweenController.Reset();
        
        Entity * attach = nullptr;
        if (mergeSlime->attach)
        {
            attach = GetEntity(mergeSlime->attachedEntityIndex);
        }
        
        TweenEvent endEvent = { 0 };
        endEvent.controller = &mergeSlime->tweenController;
        endEvent.deleteEntity = mergedSlime;
        mergedSlime->tweenController.endEvents.Add(endEvent);
        
        MoveEntity(mergedSlime, attach, nullptr, mergeSlime->tilePos, BLOCK_MOVE_FUNC, BOUNCE_SPEED);
        
    }
    
    return mergeSlime;
}

inline void SetEntityPosition(Entity * entity, Entity * attachedEntity, IVec2 tilePos)
{
    SM_ASSERT(entity->active, "entity does not exist");
    
    entity->changed = true;
    entity->tilePos = tilePos;
    
    if (attachedEntity)
    {
        IVec2 dir = (attachedEntity->tilePos - entity->tilePos);
        dir.x = dir.x == 0 ? 0 : Sign(dir.x);
        dir.y = dir.y == 0 ? 0 : Sign(dir.y);
        // SM_ASSERT(IsDoor(entity) || IsDoor(attachedEntity) || dir.SqrMagnitude() == 1, "Invalid direction");
        
        Entity * door = nullptr;
        if (IsDoor(entity)) door = entity;
        if (IsDoor(attachedEntity)) door = attachedEntity;
        if (door && (tilePos == door->tilePos))
        {
            dir = -GetDoorDirection(door);
            }
        
        if (IsSlime(entity))
        {
            entity->attach = true;
            entity->attachedEntityIndex = attachedEntity->entityIndex;
            entity->attachDir = dir;
        }
        
        if (IsSlime(attachedEntity))
        {
            attachedEntity->attach = true;
            attachedEntity->attachedEntityIndex = entity->entityIndex;
            attachedEntity->attachDir = -dir;
        }
    }
    else
    {
        entity->attach = false;
        entity->attachedEntityIndex = entity->entityIndex;
    }
}

inline Entity * CreateSlimeClone(Entity * ent)
{
    ent->changed = true;
    IVec2 tilePos = ent->tilePos;
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
    Entity * freeEntity = nullptr;
    for (uint32 i = 0; i < slimeEntityIndices.count; i++)
    {
        Entity * slime = &gameState->entities[slimeEntityIndices[i]];
        if (!slime->active)
        {
            freeEntity = slime;
            *freeEntity = *ent;
            freeEntity->entityIndex = slimeEntityIndices[i];
            freeEntity->active = true;
            freeEntity->tilePos = tilePos;
            freeEntity->mass = 1;
            freeEntity->tileSize = GetSlimeSize(freeEntity);
            freeEntity->color = GRAY;  
            freeEntity->pivot = GetTilePivot(freeEntity);
            freeEntity->changed = true;
        }
    }
    SM_ASSERT(freeEntity, "slimes slots are full");
    return freeEntity;
    
}


// --------------------------------------------------------------------

inline Entity * GetEntity(int32 i)
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
    Rectangle rect = { entity->pivot.x, entity->pivot.y, entity->tileSize.x, entity->tileSize.y };
    return rect;
}

inline bool8 IsSlime(Entity * entity)
{
    return entity->type == ENTITY_TYPE_SLIME;
}

inline bool8 IsCable(Entity * entity)
{
    return entity->type == ENTITY_TYPE_CABLE_SOURCE ||
        entity->type == ENTITY_TYPE_CABLE_WIRE ||
        entity->type == ENTITY_TYPE_CABLE_CONNECT;
    }

inline void SetSlimeSprite(Entity * slime, IVec2 dir)
{
    
    slime->sprite = GetSlimeSprite(dir);
    }

inline void SetSlimeAnimatedSprite(Entity * slime, IVec2 dir)
{
    enum 
    {
        SLIME_ANI_RIGHT,
        SLIME_ANI_LEFT,
        SLIME_ANI_UP,
        SLIME_ANI_DOWN,
    };
    
    real32 ani_seconds_per_frame = 0.05f;
    
    if (dir == IVec2 { -1, 0 })
    {
        // LEFT
        BeginAnimation(&slime->animatedSprite, SLIME_ANI_LEFT, ani_seconds_per_frame, false, false);
        }
    else if (dir == IVec2 { 1, 0 })
    {
        // RIGHT
        BeginAnimation(&slime->animatedSprite, SLIME_ANI_RIGHT, ani_seconds_per_frame, false, false);
        }
    else if (dir == IVec2 { 0, -1 })
    {
        // UP
        BeginAnimation(&slime->animatedSprite, SLIME_ANI_UP, ani_seconds_per_frame, false, false);
    }
    else if (dir == IVec2 { 0, 1 })
    {
        // DOWN
        BeginAnimation(&slime->animatedSprite, SLIME_ANI_DOWN, ani_seconds_per_frame, false, false);
    }
}

inline real32 GetStretchSpeed(Entity * player)
{
    // TODO hacky code
    
    real32 speed = 0.0f;
    
    if (player->attachDir.x > 0)
    {
        speed = MOVE_SPEED * 0.3f; 
    }
    else if (player->attachDir.y < 0)
    {
        speed = MOVE_SPEED * 0.1f;
    }
    else if (player->attachDir.x < 0)
    {
        speed = MOVE_SPEED * 0.1f;
    }
    else if (player->attachDir.y > 0)
    {
        speed = MOVE_SPEED * 0.3f;
    }
    else 
    {
        speed = MOVE_SPEED * 0.2f;
    }
    
    if (player->mass == 2)
    {
        speed *= 1.2f;
    }
    
    return speed;
}

inline void StretchEntity(Entity * entity, 
                          IVec2 moveDir, IVec2 startAttach, IVec2 endAttach,
                          IVec2 startPos, IVec2 endPos,
                          real32 stretch, real32 (*MoveFunc)(real32),
                          real32 speed = MOVE_SPEED, bool8 invert = false,
                          Entity * startAttachEntity = nullptr, Entity * endAttachEntity = nullptr)
{
    
    Vector2 startPivot = GetTilePivot(startPos, entity->tileSize, startAttach);
    if (startAttachEntity && IsDoor(startAttachEntity) && startAttachEntity->tilePos == startPos)
    {
        startPivot -= Vector2 { (real32)startAttach.x, (real32)startAttach.y } * 5;
    }
    
    Vector2 endPivot = GetTilePivot(endPos, entity->tileSize, endAttach);
    if (endAttachEntity && IsDoor(endAttachEntity) && endAttachEntity->tilePos == endPos)
    {
        endPivot -= Vector2 { (real32)endAttach.x, (real32)endAttach.y } * 5;
    }
    
    Vector2 stretchV = Vector2 { 1 / stretch, stretch };
    if (invert) stretchV = Vector2Invert(stretchV);
    if (Abs(moveDir.y) == 1)
    {
        stretchV = Vector2Invert(stretchV);
    }
    
    Vector2 size_mid = entity->tileSize * stretchV;
    Vector2 endS = GetTilePivot(endPos, size_mid, endAttach);
    if (endAttachEntity && IsDoor(endAttachEntity) && endAttachEntity->tilePos == endPos)
    {
         endS -= Vector2 { (real32)endAttach.x, (real32)endAttach.y } * 5;
    }
    
    Vector2 startS = GetTilePivot(startPos, size_mid, startAttach);
    if (startAttachEntity && IsDoor(startAttachEntity) && startAttachEntity->tilePos == startPos)
    {
         startS-= Vector2 { (real32)startAttach.x, (real32)startAttach.y } * 5;
    }
    
    Vector2 end_mid = (endS + startS) / 2;
    
    real32 dist1 = Vector2Distance(startPivot, end_mid) / MAP_TILE_SIZE;
    real32 dist2 = Vector2Distance(end_mid, endPivot) / MAP_TILE_SIZE;
    
    TweenParams squash = {};
    squash.paramType = PARAM_TYPE_VECTOR2;
    squash.startVec2 = entity->tileSize;
    squash.endVec2 = size_mid;
    squash.realVec2 = &entity->tileSize;
    
    TweenParams param = {};
    param.paramType = PARAM_TYPE_VECTOR2;
    param.startVec2 = startPivot;
    param.endVec2 = end_mid;
    param.realVec2  = &entity->pivot;
    
    TweenParams squash2 = {};
    squash2.paramType = PARAM_TYPE_VECTOR2;
    squash2.startVec2 = size_mid;
    squash2.endVec2 = entity->tileSize;
    squash2.realVec2 = &entity->tileSize;
    
    TweenParams param2 = {};
    param2.paramType = PARAM_TYPE_VECTOR2;
    param2.startVec2 = end_mid;
    param2.endVec2 = endPivot;
    param2.realVec2  = &entity->pivot;
    
    int ch2 = AddTweenUnique(entity->tweenController, 
                             CreateTween(squash, MoveFunc,
                                         speed, dist1));
    
    AddTween(entity->tweenController,
             CreateTween(squash2, MoveFunc, speed, dist2), ch2);
    
    int32 channel = entity->tweenController.FindChannelByTweenProperty(PARAM_TYPE_VECTOR2, &entity->pivot);
    
    if (channel < 0)
    {
        int ch1 = AddTweenUnique(entity->tweenController, CreateTween(param, MoveFunc, speed, dist1));
        AddTween(entity->tweenController, CreateTween(param2, MoveFunc, speed, dist2), ch1);
    }
    else
    {
        AddTween(entity->tweenController, CreateTween(param, MoveFunc, speed, dist1), channel);
        AddTween(entity->tweenController, CreateTween(param2, MoveFunc, speed, dist2), channel);
    }
    
    }

inline void MoveEntity(Entity * entity, Entity * attachedEntity, TweenEvent * playEvent,
                       IVec2 targetPos, float (*MoveFunc)(float), float speed,
                       bool8 isStretch)
{
    SM_ASSERT(entity->active, "entity does not exist");
    Entity old = *entity;
    Vector2 startPivot = GetTilePivot(entity);
    
    SetEntityPosition(entity, attachedEntity, targetPos);
    Vector2 endPivot = GetTilePivot(entity);
    
    if (Vector2Equals(startPivot, endPivot))
    {
        // SM_ASSERT(false, "entity not moving");
        return;
    }
    
    
                if (IsSlime(entity))
    {
        IVec2 offset = targetPos - old.tilePos;
        
        if ((Abs(offset).x == 0 || Abs(offset).y == 0))
        {
            if ((Abs(old.attachDir) != Abs(entity->attachDir)))
            {
                Vector2 middlePivot = GetTilePivot(targetPos, entity->tileSize, old.attachDir);
                
                if (offset == IVec2 {0, 0} ||
                    entity->attachDir == IVec2 { Sign(offset.x), Sign(offset.y) })
                {
                    middlePivot += Vector2 
                    { 
                        0.5f * entity->attachDir.x * (MAP_TILE_SIZE - entity->tileSize.x),
                        0.5f * entity->attachDir.y * (MAP_TILE_SIZE - entity->tileSize.y)
                    };
                }
                
                if (attachedEntity && IsDoor(attachedEntity) && attachedEntity->tilePos == targetPos)
                {
                    middlePivot -= Vector2 { (real32)entity->attachDir.x, (real32)entity->attachDir.y } * 5;
                }
                else if (Entity * prevAttach = GetEntity(old.attachedEntityIndex);
                         IsDoor(prevAttach) && (targetPos == prevAttach->tilePos))
                {
                    middlePivot -= Vector2 { (real32)old.attachDir.x, (real32)old.attachDir.y } * 5;
                }
                
                TweenParams params1 = {};
                params1.paramType = PARAM_TYPE_VECTOR2;
                params1.startVec2 = startPivot;
                params1.endVec2 = middlePivot;
                params1.realVec2  = &entity->pivot;
                
                TweenParams params2 = {};
                params2.paramType = PARAM_TYPE_VECTOR2;
                params2.startVec2 = middlePivot;
                params2.endVec2 = endPivot;
                params2.realVec2  = &entity->pivot;
                
                 real32 dist = Vector2Distance(startPivot, middlePivot);
                 real32 tileDist = dist / MAP_TILE_SIZE;
                
                real32 dist2 = Vector2Distance(middlePivot, endPivot) / MAP_TILE_SIZE;
                
                uint32 channel = AddTweenUnique(entity->tweenController, CreateTween(params1, MoveFunc, speed, tileDist));
                
                AddTween(entity->tweenController, CreateTween(params2, MoveFunc, speed, dist2), channel);
                
                }
            else
            {
                
                IVec2 idir = IVec2 { Sign(targetPos.x - old.tilePos.x), Sign(targetPos.y - old.tilePos.y) };
                
                IVec2 mid_tile = targetPos;
                IVec2 startAttach = old.attachDir;
                IVec2 endAttach = entity->attachDir;
                
                if (offset.SqrMagnitude() > 1)
                {
                    endAttach = startAttach;
                    mid_tile = old.tilePos + idir;
                }
                
                if (isStretch)
                {
                    float stretch = 0.8f;
                    Entity * sEnt = GetEntity(old.attachedEntityIndex);
                    
                    bool8 reverse = entity->attachDir.x == 0 && old.attachDir == -entity->attachDir;
                    
                StretchEntity(entity, idir, startAttach, endAttach, 
                                  old.tilePos, mid_tile, stretch, MoveFunc, speed, reverse, sEnt, attachedEntity);
                }
                else
                {
                    mid_tile = old.tilePos;
                }
                
                if ((targetPos - mid_tile).SqrMagnitude() > 0)
                {
                    int32 ch1 = entity->tweenController.FindChannelByTweenProperty(PARAM_TYPE_VECTOR2, &entity->pivot);
                    
                    Vector2 startPivot = GetTilePivot(mid_tile, entity->tileSize, old.attachDir);
                    
                    
                    TweenParams param1 = {0};
                    TweenParams param2 = {0};
                    float dist = Vector2Distance(startPivot, endPivot);
                    
                    if (Abs(entity->attachDir) != Abs(idir))
                    {
                    Vector2 midPivot = GetTilePivot(targetPos, entity->tileSize, old.attachDir);
                        
                         dist= Vector2Distance(startPivot, midPivot);
                        
                     param1.paramType = PARAM_TYPE_VECTOR2;
                    param1.startVec2 = startPivot;
                    param1.endVec2 = midPivot;
                    param1.realVec2 = &entity->pivot;
                
                param2.paramType = PARAM_TYPE_VECTOR2;
                param2.startVec2 = midPivot;
                param2.endVec2 = endPivot;
                        param2.realVec2  = &entity->pivot;
                        
                    }
                    else
                    {
                        param1.paramType = PARAM_TYPE_VECTOR2;
                        param1.startVec2 = startPivot;
                        param1.endVec2 = endPivot;
                        param1.realVec2 = &entity->pivot;
                        }
                    
                    float tileDist = dist / MAP_TILE_SIZE;
                    
                    if (ch1 < 0)
                    {
                        if (param1.paramType != PARAM_TYPE_NONE)
                         ch1 = AddTweenUnique(entity->tweenController, 
                                             CreateTween(param1, MoveFunc, speed, tileDist));
                        if (param2.paramType != PARAM_TYPE_NONE)
                            AddTween(entity->tweenController, CreateTween(param2, MoveFunc, speed, 0.1f), ch1);
                        }
                    else
                    {
                        if (param1.paramType != PARAM_TYPE_NONE)
                            AddTween(entity->tweenController, 
                                 CreateTween(param1, MoveFunc, speed, tileDist), ch1);
                        if (param2.paramType != PARAM_TYPE_NONE)
                            AddTween(entity->tweenController, 
                                 CreateTween(param2, MoveFunc, speed, 0.1f), ch1);
                    }
                }
            }
        }
        else
        {
            
            Entity * oldAttach = old.attach ? GetEntity(old.attachedEntityIndex) : nullptr;
            
            IVec2 dir = -entity->attachDir;
            if (oldAttach)
            {
                if (old.attachDir.x != 0)
                {
                    dir = IVec2 { 0, Sign(offset.y) };
                }
                else
                {
                    dir = IVec2 { Sign(offset.x), 0 };
                }
            }
            
            Vector2 middlePivot = startPivot + 
            (Vector2 
             {
                 (float)dir.x, (float)dir.y
             } * (DEFAULT_TILE_SIZE + entity->tileSize) * 0.5f);
            
            TweenParams params1 = {};
            params1.paramType = PARAM_TYPE_VECTOR2;
            params1.startVec2 = startPivot;
            params1.endVec2 = middlePivot;
            params1.realVec2  = &entity->pivot;
            
            TweenParams params2 = {};
            params2.paramType = PARAM_TYPE_VECTOR2;
            params2.startVec2 = middlePivot;
            params2.endVec2 = endPivot;
            params2.realVec2  = &entity->pivot;
            
            real32 dist1 = Vector2Distance(startPivot, middlePivot) / MAP_TILE_SIZE;
            real32 dist2 = Vector2Distance(middlePivot, endPivot) / MAP_TILE_SIZE;
            
            uint32 channel = AddTweenUnique(entity->tweenController, CreateTween(params1, EaseInSine, speed, dist1 * 0.7f));
            AddTween(entity->tweenController, CreateTween(params2, EaseInSine, speed, dist2 * 0.7f), channel);
            
        }
        }
    else
    {
        
        IVec2 offset = targetPos - old.tilePos;
        IVec2 moveDir = IVec2 { Sign(offset.x), Sign(offset.y) };
        IVec2 mid_tile = old.tilePos + moveDir;
        
        if (isStretch)
        {
            StretchEntity(entity, 
                      moveDir, 
                      IVec2 { 0, 0 },
                          IVec2 { 0, 0 }, 
                          old.tilePos,
                          mid_tile,
                          0.8f,
                          MoveFunc,
                          speed,
                          true);
        }
        else
        {
            mid_tile = old.tilePos;
        }
        
        if ((targetPos - mid_tile).SqrMagnitude() > 0)
            {
                
                int32 ch1 = entity->tweenController.FindChannelByTweenProperty(PARAM_TYPE_VECTOR2, &entity->pivot);
                
                Vector2 midPivot = GetTilePivot(mid_tile, entity->tileSize);
                
                float dist = Vector2Distance(midPivot, endPivot);
                float tileDist = dist / MAP_TILE_SIZE;
                TweenParams param = {};
                param.paramType = PARAM_TYPE_VECTOR2;
                param.startVec2 = midPivot;
                param.endVec2 = endPivot;
            param.realVec2  = &entity->pivot;
            
            if (ch1 < 0)
            {
                AddTweenUnique(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist));
                }
            else
            {
                AddTween(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist), ch1);
            }
            
                SM_TRACE("channel: %d", ch1);
                SM_TRACE("end tile pos: %d", targetPos.x);
            }
        
        }
    
    if (!entity->tweenController.NoTweens())
    {
        if (playEvent)
        {
            playEvent->controller = &entity->tweenController;
            }
        else
        {
        OnPlayEvent(&entity->tweenController);
        }
    }
    
    if (CheckOutOfBound(targetPos))
    {
        TweenEvent deleteEvent = { 0 };
        deleteEvent.deleteEntity = entity;
        if (IsSlime(entity))
        {
            deleteEvent.undo = true;
        }
        entity->tweenController.endEvents.Add(deleteEvent);
        
        }
    
}

inline void DettachSlime(Entity * slime)
{
    slime->attach = false;
    slime->attachDir = IVec2 { 0, 0 };
    }

inline void SetAttach(Entity * attacher, Entity * attachee, IVec2 dir)
{
    SM_ASSERT((attacher->type == ENTITY_TYPE_SLIME), "entity is not attachable");
    SM_ASSERT(attacher->active && attachee->active, "entity does not exist");

    if (IsSlime(attacher))
    {
        attacher->attach = true;
        attacher->attachedEntityIndex = attachee->entityIndex;
        attacher->attachDir = dir;
    }

    if (IsSlime(attachee))
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

inline bool8 SetGlassBeBroken(Entity * glass)
{
    bool8 changed = false;
    SM_ASSERT(glass && glass->active, "entity does not exist");
    Sprite newSprite = GetBrokenGlassSprite();
    if (newSprite != glass->sprite)
    {
        changed = true;
        glass->sprite = newSprite;
    }
    return changed;
}

inline Vector2 GetSlimeSize(int32 mass)
{
     return mass == 1 ? DEFAULT_TILE_SIZE * 0.5f : DEFAULT_TILE_SIZE * 0.7f;
}

inline Vector2 GetSlimeSize(Entity * slime)
{
    return GetSlimeSize(slime->mass);
}


inline bool8 AttachSlime(Entity * slime, IVec2 dir)
{
    SM_ASSERT((slime->type == ENTITY_TYPE_SLIME), "entity is not attachable");

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
    
    EntityLayer layers[] = { LAYER_WALL, LAYER_BLOCK, LAYER_GLASS, LAYER_LOCK, LAYER_DOOR, };
    
    auto entList = FindAllEntitiesFromLocationAndLayers(tilePos, layers, ArrayCount(layers));
    
    for (uint32 idx = 0; idx < entList.count; idx++)
    {
         entity = entList[idx];
        if (entity && entity->tilePos == tilePos)
        {
            switch(entity->type)
            {
                case ENTITY_TYPE_LOCK:
                {
                    if (entity->open) break;
                }
                case ENTITY_TYPE_BRIDGE:
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
                case ENTITY_TYPE_DOOR:
                {
                    has = DoorBlocked(entity, attachDir);
                        
                        if (has) break;
                }
                case ENTITY_TYPE_PIT:
                {
                    has = false;
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

inline Array<Entity *, LAYER_COUNT> FindAllEntitiesFromLocationAndLayers(IVec2 pos, EntityLayer * layers, uint32 layerCount)
{
    Array<Entity *, LAYER_COUNT> result;
    for (uint32 layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        int32 layer = layers[layerIndex];
        for (uint32 i = 0; i < gameState->entityTable[layer].count; i++)
        {
            Entity * ent = GetEntity(gameState->entityTable[layer][i]);
            if (ent && ent->tilePos == pos)
            {
                result.Add(ent);
                break;
            }
        }
    }
    return result;
}

inline Entity * FindEntityByLocationAndLayers(IVec2 pos, EntityLayer * layers, uint32 arrayCount)
{
    for (uint32 layerIndex = 0; layerIndex < arrayCount; layerIndex++)
    {
        int32 layer = layers[layerIndex];
        auto & entityIndeices = gameState->entityTable[layer];
        for (uint32 i = 0; i < entityIndeices.count; i++)
        {
            Entity * entity = GetEntity(entityIndeices[i]);
            if (entity && entity->tilePos == pos)
            {
                return entity;
                }
        }
    }
    
    return nullptr;
}

inline Entity * FindAttachSlime(Entity * attachObject)
{
    // SM_ASSERT(!IsSlime(attachObject), "Slime Cannot attach to slime");
    
    Entity * attachSlime = nullptr;
    
    auto slimeIndices = gameState->entityTable[LAYER_SLIME];
    for (uint32 i = 0; i < slimeIndices.count; i++)
    {
        Entity * slime = GetEntity(slimeIndices[i]);
        if (slime && slime->attachedEntityIndex == attachObject->entityIndex)
        {
            attachSlime = slime;
            break;
        }
    }
    
    return attachSlime;
    }

inline Entity * FindBlockEntityFromTo(IVec2 from, IVec2 to, IVec2 dir)
{
    EntityLayer checkLayers[] = { LAYER_WALL, LAYER_DOOR, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK, LAYER_PIT };
    
    Entity * result = nullptr;
    for (IVec2 pos = from; ; pos += dir)
    {
        Entity * ent = FindEntityByLocationAndLayers(pos, checkLayers, ArrayCount(checkLayers));
        if (ent)
        {
            if (ent->type == ENTITY_TYPE_GLASS && ent->broken ||
                ent->type == ENTITY_TYPE_DOOR && !DoorBlocked(ent, dir)) continue;
            result = ent;
            break;
        }
        
        if (pos == to)
        {
            break;
        }
    }
    
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
            Entity * attach = GetEntity(slime->attachedEntityIndex);
            if (attach && attach->type == ENTITY_TYPE_BLOCK)
            {
                IVec2 oldPos = slime->tilePos;
                IVec2 newPos = attach->tilePos - slime->attachDir;
                if (oldPos != newPos)
                {
                    
                    TweenEvent * playEvent = nullptr;
                    if (!(attach->tweenController.start || attach->tweenController.playing) && !attach->tweenController.NoTweens())
                    {
                        int32 index = attach->tweenController.startEvents.Add(TweenEvent{ 0 });
                        playEvent = &attach->tweenController.startEvents[index];
                    }
                    
                    IVec2 dir = newPos - oldPos;
                    // SM_ASSERT(((dir.x != 0 && dir.y == 0) || (dir.y != 0 && dir.x == 0 )), "invalid direction");
                    dir.x = dir.x != 0 ? Sign(dir.x) : 0;
                    dir.y = dir.y != 0 ? Sign(dir.y) : 0;
                    if (slime->tweenController.playing) 
                    {
                        slime->tweenController.Reset();
                    }
                    
                    Entity * blockedEnt = FindBlockEntityFromTo(oldPos + dir, newPos, dir);
                    
                    real32 (*MoveFunc)(float) = PLAYER_PROJ_FUNC;
                    float aniSpeed = BOUNCE_SPEED;
                    if (!attach->tweenController.NoTweens())
                    {
                        Tween * moveTween = attach->tweenController.FindTweenByTweenProperty(PARAM_TYPE_VECTOR2, &attach->pivot);
                        
                        if (!FloatEquals(aniSpeed, moveTween->dt))
                        {
                            aniSpeed = moveTween->dt;
                            MoveFunc = moveTween->Easing;
                        }
                    }
                    
                    IVec2 targetPos = newPos;
                    
                    if (blockedEnt)
                    {
                        if ((blockedEnt->type == ENTITY_TYPE_GLASS &&
                                 blockedEnt->broken) ||
                            (blockedEnt->type == ENTITY_TYPE_DOOR &&
                             DoorBlocked(blockedEnt, dir)))
                        {
                            continue;
                        }

                        
                        if (IsSlime(blockedEnt))
                        {
                            MergeSlimes(blockedEnt, slime);
                        return;
                        }
                        targetPos = blockedEnt->tilePos - dir;
                        attach = blockedEnt;
                    }
                    
                    
                    MoveEntity(slime, attach, playEvent, targetPos, MoveFunc, aniSpeed);
                    
                    }
                }
        }
        
    }
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
             if (entity && entity != last && IsMovable(entity) && entity->tilePos == pos)
            {
                last = entity;
                // entity->tilePos += bounceDir;
                Entity * attach = nullptr;
                IVec2 targetPos = entity->tilePos + bounceDir;
                if (IsSlime(entity) && entity->attach)
                {
                    FindAttachableResult result = FindAttachable(targetPos + entity->attachDir, entity->attachDir);
                    if (result.has)
                    {
                        attach = result.entity;
                    }
                }
                
                MoveEntity(entity, attach, nullptr, targetPos, BLOCK_MOVE_FUNC, BOUNCE_SPEED);
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
