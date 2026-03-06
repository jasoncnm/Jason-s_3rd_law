/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
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
        AddTweenUnique(mergeSlime->tweenController, CreateTween(params2));
        
        TweenParams params1 = {};
        params1.paramType = PARAM_TYPE_FLOAT;
        params1.startF = startSize;
        params1.endF = endSize;
        params1.realF = &mergeSlime->tileSize;
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
    SM_ASSERT(entity->movable, "entity cannot be moved");
    
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
            freeEntity->type = ENTITY_TYPE_CLONE;
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
    Rectangle rect = { entity->pivot.x, entity->pivot.y, entity->tileSize, entity->tileSize };
    return rect;
}

inline bool8 IsSlime(Entity * entity)
{
    return entity->type == ENTITY_TYPE_CLONE || entity->type == ENTITY_TYPE_PLAYER;
}

#if 0
inline bool8 BridgeBlocked(Entity * bridge, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool8 result = false;
    if (reachDir.x == 1)
    {
        result = (bridge->tileID == BRIDGE_RIGHT_A || bridge->tileID == BRIDGE_RIGHT_B);
    }
    else if (reachDir.x == -1)
    {
        result = (bridge->tileID == BRIDGE_LEFT_A || bridge->tileID == BRIDGE_LEFT_B);
    }
    else if (reachDir.y == 1)
    {
        result = (bridge->tileID == BRIDGE_DOWN_A || bridge->tileID == BRIDGE_DOWN_B);
    }
    else if (reachDir.y == -1)
    {
        result = (bridge->tileID == BRIDGE_UP_A || bridge->tileID == BRIDGE_UP_B);
    }
    
    return result;
}
#endif

inline void SetEntitySprite(Entity * entity, TileID tileID)
{
    entity->tileID = tileID;
    entity->sprite = GetSprite(tileID);
}

inline void SetSlimeSprite(Entity * slime, IVec2 dir)
{
    if (dir == IVec2 { -1, 0 })
    {
        SetEntitySprite(slime, PLAYER_LEFT);
    }
    else if (dir == IVec2 { 1, 0 })
    {
        SetEntitySprite(slime, PLAYER_RIGHT);
    }
    else if (dir == IVec2 { 0, -1 })
    {
        SetEntitySprite(slime, PLAYER_UP);
    }
    else if (dir == IVec2 { 0, 1 })
    {
        SetEntitySprite(slime, PLAYER_DOWN);
    }
    else
    {
        SetEntitySprite(slime, PLAYER_IDLE);
    }
}

inline AddEntityResult
AddEntity(EntityType type, IVec2 tilePos, TileID tileID, Color color = WHITE, int32 tileSize = MAP_TILE_SIZE)
{
    AddEntityResult result;

    Entity entity = {};
    entity.type = type;
    entity.tilePos = tilePos;
    entity.tileID = tileID;
    entity.sprite = GetSprite(tileID);
    entity.color = color;
    entity.active = true;
    entity.tileSize = (float)tileSize;
    entity.pivot = GetTilePivot(tilePos, (float)tileSize);    
        
    result.entityIndex = (uint16)gameState->entities.Add(entity);
    result.entity = &gameState->entities[result.entityIndex];

    result.entity->entityIndex = result.entityIndex;

    return result;
    }


inline AddEntityResult
AddCable(IVec2 tilePos, TileID tileID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, tileID);
    
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
AddDoor(IVec2 tilePos, TileID tileID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, tileID);

    entityResult.entity->movable = true;
    entityResult.entity->mass = 100;
    
    entityResult.entity->conductive = false;
    entityResult.entity->open = false;
    entityResult.entity->cableType = CABLE_TYPE_DOOR;

    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    return entityResult;
}

inline AddEntityResult
AddSource(IVec2 tilePos, TileID tileID, bool8 left, bool8 right, bool8 up, bool8 down)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, tileID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_SOURCE;
    entityResult.entity->left = left;
    entityResult.entity->right = right;
    entityResult.entity->up = up;
    entityResult.entity->down = down;

    return entityResult;
}

inline AddEntityResult
AddConnection(IVec2 tilePos, TileID tileID)
{
    AddEntityResult entityResult = AddEntity(ENTITY_TYPE_ELECTRIC_DOOR, tilePos, tileID);
    
    entityResult.entity->conductive = false;
    entityResult.entity->cableType = CABLE_TYPE_CONNECTION_POINT;
    entityResult.entity->left = true;
    entityResult.entity->right = true;
    entityResult.entity->up = true;
    entityResult.entity->down = true;

    return entityResult;
}

