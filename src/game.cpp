
#include "game.h"
#include "game_util.h"
#include "render_interface.h"
#include "action_input.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


#include "entity.cpp"
#include "electric_door.cpp"
#include "level_loader.cpp"
#include "tween.cpp"
#include "tween_controller.cpp"
#include "save_game.cpp"

/*
TODO BUGS: FIX THE BUGS THAT NEEDS TO BE FIXED
- Fix weird animation bugs 
- No restart good idea ??

  TODO: Things that I can do beside arts and design I guess
1. background 1wk
3. collectable: show in ui, 4d
5. Sound effect, 1wk
6. bug fixes, improve post effect 1wk
2. saves and loads 3d

 TODO: PostProcessing
3. chromatic aberration
6. shake

// NOTE: done but need testing
1. tutorial logic: this week
2. key and door
*/

//  ========================================================================
//              NOTE: Game Structs (internal)
//  ========================================================================

//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================
void SetShake(float duration)
{
    gameState->shake = true;
    gameState->time = (real32)GetTime();
    gameState->shakeTime = duration;
    }

inline UndoState::EntityArray GetCurrentStateEntities()
{
    EntityLayer pushLayers[] = {
        LAYER_DOOR,
         // LAYER_CABLE,
        LAYER_SOURCE,
        LAYER_CONNECTION,
        LAYER_GLASS,
        LAYER_SLIME,
        LAYER_BLOCK,
        LAYER_KEY_LOCK,
    };
    uint32 layerCount = ArrayCount(pushLayers);
    
    uint32 len = 0;
    for (uint32 idx = 0; idx < layerCount; idx++)
    {
        uint32 layer = pushLayers[idx];
         len += gameState->entityTable[layer].count;
    }
    
    Entity * entities =
    (Entity *)BumpAllocArray(gameMemory->transientStorage, len, sizeof(Entity));
    
    uint32 i = 0;
    for (uint32 idx = 0; idx < layerCount; idx++)
    {
        uint32 layer = pushLayers[idx];
        auto & entList = gameState->entityTable[layer];
        for (uint32 entId = 0; entId < entList.count; entId++)
        {
            Entity & ent = gameState->entities[entList[entId]];
            if (layer == LAYER_CABLE && ent.cableType == CABLE_TYPE_CONNECT)
            {
                continue;
            }
            
            entities[i++] = ent;
            
        }
    }
    
    return { i, entities };
    }

inline void CheckPushState(Array<CheckThings, 100> & checkList, CheckThings & current)
{
            Entity * ent = current.pushEnt;
    switch(current.pushResult.state)
    {
        case PUSH_BLOCKED:
        {
            current.parent->pushResult.state = PUSH_BLOCKED;
                current.parent->pushResult.blockedEntity = current.pushEnt;
            
            break;
        }
        case PUSH_MOVED:
        {
            current.parent->pushResult.state = PUSH_MOVED;
            current.parent->pushResult.pushing = true;
            
            TweenEvent * playEvent = nullptr;
            TweenController & c = current.parent->pushEnt->tweenController;
            if (!c.NoTweens()) 
            {
                int32 channel = c.FindMovingChannel();
                Tween & ani = c.channels[channel].last();
                
                int32 index = ani.endEvents.Add(TweenEvent{0});
                playEvent = &ani.endEvents[index];
                }
            if (IsSlime(current.parent->pushEnt) && current.parent->pushEnt->attachedEntityIndex == ent->entityIndex)
            {
                MoveEntity(ent, current.parent->pushEnt, playEvent, ent->tilePos + current.pushDir, 
                           BLOCK_MOVE_FUNC, MOVE_SPEED);
            }
            else if (IsSlime(ent) && ent->attach)
            {
                Entity * attachEntity = GetEntity(ent->attachedEntityIndex);
                MoveEntity(ent, attachEntity, playEvent, ent->tilePos + current.pushDir,  
                           BLOCK_MOVE_FUNC, MOVE_SPEED);
            }
            else
            {
                MoveEntity(ent, nullptr, playEvent, ent->tilePos + current.pushDir,  
                           BLOCK_MOVE_FUNC, MOVE_SPEED);
            }
            
            break;
        }
        case PUSH_MERGED:
        {
            MergeSlimes(current.pushResult.mergeEntity, ent); 
            break;
        }
        }
}

