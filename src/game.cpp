
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

/*
TODO BUGS: FIX THE BUGS THAT NEEDS TO BE FIXED
- Fix weird animation bugs 

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

inline EntityArray GetCurrentStateEntities()
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
                int channel = c.FindMovingChannel();
                Tween & ani = c.channels[channel].last();
                
                int index = ani.endEvents.Add(TweenEvent{0});
                playEvent = &ani.endEvents[index];
                }
            if (IsSlime(current.parent->pushEnt) && current.parent->pushEnt->attachedEntityIndex == ent->entityIndex)
            {
                MoveEntity(ent, current.parent->pushEnt, playEvent, ent->tilePos + current.pushDir, BLOCK_MOVE_FUNC);
            }
            else if (IsSlime(ent) && ent->attach)
            {
                Entity * attachEntity = GetEntity(ent->attachedEntityIndex);
                MoveEntity(ent, attachEntity, playEvent, ent->tilePos + current.pushDir,  BLOCK_MOVE_FUNC);
            }
            else
            {
                MoveEntity(ent, nullptr, playEvent, ent->tilePos + current.pushDir,  BLOCK_MOVE_FUNC);
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
            int channel = ent->tweenController.FindMovingChannel();
            Tween & current = ent->tweenController.channels[channel].last();
            
            int index = current.endEvents.Add(TweenEvent{ 0 });
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
                
                MoveEntity(projectedEnt, attach, playEvent, targetPos,  BLOCK_MOVE_FUNC);
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
                    
                    MoveEntity(projectedEnt, attach, playEvent, pos - pushDir,  BLOCK_MOVE_FUNC);
                        
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
                    
                    MoveEntity(projectedEnt, attach, playEvent, pos - pushDir,  BLOCK_MOVE_FUNC);
                        return;
                }
                case ENTITY_TYPE_GLASS:
                {
                    if (!target->broken && IsSlime(projectedEnt))
                    {
                        MoveEntity(projectedEnt, target, nullptr, pos - pushDir,  BLOCK_MOVE_FUNC); 
                        return;
                    }
                    else if (!target->broken)
                    {
                    MoveEntity(projectedEnt, nullptr, playEvent, pos - pushDir, BLOCK_MOVE_FUNC);
                    
                    int channel = projectedEnt->tweenController.FindMovingChannel();
                    
                    Tween & current = projectedEnt->tweenController.channels[channel].last();
                    
                        int index = current.endEvents.Add(TweenEvent{0});
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
                    
                    MoveEntity(projectedEnt, attach, playEvent, targetPos,  BLOCK_MOVE_FUNC);
                    return;
                }
            }
        }
        
        if (CheckOutOfBound(pos))
        {
            projectedEnt->tilePos = pos;
            DeleteEntity(projectedEnt);
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
                        int index = parent->parent->pushEnt->tweenController.endEvents.Add(TweenEvent{0});
                        playEvent = &parent->parent->pushEnt->tweenController.endEvents[index];
                    }
                    MoveEntity(parent->pushEnt, nullptr, playEvent, 
                               current.pushEnt->tilePos - current.pushDir,  BLOCK_MOVE_FUNC);
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
    int newWidth = GetScreenWidth();
    int newHeight = GetScreenHeight();
    int mapMax = (currentMap.width > currentMap.height) ? currentMap.width : currentMap.height;
    
    float zoom = (zoom_per_tile / mapMax);
    (newWidth < newHeight) ? zoom *= newWidth : zoom *= newHeight;
    
    return zoom;
}

inline void UpdateCameraToTileMapSmooth(Map & map, Vector2 pos, uint32 mapIndex)
{
    gameState->cameraTweenController.Reset();
    
    // TODO adjust move and zoom speed based on move and zoom distance
    
    TweenParams params = {};
    params.paramType = PARAM_TYPE_VECTOR2;
    params.startVec2 = gameState->camera.target;
    params.endVec2 = pos;
    params.realVec2  = &gameState->camera.target;
    AddTweenUnique(gameState->cameraTweenController, CreateTween(params, CAMERA_MOVE_FUNC, 1.7f));
    
    Map & lastMap = gameState->tileMaps[gameState->currentMapIndex];
    float oldZoom = gameState->camera.zoom;
    float newZoom = GetCameraZoom(map);
    if (!FloatEquals(oldZoom, newZoom))
    {
        TweenParams params = {};
        params.paramType = PARAM_TYPE_FLOAT;
        params.startF = oldZoom;
        params.endF = newZoom;
        params.realF  = &gameState->camera.zoom;
        AddTweenUnique(gameState->cameraTweenController, CreateTween(params, CAMERA_ZOOM_FUNC, 1.7f));
    }
    
    OnPlayEvent(&gameState->cameraTweenController);
    gameState->currentMapIndex = mapIndex;
}

inline bool8 UpdateCamera(bool refocus = false)
{
    bool8 updated = false;
    
    Entity * followEnt = GetEntity(gameState->cameraFollowEntityIndex);
    if (!followEnt) return false;
    
    Vector2 followPos = followEnt->pivot;
    
    if (followEnt->type == ENTITY_TYPE_LOCK)
    {
        Vector2 center = Vector2Add(followEnt->pivot, 
                                    Vector2 
                                    {
                                        followEnt->tileSize * 0.5f,
                                        followEnt->tileSize * 0.5f 
                                    });
        
        gameState->camera.target = Vector2Lerp(gameState->camera.target, center, 10 * GetFrameTime());
        
        return true;
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
        (followEnt->tweenController.channels[followEnt->tweenController.FindMovingChannel()].last().Easing == nullptr) && 
        !CheckCollisionRecs(finalRect, tileMapRec))
    {
        gameState->cameraTweenController.Reset();
        
        Vector2 moveDir = GetTilePivot(followEnt) - followEnt->pivot;
        Vector2 center = Vector2Add(followEnt->pivot, 
                                    Vector2 
                                    {
                                        followEnt->tileSize * 0.5f,
                                        followEnt->tileSize * 0.5f 
                                    });
        Vector2 camPos = gameState->camera.target;
        
        if ((Sign(center.x - gameState->camera.target.x) == Sign(moveDir.x)))
        {
            gameState->camera.target.x = Lerp(camPos.x, center.x, 10 * GetFrameTime());
        }
        
        if ((Sign(center.y - gameState->camera.target.y) == Sign(moveDir.y)))
        {
            gameState->camera.target.y = Lerp(camPos.y, center.y, 10 * GetFrameTime());
        }
        return true;
    }
    
    for (int i = 0; i < gameState->tileMapCount; i++)
    {
        Map & map = gameState->tileMaps[i];
        Vector2 mapMin = GetTilePivot(map.tilePos, MAP_TILE_SIZE);
        
        Rectangle followRec = GetEntityRect(followEnt);
        Rectangle tileMapRec =
        {
            mapMin.x + MAP_TILE_SIZE,
            mapMin.y + MAP_TILE_SIZE,
            (float)map.width  * (float)MAP_TILE_SIZE,
            (float)map.height * (float)MAP_TILE_SIZE
        };
        
        if( CheckCollisionRecs(followRec, tileMapRec) )
        {
            
            Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, map.height * 0.5f + map.tilePos.y + 0.5f);
            
            if (gameState->cameraFollowEntityIndex == gameState->playerEntityIndex)
            {
                gameState->playerMapIndex = i;
            }
            
            if (!Vector2Equals(pos, gameState->camera.target))
            {
                if (!map.firstEnter)
                {
                    map.initUndoState = { gameState->playerEntityIndex, gameState->entities.GetVectorSTD() };
                    map.firstEnter = true;
                }
                
                if (gameState->currentMapIndex == -1)
                {
                    gameState->screenWidth = GetScreenWidth();
                    gameState->screenHeight = GetScreenHeight();
                    
                    gameState->currentMapIndex = i;
                    
                    gameState->camera.rotation = 0.0f;
                    gameState->camera.target = pos;
                    gameState->camera.zoom = GetCameraZoom(map);
                    gameState->camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
                }
                
                if (gameState->currentMapIndex != i)
                {
                    UpdateCameraToTileMapSmooth(map, pos, i);
                }
                else if (JustPressed(RECOVER_KEY) || refocus)
                {
                    UpdateCameraToTileMapSmooth(map, pos, i);
                }
                
                updated = true;
            }
            break;
        }
    }
    
    if (GetScreenWidth() != gameState->screenWidth || GetScreenHeight() != gameState->screenHeight)
    {
        gameState->camera.zoom = GetCameraZoom(gameState->tileMaps[gameState->currentMapIndex]);
        gameState->camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        gameState->screenWidth = GetScreenWidth();
        gameState->screenHeight = GetScreenHeight();
    }
    
    if (!gameState->cameraTweenController.NoTweens())
    {
        gameState->cameraTweenController.Update();
    }
    
    return updated;
}


inline void SetUndoEntities(std::vector<Entity> & undoEntities)
{
    if (gameState->entities.count < (uint32)undoEntities.size())
        gameState->entities.count = (uint32)undoEntities.size();
    
    for (int i = 0; i < undoEntities.size(); i++)
    {
        Entity & e = undoEntities[i];
        gameState->entities[e.entityIndex] = e;
        gameState->entities[e.entityIndex].tweenController.Reset();
        gameState->entities[e.entityIndex].pivot = GetTilePivot(&e);
        
        if (IsSlime(&e) && (e.actionState == ANIMATE_STATE || e.actionState == SPLIT_STATE))
        {
            gameState->entities[e.entityIndex].actionState = MOVE_STATE;
        }
        
    }
    
    // SetupEntityTable(*gameState);
}



inline void Undo()
{
    UndoState & undoState = gameState->undoStack.back();
    gameState->playerEntityIndex = undoState.playerIndex;
    gameState->cameraFollowEntityIndex = gameState->playerEntityIndex;
    
    std::vector<Entity> & undoEntities = undoState.undoEntities;
    SetUndoEntities(undoEntities);        
    gameState->undoStack.pop_back();
    UpdateCamera(true);
}


inline void Restart()
{
     EntityArray ea = GetCurrentStateEntities();
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
            
            uint32 channel = AddTweenUnique(player->tweenController, CreateTween(params1, PLAYER_MOVE_FUNC, SLIME_MOVE_SPEED, tileDist));
            
            AddTween(player->tweenController, CreateTween(params2, PLAYER_MOVE_FUNC, SLIME_MOVE_SPEED * 2), channel);
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
                
                MoveEntity(player, findResult.entity, nullptr, actionTilePos, PLAYER_MOVE_FUNC);
                }
            else
            {
                EntityLayer layers[] = {LAYER_SLIME  };
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
                    MoveEntity(player, attachedEntity, nullptr, newTile, PLAYER_MOVE_FUNC);
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
            bool8 blockedByPit = (pushResult.blockedEntity->type == ENTITY_TYPE_LOCK)
                || (pushResult.blockedEntity->type == ENTITY_TYPE_PIT);
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
                    MoveEntity(player, pushResult.blockedEntity, nullptr, player->tilePos, PLAYER_MOVE_FUNC);
                }
                return true;
                }
            MoveEntity(player, pushResult.blockedEntity, nullptr, player->tilePos, PLAYER_MOVE_FUNC);
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


inline void DrawSpriteLayers(EntityLayer * layers, int arrayCount)
{
    for (int layerIndex = 0; layerIndex < arrayCount; layerIndex++)
    {
        int layer = layers[layerIndex];
        
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
                DrawSprite(gameState->camera, gameState->texture, entity->sprite, entity->pivot, entity->tileSize, color);
                
                #if 0
                if (IsSlime(entity))
                {
                    DrawCircleV(Vector2Add(entity->pivot, {entity->tileSize/2, entity->tileSize/2}), 3, ColorAlpha(YELLOW, 0.8f));
                    DrawTile(PivotToTilePos(entity->pivot, entity->tileSize), ColorAlpha(RED, 0.5f));
                    }
                #endif
                }
        }
    } 
}

inline bool8 SlimeSelection(Entity * player)
{
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
    
    bool8 stateChanged = false;
    
    if (JustPressed(POSSES_KEY))// && gameState->lv2Map && gameState->lv2Map->firstEnter)
    {
        
        Entity * nextPlayerEntity = nullptr;
        for (uint32 i = 0; i < slimeEntityIndices.count; i++)
        {
            Entity * slime = GetEntity(slimeEntityIndices[i]);
            if (slime == player)
            {
                int nextPlayerIndex = (i + 1) % slimeEntityIndices.count;
                Entity * e = GetEntity(slimeEntityIndices[nextPlayerIndex]);
                if (e)
                {
                    nextPlayerEntity = e;
                }
                break;
            }
        }
        if (nextPlayerEntity)
        {
            player->color = GRAY;
            gameState->playerEntityIndex = nextPlayerEntity->entityIndex;
            gameState->cameraFollowEntityIndex = gameState->playerEntityIndex;
            player = GetEntity(gameState->playerEntityIndex);
            player->color = WHITE;
            stateChanged = true;
            
        } 
    }
    
    return stateChanged;
}

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
            int spriteSizeX = entity->sprite.spriteSize.x;
            int spriteSizeY = entity->sprite.spriteSize.y;
            entity->sprite = GetSprite(entity->spriteID);
            if (layer == LAYER_WALL) offset = { spriteSizeX, spriteSizeY };
            if (layer == LAYER_GLASS) offset = { spriteSizeX, 7 * spriteSizeY };   
            entity->sprite.altasOffset = entity->sprite.altasOffset + offset;
        }
    }
}

void GameplayUpdateAndRender()
{
    // NOTE: Debug Switch Monitor
    if (GetMonitorCount() > 1)
    {
        if (IsKeyPressed(KEY_ONE))
        {
            SetWindowMonitor(1);
        }
        
        if (IsKeyPressed(KEY_TWO))
        {
            SetWindowMonitor(0);
        }
        
        if (GetMonitorCount() > 2)
        {
            if (IsKeyPressed(KEY_THREE))
            {
                SetWindowMonitor(2);
            }
        }
    }
    
    // NOTE: Debug Camera Control
    {
        // NOTE: CameraZoom
        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        gameState->camera.zoom = expf(logf(gameState->camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        
        // NOTE: Camera Drag
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            Vector2 mouseDelta = GetMouseDelta();
            gameState->camera.target.x -= mouseDelta.x;
            gameState->camera.target.y -= mouseDelta.y;
        }
        
        //if (gameState->camera.zoom > 10.0f) gameState->camera.zoom = 10.0f;
        if (gameState->camera.zoom < 0.1f) gameState->camera.zoom = 0.1f;
        
            UpdateCamera();
    }
    
    if (IsKeyPressed(KEY_GRAVE))
    {
        gameState->enableFX = !gameState->enableFX;
    }
    
    // NOTE: Recored if State Changes
    bool8 stateChanged = false;
    
    // NOTE: Actions
    if (GetPlayer() && !gameState->simulating) {
        
        Entity * player = GetEntity(gameState->playerEntityIndex);
        
        EntityArray prevState = GetCurrentStateEntities();
        uint32 prevPlayerIndex = gameState->playerEntityIndex;
        
        // NOTE SlimeSelection
        stateChanged = SlimeSelection(player);
        
        {
            switch(player->actionState)
            {
                case MOVE_STATE:
                {
                    
                    // NOTE: read input
                    if (JustPressed(SPLIT_KEY))
                    {
                        IVec2 splitDir = -player->attachDir;
                        
                        stateChanged = stateChanged || SplitAction(player, splitDir);
                        
                        break;
                    }
                    
                    IVec2 actionDir = { 0 };
                    
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
                    
                    break;
                }
                }
            
        }
        
        if (stateChanged)
        {
            gameState->undoStack.push_back(prevPlayerIndex, prevState);
            }
        
        UpdateElectricDoor();
        UpdateSlimes();
        
    }
    else
    {
        
        UpdateElectricDoor();
        UpdateSlimes();
        
    }
    
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
            
            // NOTE: Restart States
            repeat = repeat && !stateChanged;
            if (JustPressed(RESET_KEY) && !repeat)
            {
                repeat = true;
                Restart();
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
                        Entity * followEnt = GetEntity(gameState->cameraFollowEntityIndex);
                        if ((!followEnt || followEnt->tweenController.NoTweens()) && layer != LAYER_KEY_LOCK && entity->tweenController.playing)
                        {
                            gameState->cameraFollowEntityIndex = entity->entityIndex;
                        }
                }
                else
                {
                    if (entity->actionState == ANIMATE_STATE) SetActionState(entity, MOVE_STATE);
                }
            } 
        }
        }
        
        Entity * followEnt = GetEntity(gameState->cameraFollowEntityIndex);
        if (!followEnt ||
            (followEnt->tweenController.NoTweens() && JustPressed(RECOVER_KEY)))
        {
            gameState->cameraFollowEntityIndex = gameState->playerEntityIndex;
        }
        
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
                            
                            gameState->cameraFollowEntityIndex = lock->entityIndex;
                            
                        }
                            break;
                        }
            }
        }
        }
        
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
            
            if (portal->spriteID == SPRITE_TUT_1)
            {
                portal->mass = 1;
                portal->spriteID = SPRITE_BLOCK;
                }
            else if (portal->spriteID == SPRITE_TUT_2)
            {
                portal->mass = 2;
                portal->spriteID = SPRITE_BLOCK_2;
            }
            
            portal->movable = true;
            portal->type = ENTITY_TYPE_BLOCK;
            portal->sprite = GetSprite(portal->spriteID);
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
        UpdateAndDrawStarFieldBG(&gameState->starFields);
        BeginMode2D(gameState->camera);
        EntityLayer orderedDrawLayers[] = 
        { 
            LAYER_PORTAL,
            LAYER_WALL, 
            LAYER_CABLE,
            LAYER_SOURCE,
            LAYER_CONNECTION,
            LAYER_PIT,
            LAYER_SLIME,
            LAYER_BLOCK,
            LAYER_GLASS,  
            LAYER_DOOR,
            LAYER_KEY_LOCK};
        
        int count = ArrayCount(orderedDrawLayers);
        DrawSpriteLayers(orderedDrawLayers, count);
        
        // Draw rectangle outline with extended parameters
        // Rectangle cameraRect = GetCameraRect(gameState->camera);
        // DrawRectangleLinesEx(cameraRect, 1, RED);
        
        Entity * followEnt = GetEntity(gameState->cameraFollowEntityIndex);
        
        if (followEnt)
        {
        Vector2 center = Vector2Add(followEnt->pivot, 
                                    Vector2 
                                    {
                                        followEnt->tileSize * 0.5f,
                                        followEnt->tileSize * 0.5f 
                                    });
        
        //DrawCircleV(center, 5,  RED);
        //DrawCircleV(gameState->camera.target, 5, YELLOW);
        }
        
        EndMode2D();
        EndTextureMode();
        
        // NOTE: Draw
        BeginDrawing();
        ClearBackground(gameState->bgColor);
        
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
            
            
            UpdateAndRenderPostShader(gameState->renderTarget, gameState->postFX, 
                                      shaderType, gameState->screenWidth, gameState->screenHeight);
            }
            else
            {
            UpdateAndRenderPostShader(gameState->renderTarget, gameState->postFX, 
                                      FX_JASON, gameState->screenWidth, gameState->screenHeight);
            }
        
#if  GAME_INTERNAL
        // NOTE: UI Draw Game Informations
        
        
        Entity * player = GetEntity(gameState->playerEntityIndex);
        if (player)
        {
        IVec2 centerPos = player->tilePos;
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
        }
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
        
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState->camera.target.x, gameState->camera.target.y,
                            gameState->camera.offset.x, gameState->camera.offset.y, gameState->camera.zoom), 10, 50, 20, RAYWHITE);
        
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: %.2f,  Entity Count: %i",
                            centerPos.x, centerPos.y,
                            player->mass, player->tileSize,  gameState->entities.count), 10, 140, 20, GREEN);
        
        DrawText(TextFormat("UndoStack Count: %d, last index: %d",
                            gameState->undoStack.count, gameState->undoStack.last - 1), 10, 170, 20, GREEN);
        
        
        int posX = GetScreenWidth() - MeasureText("Entity Action State: FFFFFFFFFFFFFFFFFFFF", 20);
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
            
            for (int j = 0; j < 4; j++)
            {
                if (!slimeA->attach && AttachSlime(slimeA, directions[j])) break;
            }
            
            slimeA->pivot = GetTilePivot(slimeA);
        }
        
        if (slimeB)
        {
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }; 
            
            for (int j = 0; j < 4; j++)
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
    
    // NOTE: SetUp Electric Door
    SetUpElectricDoor();
    
    // NOTE: Initalize gameState->undoStack record
    gameState->undoStack.reset();
    
    gameState->currentMapIndex = -1;
    gameState->simulating = false;
    
    gameState->cameraFollowEntityIndex = gameState->playerEntityIndex;
    
}

void CleanUpGame()
{
    gameState->undoStack.reset();
    CleanUpKeyMapping();
    
    gameState->initialized = false;    
    gameState->cameraTweenController.Reset();
    for (int i = 0; i < LAYER_COUNT; i++)
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
        gameState->renderTarget = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    }
    
    // TODO: Temp code
    static bool8 init = false;
    if (!init)
    {
        init = true;
        GuiLoadStyle(RAYLIB_GUI_STYLE_PATH);
    }
    
    Color colorA = IntToRGBA(0x62345);
     
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
            int TitleTextX = (GetScreenWidth() - MeasureText(Title, 40)) / 2;
            int TitleTextY = (GetScreenHeight() - 40) / 2 - 100;
            DrawText(Title, TitleTextX, TitleTextY, 40, DARKGREEN);
            
            const char * Instructions = "PRESS Any Key to JUMP to GAMEPLAY SCREEN";
            int instX = (GetScreenWidth() - MeasureText(Instructions, 20)) / 2;
            int instY = (GetScreenHeight()) / 2;
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
            
            #if 0
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
                    int dataSize;
                    Entity * loadedEntities = (Entity *)LoadFileData(fileName, &dataSize);
                    int loadedEntityCount = dataSize / sizeof(gameState->entities[0]);
                    
                    for (int i = 0; i < loadedEntityCount; i++)
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
            
            #if 0
            // TODO: Experimental features, Very breakable!!!
            const char * SaveGameText = "save game";
            bounds.y += 200;
            if (GuiButton(bounds, SaveGameText))
            {
                EntityLayer saveLayers[] = { LAYER_DOOR, LAYER_CABLE, LAYER_SOURCE, LAYER_CONNECTION, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK };
                int saveEntityCount = 0;
                for (int i = 0; i < ArrayCount(saveLayers); i++)
                {
                    saveEntityCount += gameState->entityTable[saveLayers[i]].count;
                }
                
                Entity * saveEntities = (Entity *)BumpAllocArray(gameMemory->transientStorage, saveEntityCount, sizeof(Entity));
                int index = 0;
                for (int layerIndex = 0; layerIndex < ArrayCount(saveLayers); layerIndex++)
                {
                    auto & layer = gameState->entityTable[saveLayers[layerIndex]];
                    for (uint32 i = 0; i < layer.count; i++)
                    {
                        SM_ASSERT(index < saveEntityCount, "Trying to write outside of allocated memory");
                        Entity entity = gameState->entities[layer[i]];
                        saveEntities[index++] = entity;
                    }
                }
                
                for (uint32 saveIndex = 0; ; saveIndex++)
                {
                    
                    char saveName[10];
                    sprintf(saveName, "Save_%d", saveIndex);
                    
                    char fileName[100];                
                    CatStrings(GAME_SAVE_PATH, StringLength(GAME_SAVE_PATH),
                               saveName, StringLength(saveName),
                               fileName, 100);
                    
                    if (!FileExists(fileName))
                    {
                        // Save data to file from byte array (write), returns true on success
                        if (!SaveFileData(fileName, (void *) saveEntities, sizeof(Entity) * saveEntityCount))
                        {
                            SM_ASSERT(false, "fail to save game state");
                        }
                        break;
                    }
                } 
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
            int endTextX = (GetScreenWidth() - MeasureText(endText, 40)) / 2;
            int endTextY = (GetScreenHeight() - 40) / 2 - 100;
            DrawText(endText, endTextX, endTextY, 40, DARKBLUE);
            
            const char * endInstructions = "PRESS Any Key to RETURN to TITLE SCREEN";
            int endInstX = (GetScreenWidth() - MeasureText(endInstructions, 20)) / 2;
            int endInstY = (GetScreenHeight()) / 2;
            DrawText(endInstructions, endInstX, endInstY, 20, DARKBLUE);
            
            EndDrawing();
            
            break;
        }
    }
    
}