inline void MoveEntity(Entity * entity, Entity * attachedEntity, TweenEvent * playEvent,
                       IVec2 targetPos, float (*MoveFunc)(float), float speed)
{
    SM_ASSERT(entity->active, "entity does not exist");
    SM_ASSERT(entity->movable, "entity cannot be moved");
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
                
                Vector2 middlePivot = 
                    endPivot + Vector2 { (real32)old.attachDir.x, (real32)old.attachDir.y } * 
                (0.5f * (MAP_TILE_SIZE - entity->tileSize));
                
                Entity * prevAttach = GetEntity(old.attachedEntityIndex);
                if (IsDoor(prevAttach) && (targetPos == prevAttach->tilePos))
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
                
                float dist = Vector2Distance(startPivot, middlePivot);
                float tileDist = dist / MAP_TILE_SIZE;
                
                uint32 channel = AddTweenUnique(entity->tweenController, CreateTween(params1, MoveFunc, speed, tileDist));
                AddTween(entity->tweenController, CreateTween(params2, MoveFunc, speed * 2), channel);
                }
            else
            {
                
                Vector2 dir = Vector2Subtract(endPivot, startPivot);
                IVec2 idir = IVec2 { Sign(dir.x), Sign(dir.y) };
                
                float dist = Vector2Distance(startPivot, endPivot);
                float tileDist = dist / MAP_TILE_SIZE;
                
                TweenParams param = {};
                param.paramType = PARAM_TYPE_VECTOR2;
                param.startVec2 = startPivot;
                param.endVec2 = endPivot;
                param.realVec2  = &entity->pivot;
                
                
                int32 channel = entity->tweenController.FindMovingChannel(PARAM_TYPE_VECTOR2);
                
                if (channel < 0)
                {
                    AddTweenUnique(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist));
                }
                else
                {
                    AddTween(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist), channel);
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
            Vector2 middlePivot = Vector2Add(startPivot, Vector2Scale({ (float)dir.x, (float)dir.y },
                                                                      0.5f * (MAP_TILE_SIZE + entity->tileSize)));
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
            
            uint32 channel = AddTweenUnique(entity->tweenController, CreateTween(params1, MoveFunc, speed * 1.5f));
            AddTween(entity->tweenController, CreateTween(params2, MoveFunc, speed * 1.5f), channel);
            
        }
        }
    else
    {
        float dist = Vector2Distance(startPivot, endPivot);
        float tileDist = dist / MAP_TILE_SIZE;
        
        TweenParams param = {};
        param.paramType = PARAM_TYPE_VECTOR2;
        param.startVec2 = startPivot;
        param.endVec2 = endPivot;
        param.realVec2  = &entity->pivot;
        
        int32 channel = entity->tweenController.FindMovingChannel(PARAM_TYPE_VECTOR2);
        
        if (channel < 0)
        {
            AddTweenUnique(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist));
        }
        else
        {
            AddTween(entity->tweenController, CreateTween(param, MoveFunc, speed, tileDist), channel);
            
        }
        
        SM_TRACE("channel: %d", channel);
        SM_TRACE("end tile pos: %d", targetPos.x);
        
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
    SM_ASSERT(glass && glass->active, "entity does not exist");
    
    glass->sprite = GetSprite(GLASS_BROKEN);
}

inline float GetSlimeSize(int32 mass)
{
    return mass == 1 ? 0.55f * MAP_TILE_SIZE :  0.7f * MAP_TILE_SIZE;
}

inline float GetSlimeSize(Entity * slime)
{
    return GetSlimeSize(slime->mass);
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
                case ENTITY_TYPE_LOCK:
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
                ent->type == ENTITY_TYPE_ELECTRIC_DOOR && !DoorBlocked(ent, dir)) continue;
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
                    
                    float aniSpeed = BOUNCE_SPEED;
                    if (!attach->tweenController.NoTweens())
                    {
                        aniSpeed = attach->tweenController.channels[attach->tweenController.FindMovingChannel(PARAM_TYPE_VECTOR2)].last().dt;
                    }
                    
                    if (blockedEnt)
                    {
                        
                        if ((blockedEnt->type == ENTITY_TYPE_GLASS &&
                                 blockedEnt->broken) ||
                            (blockedEnt->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                             blockedEnt->cableType == CABLE_TYPE_DOOR &&
                             DoorBlocked(blockedEnt, dir)))
                        {
                            continue;
                        }

                        
                        if (IsSlime(blockedEnt))
                        {
                            MergeSlimes(blockedEnt, slime);
                        return;
                        }
                        MoveEntity(slime, blockedEnt, playEvent, blockedEnt->tilePos - dir, BLOCK_MOVE_FUNC, aniSpeed);
                    }
                    else 
                    {
                        
                        MoveEntity(slime, attach, playEvent, newPos, BLOCK_MOVE_FUNC, aniSpeed);
                    }
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
            if (entity && entity != last && entity->movable && entity->tilePos == pos)
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