inline void ProjectAndCheck(Entity * projectedEnt, 
                            Array<CheckThings, 100> & checkList,
                            IVec2 pushDir, 
                            int32 & accumulatedMass,
                            EntityLayer * checkLayers, 
                            uint32 layerCount)
{
    Entity * pushEnt = checkList.last().parent->pushEnt;
    
    TweenEvent * playEvent = nullptr;
    
    CheckThings * thing = checkList.last().parent;
    while(thing->parent != thing)
    {
        Entity * ent = thing->pushEnt;
        
        if (!ent->tweenController.NoTweens())
        {
            int32 channel = ent->tweenController.FindMovingChannel();
            Tween & current = ent->tweenController.channels[channel].last();
            
            int32 index = current.endEvents.Add(TweenEvent{ 0 });
            playEvent = &current.endEvents[index];
            
            // playEvent = &ent->tweenController.endEvent;
            break;
        }
        thing = thing->parent;
    }
    
    bool8 defered = checkList.last().parent->pushResult.state == PROJECT_DEFERRED;
    
    for (IVec2 pos = projectedEnt->tilePos + pushDir; ; pos += pushDir)
    {
        auto entList = FindAllEntitiesFromLocationAndLayers(pos, checkLayers, layerCount); 
        for (uint32 idx = 0; idx < entList.count; idx++)
        {
            Entity * target = entList[idx];
            
            if (target->actionState == FREEZE_STATE)
            {
                Entity * blockedEntity = target;
                IVec2 targetPos = blockedEntity->tilePos - pushDir;
                
                if (blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR && DoorBlocked(blockedEntity, -pushDir))
                {
                    targetPos = blockedEntity->tilePos;
                }
                
                Entity * attach = defered ? pushEnt : blockedEntity;
                
                MoveEntity(projectedEnt, attach, playEvent, targetPos,  
                           BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                return;
            }
            
            switch(target->type)
            {
                case ENTITY_TYPE_PLAYER:
                case ENTITY_TYPE_CLONE:
                {
                    if (IsSlime(projectedEnt))
                    {
                        MergeSlimes(target, projectedEnt);
                        return;
                    }
                    
                    Entity * attach = nullptr;
                    
                    if (!target->attach || target->attachDir == -pushDir)
                    {
                    // NOTE one special case 
                        checkList.last().pushResult.state = PROJECT_DEFERRED;
                        
                        CheckThings second = {};
                        second.visited = false;
                        second.pushDir = pushDir;
                        second.pushEnt = target;
                        second.checkType = CHECK_PROJECT;
                        second.parent = &checkList.last();
                        checkList.Add(second);
                        
                        attach = target;
                        
                    }
                    
                    MoveEntity(projectedEnt, attach, playEvent, pos - pushDir,
                               BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                        
                        return;
                    }
                case ENTITY_TYPE_BLOCK:
                {
                    CheckThings newThings = {};
                    newThings.visited = false;
                    newThings.pushDir = pushDir;
                    newThings.pushEnt = projectedEnt;
                    newThings.pushResult = { false, PUSH_NONE, nullptr };
                    newThings.checkType = CHECK_NONE;
                    newThings.parent = &checkList.last();
                    checkList.Add(newThings);
                    
                    Entity * attach = target;
                    
                    if (defered)
                    {
                        attach = pushEnt;
                        }
                    
                    MoveEntity(projectedEnt, attach, playEvent, pos - pushDir,  
                               BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                        return;
                }
                case ENTITY_TYPE_GLASS:
                {
                    if (!target->broken && IsSlime(projectedEnt))
                    {
                        Entity * attach = target;
                        if (defered)
                        {
                            attach = pushEnt;
                        }
                        
                        MoveEntity(projectedEnt, attach, nullptr, pos - pushDir,  
                                   BLOCK_MOVE_FUNC, BOUNCE_SPEED); 
                        return;
                    }
                    else if (!target->broken)
                    {
                        MoveEntity(projectedEnt, nullptr, playEvent, pos - pushDir,
                                   BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                    
                    int32 channel = projectedEnt->tweenController.FindMovingChannel();
                    
                    Tween & current = projectedEnt->tweenController.channels[channel].last();
                    
                        int32 index = current.endEvents.Add(TweenEvent{0});
                        current.endEvents[index].breakEntity = target;
                    
                    
                    target->broken = true;
                    // SetGlassBeBroken(target);
                    Entity * attachSlime = FindAttachSlime(target);
                    if (attachSlime) attachSlime->attach = false;
                    }
                    
                    break;
                }
                case ENTITY_TYPE_ELECTRIC_DOOR:
                {
                    SM_ASSERT(target->cableType == CABLE_TYPE_DOOR, "other cable type is not reachble");
                    bool8 blocked = DoorBlocked(target, pushDir) || DoorBlocked(target, -pushDir);
                    if (!blocked) break;
                }
                case ENTITY_TYPE_WALL:
                case ENTITY_TYPE_PIT:
                case ENTITY_TYPE_LOCK:
                {
                    Entity * blockedEntity = target;
                    IVec2 targetPos = blockedEntity->tilePos - pushDir;
                    
                    if (blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR && DoorBlocked(blockedEntity, -pushDir))
                    {
                        targetPos = blockedEntity->tilePos;
                    }
                    
                    Entity * attach = blockedEntity;
                    
                    if (defered)
                    {
                        attach = pushEnt;
                        }
                    
                    MoveEntity(projectedEnt, attach, playEvent, targetPos,  
                               BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                    return;
                }
            }
        }
        
        if (CheckOutOfBound(pos))
        {
            MoveEntity(projectedEnt, nullptr, playEvent, pos, 
                       BLOCK_MOVE_FUNC, BOUNCE_SPEED);
            TweenEvent deleteEvent = { 0 };
            deleteEvent.deleteEntity = projectedEnt;
            projectedEnt->tweenController.endEvents.Add(deleteEvent);
            // DeleteEntity(projectedEnt);
            return;
        }
        
    }
    
}

inline void PushCheck(Array<CheckThings, 100> & checkList, int32 & accumulatedMass, 
                                 int32 startMass, EntityLayer * checkLayers, uint32 layerCount)
{
    CheckThings & current = checkList.last();
    auto entList = FindAllEntitiesFromLocationAndLayers(current.pushEnt->tilePos + current.pushDir, 
                                                        checkLayers, layerCount); 
    for (uint32 idx = 0; idx < entList.count; idx++)
    {
        Entity * target = entList[idx];
        
        if (target->actionState == FREEZE_STATE)
        {
            current.pushResult.state = PUSH_BLOCKED;
            current.pushResult.blockedEntity = target;
            return;
            }
        
        switch(target->type)
        {
            case ENTITY_TYPE_ELECTRIC_DOOR:
            {
                SM_ASSERT(target->cableType == CABLE_TYPE_DOOR, "other cable type is not reachble");
                
                if (DoorBlocked(target, current.pushDir))
                {
                    current.pushResult.state = PUSH_BLOCKED;
                    current.pushResult.blockedEntity = target;
                        return;
                }
                break;
            }
            case ENTITY_TYPE_GLASS:
            {
                if (!target->broken)
                {
                    current.pushResult.state = PUSH_BLOCKED;
                    current.pushResult.blockedEntity = target;
                        return;
                }
                break;
            }
            case ENTITY_TYPE_PLAYER:
            case ENTITY_TYPE_CLONE:
            {
                if (IsSlime(current.pushEnt))
                {
                    if (current.pushDir == -current.pushEnt->attachDir)
                    {
                        current.pushResult.state = PUSH_NONE;
                        return;
                    }
                    current.pushResult.state = PUSH_MERGED;
                    current.pushResult.mergeEntity = target;
                    return;
                }
                else if (target->attachDir == -current.pushDir)
                {
                    CheckThings newThings = {};
                    newThings.visited = false;
                    newThings.pushDir = current.pushDir;
                    newThings.pushEnt = target;
                    newThings.pushResult = { false, PUSH_NONE, nullptr };
                    newThings.checkType = CHECK_MOVE;
                    newThings.parent = &current;
                    checkList.Add(newThings);
                    return;
                    
                }
                }
            case ENTITY_TYPE_LOCK:
            case ENTITY_TYPE_PIT:
            case ENTITY_TYPE_WALL:
            {
                current.pushResult.state = PUSH_BLOCKED;
                current.pushResult.blockedEntity = target;
                    return;
            }
            case ENTITY_TYPE_BLOCK:
            {
                {
                    EntityLayer layers[] = { LAYER_DOOR };
                    Entity * door = FindEntityByLocationAndLayers(target->tilePos, layers, ArrayCount(layers));
                    if (door && DoorBlocked(door, -current.pushDir))
                    {
                        current.pushResult.state = PUSH_BLOCKED;
                        current.pushResult.blockedEntity = target;
                            return;
                    }
                }
                
                
                bool isProjectable = true;
                
                { // NOTE: IsProjectable
                IVec2 tilePos = target->tilePos;
                IVec2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
                for (uint32 i = 0; i < 4; i++)
                {
                    if (dirs[i] == -current.pushDir) continue;
                    Entity * ent = FindEntityByLocationAndLayers(tilePos + dirs[i], checkLayers, layerCount);
                    if (ent)
                    {
                        // NOTE: slime dose not block the block if it is attach to the block...
                        if ((ent->type == ENTITY_TYPE_ELECTRIC_DOOR) &&
                                (ent->cableType != CABLE_TYPE_DOOR || !SameSide(ent, ent->tilePos, dirs[i]) ||
                                 !DoorBlocked(ent, dirs[i])) ||
                            (ent->type == ENTITY_TYPE_GLASS && ent->broken) ||
                            (IsSlime(ent) && (!ent->attach || dirs[i] != current.pushDir)))
                        {
                            
                            continue;
                        }
else if (IsSlime(ent) && GetEntity(ent->attachedEntityIndex) == target)
                        {
                                if (dirs[i] == current.pushDir)
                                {
                                    // TODO: very weird edge case behavior
                                    Entity * blockedEntity = FindEntityByLocationAndLayers(ent->tilePos + dirs[i],
                                                                                           checkLayers, layerCount);
                                    if (blockedEntity)
                                    {
                                        if (blockedEntity->type != ENTITY_TYPE_GLASS || !blockedEntity->broken)
                                        {
                                        current.pushResult.state = PUSH_BLOCKED;
                                            current.pushResult.blockedEntity = target;
                                        }
                                        return;
                                    }
                                }
                            continue;
                            }
                            else if (ent->type == ENTITY_TYPE_BLOCK && dirs[i] == current.pushDir)
                            {
                                if (current.pushEnt->type == ENTITY_TYPE_BLOCK)
                                {
                                    // continue;
                                }
                                // TODO: very weird edge case behavior
                            }
                            isProjectable = false;
                        break;
                    }
                }
                }
                
                if (isProjectable)
                {
                    current.pushResult.state = PUSH_MOVED;
                    current.pushResult.pushing = true;
                    
                    CheckThings newThings = {};
                    newThings.visited = false;
                    newThings.pushDir = current.pushDir;
                    newThings.pushEnt = target;
                    newThings.pushResult = { false, PUSH_NONE, nullptr };
                    newThings.checkType = CHECK_PROJECT;
                    newThings.parent = &current;
                    checkList.Add(newThings);
                    
                    return;
                }
                
                accumulatedMass += target->mass;
                if (accumulatedMass > startMass)
                {
                    current.pushResult.state = PUSH_BLOCKED;
                    current.pushResult.blockedEntity = target;
                        return;
                }
                
                CheckThings newThings = {};
                newThings.visited = false;
                newThings.pushDir = current.pushDir;
                newThings.pushEnt = target;
                newThings.pushResult = { false, PUSH_NONE, nullptr };
                newThings.checkType = CHECK_MOVE;
                newThings.parent = &current;
                checkList.Add(newThings);
                    return;
            }
        }
    }
    
    current.pushResult.state = PUSH_MOVED;
        
        return;
}


PushResult ActionCheck(Entity * startEnt, IVec2 pushDir, CheckType startState)
{
    SM_ASSERT(startEnt->movable, "Static entity cannot be pushing blocks!");
    
    // IMPORTANT: the order of the layers are important, for example, we don't want to check blocks before checking doors in the same tile
    EntityLayer checkLayers[] = { LAYER_WALL, LAYER_DOOR, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK, LAYER_KEY_LOCK };
    uint32 layerCount = ArrayCount(checkLayers);
    
    Array<CheckThings, 100> checkList;
    
    CheckThings root = 
    { false, pushDir, startState, startEnt, { false, PUSH_NONE, nullptr } };
    root.parent = &root;
    
    checkList.Add(root);
    
    int32 startMass = startEnt->mass;
    int32 accumulatedMass = 0;
    while (true)
    {
        CheckThings & current = checkList.last();
        
        if (current.visited)
        {
            if (checkList.count == 1)
            {
                break;
            }
            checkList.RemoveLast();
            PushState currentState = current.pushResult.state;
            
            if (current.checkType == CHECK_MOVE)
            {
                CheckPushState(checkList, current);
            }
            else if (current.checkType == CHECK_PROJECT)
            {
                // TODO: What I'm doing here is hacky
                CheckThings * parent = current.parent;
                if (parent->pushEnt != current.pushEnt && parent->pushResult.state == PROJECT_DEFERRED)
                {
                    TweenEvent * playEvent = nullptr;
                    if (!parent->parent->pushEnt->tweenController.NoTweens())
                    {
                        int32 index = parent->parent->pushEnt->tweenController.endEvents.Add(TweenEvent{0});
                        playEvent = &parent->parent->pushEnt->tweenController.endEvents[index];
                    }
                    MoveEntity(parent->pushEnt, nullptr, playEvent, 
                               current.pushEnt->tilePos - current.pushDir,  BLOCK_MOVE_FUNC, BOUNCE_SPEED);
                }
            }
            }
        else
        {
            current.visited = true;
            
            if (current.checkType == CHECK_PROJECT)
            {
                ProjectAndCheck(current.pushEnt, checkList, current.pushDir, accumulatedMass, checkLayers, layerCount);
            }
            else
            {
                PushCheck(checkList, accumulatedMass, startMass, checkLayers, layerCount);
            }
                    }
    }
    
    // NOTE: Deal with root outside of the function
    return checkList.last().pushResult;
    }

inline float GetCameraZoom(Map & currentMap)
{
    int32 newWidth = GetScreenWidth();
    int32 newHeight = GetScreenHeight();
    
     int32 camMax = (currentMap.width > currentMap.height) ? currentMap.width : currentMap.height;
    
    // int32 camMax = 11; 
    
    float zoom = (zoom_per_tile / camMax);
    (newWidth < newHeight) ? zoom *= newWidth : zoom *= newHeight;
    
    return zoom;
}

inline void UpdateCameraToTileMapSmooth(Map & map, Vector2 pos, uint32 mapIndex)
{
    Vector2 endPos = gameState->camera.base.target;
     gameState->camera.tweenController.Reset();
    gameState->camera.base.target = endPos;
    
    // TODO adjust move and zoom speed based on move and zoom distance
    
    TweenParams params = {};
    params.paramType = PARAM_TYPE_VECTOR2;
    params.startVec2 = endPos;
    params.endVec2 = pos;
    params.realVec2  = &gameState->camera.base.target;
    AddTweenUnique(gameState->camera.tweenController, CreateTween(params, CAMERA_MOVE_FUNC, CAMERA_MOVE_SPEED));
    
    Map & lastMap = gameState->tileMaps[gameState->currentMapIndex];
    float oldZoom = gameState->camera.base.zoom;
    float newZoom = GetCameraZoom(map);
    if (!FloatEquals(oldZoom, newZoom))
    {
        TweenParams params = {};
        params.paramType = PARAM_TYPE_FLOAT;
        params.startF = oldZoom;
        params.endF = newZoom;
        params.realF  = &gameState->camera.base.zoom;
        AddTweenUnique(gameState->camera.tweenController, CreateTween(params, CAMERA_ZOOM_FUNC, CAMERA_ZOOM_SPEED));
    }
    
    OnPlayEvent(&gameState->camera.tweenController);
    gameState->currentMapIndex = mapIndex;
}

 FindTileMapResult FindTileMap(IVec2 tilePos)
{
    FindTileMapResult result = { 0 };
    for (int32 i = 0; i < gameState->tileMapCount; i++)
    {
        Map & map = gameState->tileMaps[i];
        
        int32 minX = map.tilePos.x + 1;
        int32 minY = map.tilePos.y + 1;
        int32 maxX = minX + map.width - 1;
        int32 maxY = minY + map.height - 1;
        
        if (tilePos.x >= minX && tilePos.x <= maxX && 
            tilePos.y >= minY && tilePos.y <= maxY)
        {
            result.map = &map;
            result.mapIndex = i;
            break;
        }
    }
    return result;
}

 void SetCamFollowState(MyCamera & cam, Entity * followEnt)
{
    if (followEnt->type == ENTITY_TYPE_LOCK)
    {
        cam.followState = MyCamera::FOLLOW_CENTER;
        return;
    }
    
    Vector2 finalPos = GetTilePivot(followEnt);
        Rectangle finalRect = { finalPos.x, finalPos.y, followEnt->tileSize, followEnt->tileSize };
        Map & current = gameState->tileMaps[gameState->currentMapIndex];
        Vector2 mapMin = GetTilePivot(current.tilePos, MAP_TILE_SIZE);
        Rectangle tileMapRec =
        {
            mapMin.x + MAP_TILE_SIZE,
            mapMin.y + MAP_TILE_SIZE,
            (float)current.width  * (float)MAP_TILE_SIZE,
            (float)current.height * (float)MAP_TILE_SIZE
    };
    
        if (followEnt->tweenController.playing && 
            (followEnt->tweenController.channels[followEnt->tweenController.FindMovingChannel()].last().dt == BOUNCE_SPEED)
            && !CheckCollisionRecs(finalRect, tileMapRec))
        {
        cam.followState = MyCamera::FOLLOW_ALONG_AXIS;
        return;
    }
    
    cam.followState = MyCamera::LOCK_TO_MAP;
    
    }

inline bool8 UpdateCamera(bool refocus = false)
{
    
    MyCamera & cam = gameState->camera;
    
    bool8 updated = false;
    
    Entity * followEnt = GetEntity(cam.followEntityIndex);
    if (!followEnt) return false;
    
    SetCamFollowState(cam, followEnt);
    
    Vector2 followPos = followEnt->pivot;
    
    switch (cam.followState)
    {
        case MyCamera::LOCK_TO_MAP:
        {
             FindTileMapResult result = FindTileMap(followEnt->tilePos);
            if (result.map)
            {
                Map * map = result.map;
                int32 mapIndex = result.mapIndex;
                Vector2 pos = TilePositionToPixelPosition(map->width * 0.5f + map->tilePos.x + 0.5f, 
                                                          map->height * 0.5f + map->tilePos.y + 0.5f);
                
                if (cam.followEntityIndex == gameState->playerEntityIndex)
                {
                    gameState->playerMapIndex = mapIndex;
                }
                
                if (cam.tweenController.NoTweens() && (JustPressed(RECOVER_KEY) || refocus))
                {
                    UpdateCameraToTileMapSmooth(*map, pos, mapIndex);
                }
                
                if (!Vector2Equals(pos, cam.base.target))
                {
                    if (IsSlime(followEnt) && 
                        followEnt->tweenController.NoTweens() && 
                        !map->firstEnter)
                    {
                        UndoState::EntityArray ea = GetCurrentStateEntities();
                        map->initUndoState.playerIndex = gameState->playerEntityIndex;
                        map->initUndoState.undoEntities.clear();
                        map->initUndoState.undoEntities.insert(map->initUndoState.undoEntities.begin(),
                                                              &ea.entities[0], 
                                                              &ea.entities[ea.entityCount]);
                        map->firstEnter = true;
                    }
                    
                    if (gameState->currentMapIndex == -1)
                    {
                        gameState->screenWidth = GetScreenWidth();
                        gameState->screenHeight = GetScreenHeight();
                        
                        gameState->currentMapIndex = mapIndex;
                        
                        cam.base.rotation = 0.0f;
                        cam.base.target = pos;
                        cam.base.zoom = GetCameraZoom(*map);
                        cam.base.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
                    }
                    
                    if (gameState->currentMapIndex != mapIndex)
                    {
                        UpdateCameraToTileMapSmooth(*map, pos, mapIndex);
                    }
                    
                    updated = true;
                }
                
                break;
            }
             cam.base.target = Vector2Lerp(cam.base.target, followEnt->pivot, 5 * GetFrameTime());
            break;
            }
        case MyCamera::FOLLOW_CENTER:
        {
            if (cam.tweenController.NoTweens())
            {
                Vector2 center = Vector2Add(followEnt->pivot, 
                                            Vector2 
                                            {
                                                followEnt->tileSize * 0.5f,
                                                followEnt->tileSize * 0.5f 
                                            });
                
                // TODO adjust move and zoom speed based on move and zoom distance
                
                TweenParams params = {};
                params.paramType = PARAM_TYPE_VECTOR2;
                params.startVec2 = cam.base.target;
                params.endVec2 = center;
                params.realVec2  = &cam.base.target;
                AddTweenUnique(cam.tweenController, CreateTween(params, CAMERA_MOVE_FUNC, CAMERA_MOVE_SPEED));
                OnPlayEvent(&cam.tweenController);
                // cam.base.target = Vector2Lerp(cam.base.target, center, 5 * GetFrameTime());
            }
            break;
        }
        case MyCamera::FOLLOW_ALONG_AXIS:
        {
            Vector2 moveDir = GetTilePivot(followEnt) - followEnt->pivot;
            Vector2 center = Vector2Add(followEnt->pivot, 
                                        Vector2 
                                        {
                                            followEnt->tileSize * 0.5f,
                                            followEnt->tileSize * 0.5f 
                                        });
            Vector2 camPos = cam.base.target;
            
            if ((Sign(center.x - cam.base.target.x) == Sign(moveDir.x)))
            {
                cam.base.target.x = Lerp(camPos.x, center.x, 10 * GetFrameTime());
            }
            
            if ((Sign(center.y - cam.base.target.y) == Sign(moveDir.y)))
            {
                cam.base.target.y = Lerp(camPos.y, center.y, 10 * GetFrameTime());
            }
            break;
        }
        }
    
    if (GetScreenWidth() != gameState->screenWidth || GetScreenHeight() != gameState->screenHeight)
    {
        cam.base.zoom = GetCameraZoom(gameState->tileMaps[gameState->currentMapIndex]);
        cam.base.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        gameState->screenWidth = GetScreenWidth();
        gameState->screenHeight = GetScreenHeight();
    }
    
    if (!cam.tweenController.NoTweens())
    {
        cam.tweenController.Update();
    }
    
    return updated;
}


inline void SetUndoEntities(std::vector<Entity> & undoEntities)
{
    if (gameState->entities.count < (uint32)undoEntities.size())
        gameState->entities.count = (uint32)undoEntities.size();
    
    for (int32 i = 0; i < undoEntities.size(); i++)
    {
        Entity & e = undoEntities[i];
        gameState->entities[e.entityIndex] = e;
        gameState->entities[e.entityIndex].tweenController.Reset();
        gameState->entities[e.entityIndex].pivot = GetTilePivot(&e);
        
        if (IsSlime(&e) && (e.actionState == ANIMATE_STATE))
        {
            gameState->entities[e.entityIndex].actionState = MOVE_STATE;
        }
        
    }
    }



inline void Undo()
{
    UndoState & undoState = gameState->undoStack.back();
    gameState->playerEntityIndex = undoState.playerIndex;
    gameState->camera.followEntityIndex = gameState->playerEntityIndex;
    
    std::vector<Entity> & undoEntities = undoState.undoEntities;
    SetUndoEntities(undoEntities);        
    gameState->undoStack.pop_back();
    UpdateCamera(true);
}


inline void Restart()
{
    UndoState::EntityArray ea = GetCurrentStateEntities();
    gameState->undoStack.push_back(gameState->playerEntityIndex, ea);
    Map & currentMap = gameState->tileMaps[gameState->currentMapIndex];
    
    UndoState & initState = currentMap.initUndoState;
    gameState->playerEntityIndex = initState.playerIndex;
    
    std::vector<Entity> & initEntities = initState.undoEntities;
    SetUndoEntities(initEntities);
    
}

bool8 MoveAction(IVec2 actionDir)
{
    Entity * player = GetEntity(gameState->playerEntityIndex);
    SM_ASSERT(player, "player is not active");
    if (!player->attach) return false;
    float moveSpeed = 4.0f;
    IVec2 currentPos = player->tilePos;
    IVec2 actionTilePos = currentPos + actionDir;
    if (player->attachDir == actionDir)
    {
        return false;
    }
    
    EntityLayer layers[] = { LAYER_DOOR };
        Entity * door = FindEntityByLocationAndLayers(currentPos, layers, ArrayCount(layers));
        if (door && DoorBlocked(door, -actionDir))
        {
            if (actionDir == -player->attachDir)
            {
            PushResult rResult = ActionCheck(player, player->attachDir, CHECK_MOVE);
            if (rResult.state != PUSH_BLOCKED)
            {
                    return true;
            }
            }
            
            Vector2 dir = { (float)actionDir.x, (float)actionDir.y };
            Vector2 startPivot = GetTilePivot(player);
            Vector2 middlePivot = Vector2Add(startPivot, Vector2Scale(dir, .2f * (MAP_TILE_SIZE - player->tileSize)));
            
            player->attach = true;
            player->attachedEntityIndex = door->entityIndex;
            player->attachDir = actionDir;
            
            Vector2 endPivot = Vector2Subtract(GetTilePivot(player), Vector2Scale(dir, 5.0f));
            
            TweenParams params1 = {};
            params1.paramType = PARAM_TYPE_VECTOR2;
            params1.startVec2 = startPivot;
            params1.endVec2 = middlePivot;
            params1.realVec2  = &player->pivot;
            
            TweenParams params2 = {};
            params2.paramType = PARAM_TYPE_VECTOR2;
            params2.startVec2 = middlePivot;
            params2.endVec2 = endPivot;
            params2.realVec2  = &player->pivot;
            
            float dist = Vector2Distance(startPivot, middlePivot);
        float tileDist = dist / MAP_TILE_SIZE;
        
        float speed = (tileDist > 1) ?  BOUNCE_SPEED : MOVE_SPEED;
            
            uint32 channel = AddTweenUnique(player->tweenController, CreateTween(params1, PLAYER_MOVE_FUNC, speed, tileDist));
            
            AddTween(player->tweenController, CreateTween(params2, PLAYER_MOVE_FUNC, speed  * 2), channel);
            OnPlayEvent(&player->tweenController);
            return true;
        }
        
    {
        EntityLayer layers[] = { LAYER_PIT };
        Entity * pit = FindEntityByLocationAndLayers(currentPos + player->attachDir, layers, ArrayCount(layers));
        if (pit)
        {
            return false;
        }
    }
    
    PushResult pushResult = ActionCheck(player, actionDir, CHECK_MOVE);
    
    UpdateSlimes();
    
    switch(pushResult.state)
    {
        case PUSH_NONE:
        {
            return false;
            }
        case PUSH_MOVED:
        {
            if (player->attachDir == -actionDir)
            {
                if (pushResult.pushing)
                {
                    if (!door) ActionCheck(player, player->attachDir, CHECK_MOVE);
                    
                    SetSlimeSprite(player, actionDir);
                    
                return true;
                }
                return false;
            }
            
            IVec2 actionTilePos = player->tilePos + actionDir;
            // NOTE: no obsticale, move player
            IVec2 standingPlatformPos = actionTilePos + player->attachDir;
            FindAttachableResult findResult = FindAttachable(standingPlatformPos, player->attachDir);
            if (findResult.has)
            {
                Entity * resultEntity = findResult.entity;
                if (resultEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                    resultEntity->cableType == CABLE_TYPE_DOOR &&
                    !SameSide(resultEntity, standingPlatformPos, player->attachDir))
                {
                    return false;
                }
                SetSlimeSprite(player, actionDir);
                MoveEntity(player, findResult.entity, nullptr, actionTilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
                }
            else
            {
                EntityLayer layers[] = { LAYER_SLIME  };
                Entity * ent = FindEntityByLocationAndLayers(standingPlatformPos, layers, ArrayCount(layers));
                
                if (ent)
                {
                    player = MergeSlimes(ent, player);
                }
                else if (Abs(player->attachDir) != Abs(actionDir))
                {
                    IVec2 newTile = standingPlatformPos;
                    IVec2 newAttach = - actionDir;
                    
                    Entity * attachedEntity = GetEntity(player->attachedEntityIndex);
                    
                    if (attachedEntity && attachedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                        attachedEntity->cableType == CABLE_TYPE_DOOR &&
                        !SameSide(attachedEntity, newTile, newAttach))
                    {
                        return false;
                    }
                    
                    SetSlimeSprite(player, player->attachDir);
                    
                    MoveEntity(player, attachedEntity, nullptr, newTile, PLAYER_MOVE_FUNC, MOVE_SPEED);
                }
                else 
                {
                    return false;
                }
            }
            return true;
        }
        case PUSH_BLOCKED:
        {
            bool8 blockedByPit = (pushResult.blockedEntity->type == ENTITY_TYPE_PIT);
            bool8 blockedByDoor = 
                pushResult.blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                pushResult.blockedEntity->cableType == CABLE_TYPE_DOOR &&
                !SameSide(pushResult.blockedEntity, player->tilePos + actionDir, actionDir);
            
            if (blockedByPit || blockedByDoor)
            {
                return false;
            }
            
            if (!door && player->attachDir == -actionDir)
            {
                PushResult rResult = ActionCheck(player, player->attachDir, CHECK_MOVE);
                if (rResult.state == PUSH_BLOCKED)
                {
                    MoveEntity(player, pushResult.blockedEntity, nullptr, player->tilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
                }
                
                SetSlimeSprite(player, actionDir);
                
                return true;
            }
            
            SetSlimeSprite(player, -player->attachDir);
            MoveEntity(player, pushResult.blockedEntity, nullptr, player->tilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
            return true;
            }
        case PUSH_MERGED:
        {
            if (!door)
            {
                MergeSlimes(pushResult.mergeEntity, player);
                return true;
            }
            return false;
            }
        
    }
    
    SM_ASSERT(false, "check case failed");
    return false;
}

bool8 SplitAction(Entity * player, IVec2 bounceDir)
{
     
    if (player->mass < 2) return false;
    
    uint32 oldAttachIndex = player->attachedEntityIndex;
    IVec2 oldAttachDirection = player->attachDir;
    
    player->mass--;
    player->tileSize = GetSlimeSize(player);
    
    Entity * clone = CreateSlimeClone(player);
    player->attach = false;
    player->pivot = GetTilePivot(player);
    ActionCheck(player, bounceDir, CHECK_PROJECT);
    ActionCheck(clone, -bounceDir, CHECK_PROJECT);
    
    Entity * playerAttach = GetEntity(player->attachedEntityIndex);
    Entity * cloneAttach = GetEntity(clone->attachedEntityIndex);
    
    if ((playerAttach->tilePos - player->attachDir) == (cloneAttach->tilePos - clone->attachDir))
    {
        player->tweenController.Reset();
        player->mass += clone->mass;
        player->tileSize = GetSlimeSize(player);
        player->attach = true;
        player->attachedEntityIndex = oldAttachIndex;
        player->attachDir = oldAttachDirection;
        player->pivot = GetTilePivot(player);
        DeleteEntity(clone);
        return false;
        }
    
    return true;
}


inline void DrawSpriteLayers(EntityLayer * layers, int32 arrayCount)
{
    for (int32 layerIndex = 0; layerIndex < arrayCount; layerIndex++)
    {
        int32 layer = layers[layerIndex];
        
        auto & entityIndexArray = gameState->entityTable[layer];
        
        for (uint32 i = 0; i < entityIndexArray.count; i++)
        {
            Entity * entity = GetEntity(entityIndexArray[i]);
            if (entity)
            {
                Color color = entity->color;
                if (entity->actionState == FREEZE_STATE)
                {
                    color = LIME;
                }
                
                DrawSprite(gameState->camera.base, gameState->texture, entity->sprite, entity->pivot, entity->tileSize, color);
                
#if 0
                
                if (!IsSlime(entity) && FindAttachSlime(entity))
                {
                    color = BLUE;
                }
                
                if (IsSlime(entity))
                {
                    DrawCircleV(Vector2Add(entity->pivot, {entity->tileSize/2, entity->tileSize/2}), 3, ColorAlpha(YELLOW, 0.8f));
                    DrawTile(PivotToTilePos(entity->pivot, entity->tileSize), ColorAlpha(RED, 0.5f));
                    }

                if (IsSlime(entity))
                {
                    real32 halfSize = entity->tileSize/2;
                    Vector2 center = Vector2Add(entity->pivot, {halfSize, halfSize});
                    Vector2 attachPos = Vector2Add(center, { halfSize * entity->attachDir.x, halfSize * entity->attachDir.y });
                    
                    IVec2 parallel = { entity->attachDir.y, entity->attachDir.x };
                    
                    Vector2 A = Vector2Add(attachPos, { parallel.x * halfSize, parallel.y * halfSize });
                    Vector2 B = Vector2Subtract(attachPos, { parallel.x * halfSize, parallel.y * halfSize });
                    DrawLineEx(A, B, 5, BLUE);
                    
                }
#endif
                
                
                }
        }
    } 
}

inline bool8 SelectNextAsPlayer(Entity * player = nullptr)
{
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
    bool8 result = false;
    Entity * nextPlayerEntity = nullptr;
    for (uint32 i = 0; i < slimeEntityIndices.count; i++)
    {
        Entity * slime = GetEntity(slimeEntityIndices[i]);
        if (slime && (slime != player))
        {
            nextPlayerEntity = slime;
            break;
        }
    }
    
    if (nextPlayerEntity)
    {
        gameState->playerEntityIndex = nextPlayerEntity->entityIndex;
        gameState->camera.followEntityIndex = gameState->playerEntityIndex;
        
        if (player) player->color = GRAY;
        
        player = GetEntity(gameState->playerEntityIndex);
        player->color = WHITE;
         result = true;
    } 
    
    return result;
}

inline bool8 SlimeSelection(Entity * player)
{
    
    bool8 stateChanged = false;
    
    if (JustPressed(POSSES_KEY))// && gameState->lv2Map && gameState->lv2Map->firstEnter)
    {
        stateChanged = SelectNextAsPlayer(player);
        }
    
    return stateChanged;
}

#if 0
void UpdateSprite(EntityLayer layer)
{
    auto & entityIndexArray = gameState->entityTable[layer];
    IVec2 dir[4] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
    
    for (uint32 i = 0; i < entityIndexArray.count; i++)
    {
        Entity * entity = GetEntity(entityIndexArray[i]);
        if (entity)
        {
            IVec2 offset = { 0 };
            int32 spriteSizeX = entity->sprite.spriteSize.x;
            int32 spriteSizeY = entity->sprite.spriteSize.y;
            entity->sprite = GetSprite(entity->spriteID);
            if (layer == LAYER_WALL) offset = { spriteSizeX, spriteSizeY };
            if (layer == LAYER_GLASS) offset = { spriteSizeX, 7 * spriteSizeY };   
            entity->sprite.altasOffset = entity->sprite.altasOffset + offset;
        }
    }
}
#endif

void GameplayUpdateAndRender()
{
    
    // NOTE: Debug Camera Control
    {
        // NOTE: CameraZoom
        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        gameState->camera.base.zoom = expf(logf(gameState->camera.base.zoom) + ((float)GetMouseWheelMove()*0.1f));
        
        // NOTE: Camera Drag
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 mouseDelta = GetMouseDelta();
            gameState->camera.base.target.x -= mouseDelta.x;
            gameState->camera.base.target.y -= mouseDelta.y;
        }
        
        //if (gameState->camera.base.zoom > 10.0f) gameState->camera.base.zoom = 10.0f;
        if (gameState->camera.base.zoom < 0.1f) gameState->camera.base.zoom = 0.1f;
        
            UpdateCamera();
    }
    
    // NOTE: Recored if State Changes
    bool8 stateChanged = false;
    
    // NOTE: Actions
    if (GetPlayer() && !gameState->simulating) {
        
        Entity * player = GetEntity(gameState->playerEntityIndex);
        
        UndoState::EntityArray prevState = GetCurrentStateEntities();
        uint32 prevPlayerIndex = gameState->playerEntityIndex;
        
        // NOTE SlimeSelection
        stateChanged = SlimeSelection(player);
        UpdateElectricDoor();
        
        {
            switch(player->actionState)
            {
                case MOVE_STATE:
                {
                    
                    IVec2 actionDir = { 0 };
                    // NOTE: read input
                    if (JustPressed(SPLIT_KEY))
                    {
                         actionDir= -player->attachDir;
                        
                        stateChanged = stateChanged || SplitAction(player, actionDir);
                        }
                    else
                    {
                    
                    bool8 isPressed = false;
                    
                    if (IsDown(LEFT_KEY))
                    {
                        actionDir = { -1 , 0};                    
                        isPressed = true;
                    }
                    
                    if (IsDown(RIGHT_KEY))
                    {
                        actionDir = {1, 0};
                        isPressed = true;
                    }
                    
                    if (IsDown(UP_KEY))
                    {
                        actionDir = {0, -1};
                        isPressed = true;
                    }
                    
                    if (IsDown(DOWN_KEY))
                    {
                        actionDir = {0, 1};
                        isPressed = true;
                    }

                    if (isPressed)
                    {
                        stateChanged = stateChanged || MoveAction(actionDir);
                        }
                    }
                    if (stateChanged) 
                    {
                        SetSlimeSprite(player, actionDir);
                    }
                    
                    break;
                }
                }
            
        }
        
        if (stateChanged)
        {
            gameState->undoStack.push_back(prevPlayerIndex, prevState);
            }
        
    }
    
    SetFreeze();
        UpdateSlimes();
    
        // NOTE: Undo and Restart
        {
            static bool8 repeat = false;
            static float timeSinceLastPress = 0;
            
            timeSinceLastPress -= GetFrameTime();
            
            if (timeSinceLastPress < 0 && IsDown(UNDO_KEY) && !gameState->undoStack.empty())
            {
                // NOTE: Undo
                Undo();
                timeSinceLastPress = press_freq;
                repeat = false;
            }
        
        #if 0
            // NOTE: Restart States
            repeat = repeat && !stateChanged;
            if (JustPressed(RESET_KEY) && !repeat)
            {
                repeat = true;
                Restart();
        }
        #endif
        }
    
    // NOTE: Keys and Locks
    {
        auto & slimeIndexTable = gameState->entityTable[LAYER_SLIME];
        for (uint32 slimeIndex = 0; slimeIndex < slimeIndexTable.count; slimeIndex++)
        {
            Entity * slime = GetEntity(slimeIndexTable[slimeIndex]);
            if (slime)
            {
                auto & keyTable = gameState->entityTable[LAYER_KEY_LOCK];
                for (uint32 keyLockIndex = 0; keyLockIndex < keyTable.count; keyLockIndex++)
                {
                    Entity * key = GetEntity(keyTable[keyLockIndex]);
                    if (key && key->type == ENTITY_TYPE_KEY && 
                        PivotToTilePos(slime->pivot, slime->tileSize) == key->tilePos)
                    {
                        Entity * lock = GetEntity(key->unlockEntityIndex);
                        if (!lock->open)
                        {
                            lock->open = true;
                            float delayTime = 1.0f;
                            TweenParams params = { 0 };
                            params.paramType = PARAM_TYPE_COLOR;
                            params.startColor = WHITE;
                            params.endColor = ColorAlpha(WHITE, 0.0f);
                            params.realColor = &lock->color;
                            AddTweenUnique(lock->tweenController, CreateTween(params, nullptr, 1.0f, delayTime));
                            TweenEvent deleteEvent;
                            deleteEvent.deleteEntity = key;
                            lock->tweenController.endEvents.Add(deleteEvent);
                            TweenEvent deleteEvent2;
                            deleteEvent2.deleteEntity = lock;
                            lock->tweenController.endEvents.Add(deleteEvent2);
                            OnPlayEvent(&lock->tweenController);
                            
                        }
                        break;
                    }
                }
            }
        }
        
    }
    
    // NOTE: Simulate
    {
        gameState->simulating = false;
        // NOTE: Update: Entity
        EntityLayer simulateLayers[] = { LAYER_SLIME, LAYER_BLOCK, LAYER_KEY_LOCK };
        for (uint32 idx = 0; idx < ArrayCount(simulateLayers); idx++)
        {
            uint32 layer = simulateLayers[idx];
            auto & entList = gameState->entityTable[layer];
            for (uint32 entIdx = 0; entIdx < entList.count; entIdx++)
            {
                Entity * entity = GetEntity(entList[entIdx]);
            if (entity)
            {
                if (!entity->tweenController.NoTweens())
                {
                    gameState->simulating = true;
                    if (IsSlime(entity))
                    {
                        if (entity->actionState == MOVE_STATE) SetActionState(entity, ANIMATE_STATE);
                    }
                        entity->tweenController.Update();
                        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
                        
                        if (//(!followEnt || followEnt->tweenController.NoTweens()) && 
                            (!followEnt || (entity->tilePos - followEnt->tilePos).SqrMagnitude() > 1)&& entity->tweenController.playing)
                        {
                            gameState->camera.followEntityIndex = entity->entityIndex;
                        }
                        else if (layer == LAYER_KEY_LOCK)
                        {
                            gameState->camera.followEntityIndex = entity->entityIndex;
                        }
                }
                else
                {
                    if (entity->actionState == ANIMATE_STATE) SetActionState(entity, MOVE_STATE);
                }
            } 
        }
        }
        
        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
        if (!followEnt || stateChanged || 
            (followEnt->tweenController.NoTweens() && JustPressed(RECOVER_KEY)))
        {
            gameState->camera.followEntityIndex = gameState->playerEntityIndex;
        }
        
    }
    
    if (!GetPlayer())
    {
        SelectNextAsPlayer();
    }
    
    
    // NOTE: tutorials
    if (gameState->currentScreen == GAME_MAIN_SCREEN)
    {
    Entity * player = GetPlayer();
    if (player && player->tweenController.NoTweens())
    {
        EntityLayer layer[] = { LAYER_BLOCK };
        Entity * portal = FindEntityByLocationAndLayers(player->tilePos, layer, 1);
        if (portal && portal->type == ENTITY_TYPE_TUT_PORTAL)
        {
            gameState->lastState = gameState->undoStack.back();
            
            for (uint32 idx = 0; idx < gameState->lastState.undoEntities.size(); idx ++)
            {
                Entity * e = &gameState->lastState.undoEntities[idx];
                if (portal->entityIndex == e->entityIndex)
                {
                    portal = e;
                }
                }
            
            if (portal->tileID == TUT_1)
            {
                    portal->mass = 1;
                    SetEntitySprite(portal, BLOCK);
                }
            else if (portal->tileID == TUT_2)
            {
                portal->mass = 2;
                    SetEntitySprite(portal, BLOCK_2);
            }
            
            portal->movable = true;
            portal->type = ENTITY_TYPE_BLOCK;
            portal->color = WHITE;
                gameState->lastTutBlockIndex = portal->entityIndex;
            
            CleanUpGame();
            
             char * currentMapID = gameState->tileMaps[gameState->currentMapIndex].mapID;
                const char * levelPath = GetDirectoryPath(MAIN_PATH);
                
            char worldPath[100];
            sprintf(worldPath, "%s/%s_Tutorial.world", levelPath, currentMapID);
            
            LoadTileMapsAndEntities(*gameState, worldPath);
            gameState->currentScreen = GAME_TUT_SCREEN;
            for (;GetKeyPressed() > 0;) {} // NOTE: Flush all the pressed key
            return;
        }
        }
    }
    else if (gameState->currentScreen == GAME_TUT_SCREEN)
    {
        auto goalTable = gameState->entityTable[LAYER_PORTAL];
        bool8 end = true;
        for (uint32 i = 0; i < goalTable.count; i++)
        {
            Entity * goal = GetEntity(goalTable[i]);
            
            if (goal->type == ENTITY_TYPE_MAIN_PORTAL)
            {
            EntityLayer layers[] = { LAYER_BLOCK };
            Entity * block = FindEntityByLocationAndLayers(goal->tilePos, layers, 1);
                if (!block || !block->tweenController.NoTweens())
                {
                    end = false;
                    break;
                }
            }
            else if (goal->type == ENTITY_TYPE_SLIME_PORTAL)
            {
                EntityLayer layers[] = { LAYER_SLIME };
                Entity * slime = FindEntityByLocationAndLayers(goal->tilePos, layers, 1);
                
                if (!slime || !slime->tweenController.NoTweens())
                {
                    end = false;
                    break;
                }
                
                Entity * attach = slime->attach ? GetEntity(slime->attachedEntityIndex) : nullptr;
                if (!attach || attach->type != ENTITY_TYPE_BLOCK)
                {
                    end = false;
                    break;
                }
                
                EntityLayer checkLayers[] = { LAYER_PORTAL };
                Entity * portal = FindEntityByLocationAndLayers(attach->tilePos, checkLayers, 1);
                if (!portal)
                {
                    end = false;
                    break;
                }
                
            }
            else
            {
                SM_ASSERT(false, "unknown type");
            }
        }
        
        if (end)
        {
            Entity player = *GetPlayer();
            CleanUpGame();
            LoadTileMapsAndEntities(*gameState, MAIN_PATH);
            gameState->playerEntityIndex = gameState->lastState.playerIndex;
            SetUndoEntities(gameState->lastState.undoEntities);
            gameState->currentScreen = GAME_MAIN_SCREEN;
            for (;GetKeyPressed() > 0;) {} // NOTE: Flush all the pressed key
            
            for (uint32 i = 0; i < Source_Indices.count; i++)
            {
                Entity * source = GetEntity(Source_Indices[i]);
                source->sourceLit = source->conductive = false;
            }
            UpdateElectricDoor();
            
            // Setup Play attach
            Entity * attachBlock = GetEntity(gameState->lastTutBlockIndex);
            SM_ASSERT(attachBlock, "attach block gone");
            SetEntityPosition(GetPlayer(), attachBlock, attachBlock->tilePos - player.attachDir);
            return;
            }
        
    }
    
    // NOTE: Render
    {
        
        BeginTextureMode(gameState->renderTarget);
        ClearBackground(gameState->bgColor);
        
        int mn = Min(GetScreenWidth(), GetScreenHeight());
        
        UpdateAndDrawStarFieldBG(&gameState->starFields, 
                                 (GetScreenWidth() - mn) / 2,
                                 (GetScreenHeight() - mn) / 2);
        BeginMode2D(gameState->camera.base);
        
        
        Color colors[] = {
            LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, BLANK, MAGENTA, RAYWHITE,
        };
        
        int32 colorCount = ArrayCount(colors);
        
        static Color * mColors = (Color *)BumpAllocArray(gameMemory->persistentStorage,
                                                   gameState->tileMapCount,
                                                   sizeof(Color));
        
        static bool * set = (bool *)BumpAllocArray(gameMemory->persistentStorage,
                                                        gameState->tileMapCount,
                                                        sizeof(bool));
        
        #if 0
        for (int32 mapIndex = 0; mapIndex < gameState->tileMapCount; mapIndex++)
        {
            if (!set[mapIndex])
            {
                set[mapIndex] = true;
                int32 colorIndex = GetRandomValue(0, colorCount-1);
                mColors[mapIndex] = colors[colorIndex];
            }
            
            Map & tileMap = gameState->tileMaps[mapIndex];
            
            DrawTileMap(gameState->camera, tileMap.tilePos, 
                        IVec2{ tileMap.width, tileMap.height },
                        mColors[mapIndex], mColors[mapIndex]);
        }
#endif
        
        EntityLayer orderedDrawLayers[] = 
        {
            LAYER_WALL, 
            LAYER_CABLE,
            LAYER_SOURCE,
            LAYER_CONNECTION,
            LAYER_PIT,
            LAYER_KEY_LOCK,
            LAYER_PORTAL,
            LAYER_SLIME,
            LAYER_BLOCK,
            LAYER_GLASS,  
            LAYER_DOOR,
            };
        
        int32 count = ArrayCount(orderedDrawLayers);
        DrawSpriteLayers(orderedDrawLayers, count);
        
        // Draw rectangle outline with extended parameters
        // Rectangle cameraRect = GetCameraRect(gameState->camera);
        // DrawRectangleLinesEx(cameraRect, 1, RED);
        
        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
        
        if (followEnt)
        {
        Vector2 center = Vector2Add(followEnt->pivot, 
                                    Vector2 
                                    {
                                        followEnt->tileSize * 0.5f,
                                        followEnt->tileSize * 0.5f 
                                    });
        
        //DrawCircleV(center, 5,  RED);
        //DrawCircleV(gameState->camera.base.target, 5, YELLOW);
        }
        
        Rectangle source = GetCameraRect(gameState->camera.base);
        if (source.width > source.height) 
        {
            source.x += (source.width - source.height) * 0.5f;
            source.width = source.height;
            }
        if (source.height > source.width) 
        {
            source.y += (source.height - source.width) * 0.5f;
            source.height = source.width;
        }
        
        DrawRectangleLinesEx(source, 5, RAYWHITE);
        
        
        EndMode2D();
        EndTextureMode();
        
        // NOTE: Draw
        BeginDrawing();
        
        
        ClearBackground(gameState->bgColor);
        
        if (IsKeyPressed(KEY_R))
        {
            UnloadTexture(gameState->texture);    // Unload render texture
            gameState->texture = LoadTexture(TEXTURE_PATH);
            SetShake(0.05f);
            }
        gameState->shakeTime -= GetFrameTime();
        if (gameState->shakeTime < 0)
        {
            gameState->shake = false;
            gameState->time = 0;
            }
        
        if (IsKeyPressed(KEY_GRAVE))
        {
            gameState->enableFX = !gameState->enableFX;
        }
        
        
        if (gameState->enableFX)
        {
            static int32 shaderType = 0;
            
            if (IsKeyPressed(KEY_KP_4))
            {
                shaderType--;
            if (shaderType < 0) shaderType = (int32)(FX_COUNT) - 1;
                }
            else if (IsKeyPressed(KEY_KP_6))
            {
                shaderType++;
            if (shaderType >= FX_COUNT) shaderType = 0;
                }
            
            UpdateAndRenderWithShader(gameState->renderTarget, gameState->postFX, 
                                      shaderType, gameState->screenWidth, gameState->screenHeight,
                                      gameState->shake, gameState->time);
            }
            else
            {
            UpdateAndRenderWithShader(gameState->renderTarget, gameState->postFX, 
                                      FX_JASON, gameState->screenWidth, gameState->screenHeight,
                                      gameState->shake, gameState->time);
            }
        
#if  GAME_INTERNAL
        // NOTE: UI Draw Game Informations
        Entity * player = GetEntity(gameState->playerEntityIndex);
        if (player)
        {
        IVec2 centerPos = player->tilePos;
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera.base);
        }
        
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: %.2f,  Entity Count: %i",
                            player->tilePos.x, player->tilePos.y,
                            player->mass, player->tileSize,  gameState->entities.count), 10, 140, 20, GREEN);
        
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState->camera.base.target.x, gameState->camera.base.target.y,
                            gameState->camera.base.offset.x, gameState->camera.base.offset.y, gameState->camera.base.zoom), 10, 50, 20, RAYWHITE);
        
#if 0
        
        DrawText(TextFormat("TotalLine: %d, Connection: %d, Door: %d, TotalSource: %d",
                                     Cable_Indices.count,
                                     CP_Indices.count,
                                     Door_Indices.count,
                                     Source_Indices.count
                                     ), 10, 280, 20, GREEN);
        
        DrawText(TextFormat("TotalEntities: %d, TotalWall: %d, TotalDoor: %d, TotalCable: %d, TotalGlass: %d, TotalSlime %d, TotalBlock: %d, TotalPit: %d, TotalPortal %d", gameState->entities.count,
                                     gameState->entityTable[LAYER_WALL].count,
                                     gameState->entityTable[LAYER_DOOR].count,
                                     gameState->entityTable[LAYER_CABLE].count,
                                     gameState->entityTable[LAYER_GLASS].count,
                                     gameState->entityTable[LAYER_SLIME].count,
                                     gameState->entityTable[LAYER_BLOCK].count,
                                     gameState->entityTable[LAYER_PIT].count,
                                     gameState->entityTable[LAYER_PORTAL].count
                                     ), 10, 250, 20, GREEN);
        
        DrawText(TextFormat("Player pivot (%.2f, %.2f), mouse world (%.2f, %.2f)",
                            player->pivot.x, player->pivot.y, mousePos.x, mousePos.y ), 10, 200, 20, GREEN);
        
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: %.2f,  Entity Count: %i",
                            centerPos.x, centerPos.y,
                            player->mass, player->tileSize,  gameState->entities.count), 10, 140, 20, GREEN);
        
        DrawText(TextFormat("UndoStack Count: %d, last index: %d",
                            gameState->undoStack.count, gameState->undoStack.last - 1), 10, 170, 20, GREEN);
        
        
        int32 posX = GetScreenWidth() - MeasureText("Entity Action State: FFFFFFFFFFFFFFFFFFFF", 20);
        DebugDrawPlayerActionState(player->actionState, posX, 50, 20, IntToRGBA(0x923eed));
        
        if (gameState->simulating)
        {
            DrawText("Game Simulating", gameState->screenWidth / 4, gameState->screenHeight / 4, 20, RED);
            }
        
#endif
        
#endif
        
        DrawText(TextFormat("%.2f ms\n%iFPS", 1000.0f / GetFPS(), GetFPS()), 10, 300, 20, GREEN);
        
        EndDrawing();
    }    
}

void InitializeGame()
{
    // NOTE: Initialization
    gameState->initialized = true;
    
    InitKeyMapping();
    
    // NOTE: Initiaize slimes
    {
        auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
        
        Entity * slimeA = GetEntity(slimeEntityIndices[0]);
        Entity * slimeB = nullptr;
        if (slimeEntityIndices.count == 2)
        {
        slimeB = GetEntity(slimeEntityIndices[1]);
        }
        
        if (slimeA)
        {
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }; 
            
            for (int32 j = 0; j < 4; j++)
            {
                if (!slimeA->attach && AttachSlime(slimeA, directions[j])) break;
            }
            
            slimeA->pivot = GetTilePivot(slimeA);
        }
        
        if (slimeB)
        {
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }; 
            
            for (int32 j = 0; j < 4; j++)
            {
                if (!slimeB->attach && AttachSlime(slimeB, directions[j])) break;
            }
            
            slimeB->pivot = GetTilePivot(slimeB);
        }
        
        if (slimeA && slimeB && slimeA->tilePos == slimeB->tilePos) 
        {
            slimeA->type = ENTITY_TYPE_PLAYER;
            slimeA->color = WHITE;
            slimeA->mass++;
            slimeA->tileSize = GetSlimeSize(slimeA);
            slimeA->pivot = GetTilePivot(slimeA);
            gameState->playerEntityIndex = slimeA->entityIndex;
            DeleteEntity(slimeB);
            }
        }
    
    // NOTE: Initalize gameState->undoStack record
    gameState->undoStack.reset();
    
    gameState->currentMapIndex = -1;
    gameState->simulating = false;
    
    gameState->camera.followEntityIndex = gameState->playerEntityIndex;
    gameState->camera.followState = MyCamera::LOCK_TO_MAP;
    
    gameState->shakeTime = 0.0f;
    
}

void CleanUpGame()
{
    gameState->undoStack.reset();
    CleanUpKeyMapping();
    
    gameState->initialized = false;    
    gameState->camera.tweenController.Reset();
    for (int32 i = 0; i < LAYER_COUNT; i++)
    {
        gameState->entityTable[i].Clear();
    }
    gameState->entities.Clear();
    
}

//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================

// Called on every frame
UPDATE_AND_RENDER(UpdateAndRender)
{
    
    if (gameState != gameStateIn) gameState = gameStateIn;
    if (gameMemory != gameMemoryIn) gameMemory = gameMemoryIn;
    
    if (IsWindowResized())
    {
        UnloadRenderTexture(gameState->renderTarget);    // Unload render texture
        // gameState->renderTarget = LoadRenderTexture(mn, mn);
        gameState->renderTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    }
    
    // TODO: Temp code
    static bool8 init = false;
    if (!init)
    {
        init = true;
        GuiLoadStyle(RAYLIB_GUI_STYLE_PATH);
    }
    
    Color colorA = IntToRGBA(0x62345); // 0x163355 0x4545 0x62345
     
    gameState->bgColor = colorA;
    
    switch(gameState->currentScreen)
    {
        case TITLE_SCREEN:
        {
            
            if (JustPressed(ANY_KEY))
            {
                gameState->currentScreen = MENU_SCREEN;
                for (;GetKeyPressed() > 0;) {} // NOTE: Flush all the pressed key
            }
            
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            UpdateAndDrawStarFieldBG(&gameState->starFields);
            
            const char * Title = "TITLE SCREEN";
            int32 TitleTextX = (GetScreenWidth() - MeasureText(Title, 40)) / 2;
            int32 TitleTextY = (GetScreenHeight() - 40) / 2 - 100;
            DrawText(Title, TitleTextX, TitleTextY, 40, DARKGREEN);
            
            const char * Instructions = "PRESS Any Key to JUMP to GAMEPLAY SCREEN";
            int32 instX = (GetScreenWidth() - MeasureText(Instructions, 20)) / 2;
            int32 instY = (GetScreenHeight()) / 2;
            DrawText(Instructions, instX, instY, 20, DARKGREEN);
            
            EndDrawing();
            
            break;
        }
        case MENU_SCREEN:
        {
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            UpdateAndDrawStarFieldBG(&gameState->starFields);
            
            float width = GetScreenWidth() - 600.0f;
            float height = 100.0f;
            width = Clamp(width, 16.0f, 1000.0f);
            height = Clamp(height, 9.0f, 150.0f);
            
            const char * NewGameText = "new game";
            Rectangle bounds =
            {
                (GetScreenWidth() - width) * 0.5f,
                (GetScreenHeight() - 40) * 0.5f - 400,
                width,
                height
            };
            
            if (GuiButton(bounds, NewGameText))
            {
                LoadTileMapsAndEntities(*gameState, MAIN_PATH);
                gameState->currentScreen = GAME_MAIN_SCREEN;
            }
            
            #if 1
            // TODO: Experimental features, Very breakable!!!
            const char * LoadGameText = "load game";
            bounds.y += 200;
            if (GuiButton(bounds, LoadGameText))
            {
                
                char fileName[100];
                CatStrings(GAME_SAVE_PATH, StringLength(GAME_SAVE_PATH),
                           "Save_0", StringLength("Save_0"),
                           fileName, 100);
                if (FileExists(fileName))
                {
                    LoadTileMapsAndEntities(*gameState, MAIN_PATH);
                    
                    // IMPORTANT: Assumming game has only one level, where entities are not add/delete from staring the new game and saving the game
                    //            and the mapping array in gameState and electricDoorSystem are correct
                    int32 dataSize;
                    Entity * loadedEntities = (Entity *)LoadFileData(fileName, &dataSize);
                    int32 loadedEntityCount = dataSize / sizeof(gameState->entities[0]);
                    
                    for (int32 i = 0; i < loadedEntityCount; i++)
                    {
                        Entity & loadedEntity = loadedEntities[i];
                        
                        if (loadedEntity.type == ENTITY_TYPE_PLAYER)
                            gameState->playerEntityIndex = loadedEntity.entityIndex;
                        
                        gameState->entities[loadedEntity.entityIndex] = loadedEntity;
                    }
                    
                    // memcpy(gameState->entities.elements, data, dataSize);
                    gameState->currentScreen = GAME_MAIN_SCREEN;
                }
                else
                {
                    SM_ERROR("faile to open file %s", fileName);                    
                }
            };
#endif
            
            const char * TestLevel = "test level";
            bounds.y += 200;
            if (GuiButton(bounds, TestLevel))
            {
                LoadTileMapsAndEntities(*gameState, TEST_PATH);
                gameState->currentScreen = GAME_MAIN_SCREEN;
                
            }
            
            const char * QuitGame = "quit game";
            bounds.y += 200;
            if (GuiButton(bounds, QuitGame))
            {
                *running = false;   
            }
            
            EndDrawing();
            
            break;            
        }
        case PAUSE_MENU_SCREEN:
        {
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            UpdateAndDrawStarFieldBG(&gameState->starFields);
            
            float width = 1000.0f;
            float height = 100.0f;
            
            const char * ContinueGameText = "continue";
            Rectangle bounds =
            {
                (GetScreenWidth() - width) * 0.5f,
                (GetScreenHeight() - 40) * 0.5f - 250,
                width,
                height
            };
            
            if (GuiButton(bounds, ContinueGameText))
            {
                gameState->currentScreen = GAME_MAIN_SCREEN;
            }
            
            #if 1
            // TODO: Experimental features, Very breakable!!!
            const char * SaveGameText = "save game";
            bounds.y += 200;
            if (GuiButton(bounds, SaveGameText))
            {
                SaveGame(*gameState, GAME_SAVE_PATH);
            }
#endif
            
            const char * QuitMenuText = "quit to main menu";
            bounds.y += 200;
            if (GuiButton(bounds, QuitMenuText))
            {
                CleanUpGame();
                gameState->currentScreen = MENU_SCREEN;
            };
            
            EndDrawing();
            
            break;
        }
        case GAME_TUT_SCREEN:
        case GAME_MAIN_SCREEN:
        {
            if (!gameState->initialized)
            {
                InitializeGame();                    
            }
            
            GameplayUpdateAndRender();
            if (IsKeyPressed(KEY_ESCAPE))
            {
                gameState->currentScreen = PAUSE_MENU_SCREEN;
                for (;GetKeyPressed() > 0;) {} // NOTE: Flush all the pressed key
            }
            break;
        }
        case ENDING_SCREEN:
        {
            
            if (JustPressed(ANY_KEY))
            {
                gameState->currentScreen = TITLE_SCREEN;
            }
            
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
            
            const char * endText = "ENDING SCREEN";
            int32 endTextX = (GetScreenWidth() - MeasureText(endText, 40)) / 2;
            int32 endTextY = (GetScreenHeight() - 40) / 2 - 100;
            DrawText(endText, endTextX, endTextY, 40, DARKBLUE);
            
            const char * endInstructions = "PRESS Any Key to RETURN to TITLE SCREEN";
            int32 endInstX = (GetScreenWidth() - MeasureText(endInstructions, 20)) / 2;
            int32 endInstY = (GetScreenHeight()) / 2;
            DrawText(endInstructions, endInstX, endInstY, 20, DARKBLUE);
            
            EndDrawing();
            
            break;
        }
    }
    
}
