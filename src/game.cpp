
#include "game.h"
#include "game_util.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "render_interface.cpp"
#include "assets.cpp"
#include "entity.cpp"
#include "electric_door.cpp"
#include "assets_loader.cpp"
#include "tween.cpp"
#include "tween_controller.cpp"
#include "save_game.cpp"

/*
TODO BUGS: FIX THE BUGS THAT NEEDS TO BE FIXED
- Fix weird animation bugs 
- Redo shake shader with the godot implementation (See Below)

uniform real32 ShakeStrength = 0;
uniform vec2 FactorA  = vec2(100.0,100.0);
uniform vec2 FactorB  = vec2(1.0,1.0);
uniform vec2 magnitude = vec2(0.01,0.01);
uniform sampler2D SCREEN_TEXTURE : hint_screen_texture, filter_linear_mipmap;

void fragment() {
    vec2 uv = SCREEN_UV;
    uv -= 0.5;
    uv *= 1.0 - 2.0 * magnitude.x;
    uv += 0.5;
    vec2 dt = vec2(0.0, 0.0);
    dt.x = sin(TIME * FactorA.x+FactorB.x) * magnitude.x;
    dt.y = cos(TIME *FactorA.y+ FactorB.y) * magnitude.y;
    COLOR = texture(SCREEN_TEXTURE, SCREEN_UV + dt * ShakeStrength);
}


TODO: Things that I can do beside arts and design I guess
3. collectable: show in ui
5. Sound effect
2. saves and loads

// NOTE: done but need testing
1. background
-  reset system
2. key and door
- Undo should update the maps reset states
 */

//  ========================================================================
//              NOTE: Internal Functions (internal)
//  ========================================================================

Entity * UndoState::GetByEntityIndex(uint32 entityIndex)
{
    for (uint32 i = 0; i < undoEntities.size(); i++)
    {
        if (undoEntities[i].entityIndex == entityIndex)
        {
            return &undoEntities[i];
        }
    }
    return nullptr;
}


UndoState & UndoStack::back()
{
    return undoStack[last - 1];
}

void UndoStack::push_back(uint32 playerIndex, uint32 starCount, 
                          DynamicArray<Entity> & entityArray)
{
    last++;
    count++;
    if (last > MAX_UNDO) last = 1;
    if (count > MAX_UNDO) count = MAX_UNDO;
    InitUndoState(&undoStack[last - 1], playerIndex, starCount, entityArray);
    }

void UndoStack::pop_back()
{
    if (count > 0)
    {
        last--;
        
        if (last <= 0) last = MAX_UNDO;
        
        count--;
        
    }
}



//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================

inline Vector2 GetStarDestPosition()
{
    Vector2 result = { 0 };
    for (uint32 i = 0; i < gameState->entityTable[LAYER_STAR_DEST].count; i++)
    {
        uint32 entityIndex = gameState->entityTable[LAYER_STAR_DEST][i];
        Entity * starDest = GetEntity(entityIndex);
        
        if (starDest && !starDest->starDested)
        {
            starDest->starDested = true;
            result = starDest->pivot;
            break;
            
        }
    }
    
    return result;
}

inline bool8 BridgeBlocked(Entity * bridge, IVec2 dir)
{
    bool8 result = 
        dir == DIR_LEFT && bridge->left || 
        dir == DIR_RIGHT && bridge->right ||
        dir == DIR_UP && bridge->up ||
        dir == DIR_DOWN && bridge->down;
    
    return result;
}

void ChangeScreen(GameScreen screen)
{
    gameState->switching = true;
    gameState->nextScreen = screen;
    for (;GetKeyPressed() > 0;) {} // NOTE: Flush all the pressed key
    }

  DynamicArray<int32> FindAllMapsWithStarCount(uint32 starCount)
{
    uint32 count = 0;
    int32 * indices = (int32 *)BumpAllocArray(gameMemory->transientStorage, starCount, sizeof(int32));
    for (uint32 mapIndex = 0; mapIndex < gameState->tileMapCount; mapIndex++)
        if (gameState->tileMaps[mapIndex].visibleStarCount == starCount)
    {
        {
            indices[count++] = mapIndex;
        }
    }
    
    DynamicArray<int32> result = { 0 };
    result.count = count;
    result.elements = indices;
    
    return result;
    
}

void InitUndoState(UndoState * undoState, 
                   uint32 playerIndex, uint32 starCount,
                   DynamicArray<Entity> & ea)
{
    if (undoState)
    {
        undoState->playerIndex = playerIndex;
        undoState->starCount = starCount;
        undoState->undoEntities.clear();
        undoState->undoEntities.insert(undoState->undoEntities.begin(), 
                                       ea.elements, 
                                       ea.elements + ea.count);
        }
}

bool8 MapIsVisible(Map & tileMap)
{
    bool8 result = tileMap.visibleStarCount <= gameState->starCount;
    return result;
}

bool8 MapIsVisible(int32 mapIndex)
{
    bool8 result = false;
    if (mapIndex >= 0)
    {
        result = MapIsVisible(gameState->tileMaps[mapIndex]);
    }
    return result;
}

#if 0
void ResetResetStates()
{
    for (uint32 mapIndex = 0; mapIndex < gameState->tileMapCount; mapIndex++)
    {
        Map & map = gameState->tileMaps[mapIndex];
        map.stateInitilized = false;
    }
}
#endif 

bool8 IsDisappearing(Entity * entity)
{
    FindTileMapResult mapResult = FindTileMap(entity->tilePos);
    bool8 result = !mapResult.map || 
        !entity->tweenController.NoTweens()  && !MapIsVisible(*mapResult.map);
    
    return result;
}

uint32 TilePosToFogIndex(Fog & fog, IVec2 pos)
{
    int32 x = pos.x - fog.tileMin.x;
    int32 y = pos.y - fog.tileMin.y;
    SM_ASSERT(x >= 0, "invalid tile position");
    SM_ASSERT(y >= 0, "invalid tile position");
    
    uint32 idx = y * fog.dim.x + x;
    
    return idx;
}

// NOTE: Add fog index that's to be revealed
void RevealTile(Fog & fog, IVec2 tilePos)
{
    uint32 fogIndex = TilePosToFogIndex(fog, tilePos);
    bool8 add = true;
    
    if (ColorIsEqual(fog.fogPixels[fogIndex], BLANK))
    {
        add = false;
    }
    else
    {
    for (uint32 i = 0; i < fog.updatingIndices.count; i++)
    {
        uint32 index = fog.updatingIndices[i];
        if (index == fogIndex)
        {
            add = false;
        }
    }
    }
    
    if (add)
    fog.updatingIndices.Add(fogIndex);
    }

void RevealEntity(Entity * entity, int32 visibility)
{
    if (gameState->currentScreen == GAME_TUT_SCREEN) return;
    IVec2 renderTilePos = PivotToTilePos(entity->pivot, entity->tileSize);
    
    for (int32 y = renderTilePos.y - visibility;
         y <= renderTilePos.y + visibility;
         y++)
    {
        for (int32 x = renderTilePos.x - visibility;
             x <= renderTilePos.x + visibility;
             x++)
        {
            if (!CheckOutOfBound(x, y))
            {
                RevealTile(gameState->fog, IVec2{x, y});
                }
        }
    }
    
}

void RevealMap(Map & map)
{
    if (gameState->currentScreen == GAME_TUT_SCREEN) return;
    IVec2 fogDim = gameState->fog.tileMax - gameState->fog.tileMin;
    for (int32 x = 0; x < map.width; x++)
    {
        for (int32 y = 0; y < map.height; y++)
        {
            IVec2 pos = map.tilePos + IVec2 {x + 1, y + 1};
            RevealTile(gameState->fog, pos);
        }
    }
}

void UpdateFog()
{
    Fog & fog = gameState->fog;
    real32 updateSpeed = 5.0f;
    for (uint32 i = 0; i < fog.updatingIndices.count; i++)
    {
        uint32 updateIndex = fog.updatingIndices[i];
        fog.fogPixels[updateIndex] = ColorLerp(fog.fogPixels[updateIndex], BLANK, GetFrameTime() * updateSpeed);
        if (ColorIsEqual(fog.fogPixels[updateIndex], BLANK))
        {
            fog.updatingIndices.RemoveIdxAndSwap(i);
            i--;
        }
    }
    
}

void CollectStar(Entity * star)
{
    star->pivot = star->collectEnd;
    star->starCollecting = false;
    star->tilePos = PivotToTilePos(star->collectEnd, DEFAULT_TILE_SIZE);
    EntityLayer layer[] = { LAYER_STAR_DEST };
    Entity * d = FindEntityByLocationAndLayers(star->tilePos, layer, 1);
    if (d) DeleteEntity(d);
    
}

void ResetStars()
{
    auto & starTable  = gameState->entityTable[LAYER_STAR];
    
    for (uint32 starIndex = 0; starIndex < starTable.count; starIndex++)
    {
        if (Entity * star = GetEntity(starTable[starIndex]); star)
        {
            
            if (star->starCollecting)
            {
                CollectStar(star);
            }
        }
    }
}

void UpdateStars()
{
    auto & starTable  = gameState->entityTable[LAYER_STAR];
    
    for (uint32 starIndex = 0; starIndex < starTable.count; starIndex++)
    {
        if (Entity * star = GetEntity(starTable[starIndex]); star)
        {
            
            if (star->starCollecting)
            {
                real32 dist = Vector2Distance(star->collectStart, star->collectEnd);
                if (dist <= 0 || gameState->starT[starIndex] > 1)
                {
                    CollectStar(star);
                    }
                else
                {
                    Vector2 start = star->collectStart;
                    Vector2 end = star->collectEnd;
                    Vector2 mid = Vector2 { end.x, start.y };
                    
                    real32 t = EaseInOutSine(gameState->starT[starIndex]);
                    
                    Vector2 screen_cur = GetSplinePointBezierQuad(start, mid, end, t);
                    
                    star->pivot = screen_cur;

                    real32 maxDT = 0.003f;
                    real32 dt = 100.0f * GetFrameTime() / (dist * 0.5f);

                    dt = Clamp(dt, 0.0f, maxDT);
                    
                    gameState->starT[starIndex] += dt;
                }
                
                
                continue;
            }
            
            Vector2 starPos = GetTilePivot(star);
            
            gameState->starT[starIndex] = 0;
            
            real32 posy = starPos.y + 3 * cosf(2 * (real32)GetTime() + starPos.x);
            real32 posx = starPos.x + 3 * sinf(2 * (real32)GetTime() + starPos.y);
            
            star->pivot.y = posy;
            star->pivot.x = posx;
        }
    }
}

void SetDrawingEntities()
{
    for (uint32 i = 0; i < gameState->entities.count; i++)
    {
        Entity * ent = GetEntity(i);
        if (ent)
        {
            FindTileMapResult mapResult = FindTileMap(ent->tilePos);
            if (ent->tweenController.NoTweens() && mapResult.map && !MapIsVisible(*mapResult.map))
            {
                ent->isVisible = false;
                continue;
            }
            
            Rectangle dest =
            {
                //topLeft.x + tileSize, topLeft.y + tileSize,
                ent->pivot.x, ent->pivot.y,
                ent->tileSize.x, ent->tileSize.y
            };
            ent->isVisible =
                CheckCollisionRecs(dest, GetCameraRect(gameState->camera.base));
                }
    }
}

void SetShake(real32 duration, real32 strength)
{
    gameState->shake = true;
    gameState->shakeTime = duration;
    gameState->shakeStrength = strength;
}

#if 0
DynamicArray<UndoState::MapUndoInfo> GetCurrentMapUndoInfos()
{
    uint32 count = gameState->tileMapCount;
    UndoState::MapUndoInfo * infos =
    (UndoState::MapUndoInfo *)BumpAllocArray(gameMemory->transientStorage, count, sizeof(UndoState::MapUndoInfo));
    
    for (uint32 i = 0; i < count; i++)
    {
        Map & map = gameState->tileMaps[i];
        infos[i].mapIndex = i;
        infos[i].initilized = map.stateInitilized;
    }
    
    DynamicArray<UndoState::MapUndoInfo> result = { 0 };
    result.count = count;
    result.elements = infos;
    return result;
}
#endif

DynamicArray<Entity> GetCurrentStateEntities()
{
    EntityLayer pushLayers[] = {
        LAYER_DOOR,
          LAYER_CABLE,
        LAYER_SOURCE,
        LAYER_CONNECTION,
        LAYER_GLASS,
        LAYER_SLIME,
        LAYER_BLOCK,
        LAYER_STAR,
        LAYER_STAR_DEST,
        LAYER_LOCK,
        LAYER_LINK,
        LAYER_PORTAL,
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
            Entity ent = gameState->entities[entList[entId]];
            
            if (layer == LAYER_STAR && ent.starCollecting)
            {
                CollectStar(&ent);
            }
            
            if (layer == LAYER_SLIME)
            {
                SM_TRACE("slime pos (%d, %d)", ent.tilePos.x, ent.tilePos.y);
            }
            
            entities[i++] = ent;
            }
    }
    
    SM_ASSERT(i == len, "incorrect len");
    
    DynamicArray<Entity> result = { 0 };
    result.count = i;
    result.elements = entities;
    
    return result;
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
                int32 channel = c.FindChannelByParamType(PARAM_TYPE_VECTOR2);
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

TweenEvent * GetPlayEventFromCheckThings(CheckThings * thing)
{
    TweenEvent * playEvent = nullptr;
    while(thing->parent != thing)
    {
        Entity * ent = thing->pushEnt;
        
        if (!ent->tweenController.NoTweens())
        {
            int32 channel = ent->tweenController.FindChannelByParamType(PARAM_TYPE_VECTOR2);
            Tween & current = ent->tweenController.channels[channel].last();
            
            int32 index = current.endEvents.Add(TweenEvent{ 0 });
            playEvent = &current.endEvents[index];
            break;
        }
        thing = thing->parent;
    }
    return playEvent;
}

inline void ProjectAndCheck(Entity * projectedEnt, 
                            Array<CheckThings, 100> & checkList,
                            IVec2 pushDir, 
                            int32 & accumulatedMass,
                            EntityLayer * checkLayers, 
                            uint32 layerCount)
{
    Entity * pushEnt = checkList.last().parent->pushEnt;
    
    
    CheckThings * thing = checkList.last().parent;
    TweenEvent * playEvent = GetPlayEventFromCheckThings(thing);
    
    bool8 defered = checkList.last().parent->pushResult.state == PROJECT_DEFERRED;
    
    IVec2 finalPos = projectedEnt->tilePos;
    Entity * attach = nullptr;
    
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
                
                if (blockedEntity->type == ENTITY_TYPE_DOOR && DoorBlocked(blockedEntity, -pushDir))
                {
                    targetPos = blockedEntity->tilePos;
                }
                
                attach = defered ? pushEnt : blockedEntity;
                finalPos = targetPos;
                
                goto MoveAndStop;
            }
            
            switch(target->type)
            {
                case ENTITY_TYPE_SLIME:
                {
                    if (IsSlime(projectedEnt))
                    {
                        MergeSlimes(target, projectedEnt);
                        return;
                    }
                    
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
                    
                    finalPos = pos - pushDir;
                    
                        goto MoveAndStop;
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
                    
                    attach = target;
                    if (defered)
                    {
                        attach = pushEnt;
                    }
                    finalPos = pos - pushDir;
                    
                    goto MoveAndStop;
                }
                case ENTITY_TYPE_GLASS:
                {
                    if (!target->broken && IsSlime(projectedEnt))
                    {
                        attach = target;
                        if (defered)
                        {
                            attach = pushEnt;
                        }
                        finalPos = pos - pushDir;
                        goto MoveAndStop;
                    }
                    else if (!target->broken)
                    {
                        // NOTE setup glass sprite after entity updates
                        target->broken = true;
                        Entity * attachSlime = FindAttachSlime(target);
                        if (attachSlime) 
                        {
                            DettachSlime(attachSlime);
                        }
                        }
                    
                    break;
                }
                case ENTITY_TYPE_LOCK:
                {
                    if (target->open) break;
                }
                case ENTITY_TYPE_DOOR:
                case ENTITY_TYPE_WALL:
                case ENTITY_TYPE_BRIDGE:
                case ENTITY_TYPE_PIT:
                {
                    if (target->type == ENTITY_TYPE_DOOR)
                    {
                    bool8 blocked = DoorBlocked(target, pushDir) || DoorBlocked(target, -pushDir);
                    if (!blocked) break;
                    }
                    
                    Entity * blockedEntity = target;
                    IVec2 targetPos = blockedEntity->tilePos - pushDir;
                    
                    if (blockedEntity->type == ENTITY_TYPE_DOOR && DoorBlocked(blockedEntity, -pushDir))
                    {
                        targetPos = blockedEntity->tilePos;
                    }
                    
                    attach = blockedEntity;
                    if (defered)
                    {
                        attach = pushEnt;
                    }
                    else if (target->type == ENTITY_TYPE_BRIDGE && !BridgeBlocked(target, pushDir))
                    {
                        attach = nullptr;
                    }
                    
                    finalPos = targetPos;
                    goto MoveAndStop;
                }
            }
        }
        
        if (CheckOutOfBound(pos))
        {
            finalPos = pos + pushDir * 2;
            goto MoveAndStop;
        }
        
    }
    
    
    MoveAndStop:;
    MoveEntity(projectedEnt, attach, playEvent, finalPos, 
               BLOCK_PROJ_FUNC, BOUNCE_SPEED, !defered);
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
            case ENTITY_TYPE_DOOR:
            {
                
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
            case ENTITY_TYPE_SLIME:
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
            {
                if (target->open) break;
            }
            case ENTITY_TYPE_PIT:
            case ENTITY_TYPE_BRIDGE:
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
                    if (door && (DoorBlocked(door, -current.pushDir)))
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
                            // NOTE: slime dose not block the block if it is attach to the block, unless slime next tile is blocked...
                            if (IsSlime(ent) && GetEntity(ent->attachedEntityIndex) == target)
                            {
                                if (dirs[i] == current.pushDir)
                                {
                                    // TODO: very weird edge case behavior
                                    Entity * blockedEntity = FindEntityByLocationAndLayers(ent->tilePos + dirs[i],
                                                                                           checkLayers, layerCount);
                                    if (blockedEntity)
                                    {
                                        if ((blockedEntity->type == ENTITY_TYPE_GLASS && !blockedEntity->broken) ||
                                            blockedEntity->type == ENTITY_TYPE_DOOR && DoorBlocked(blockedEntity, dirs[i]) ||
                                            blockedEntity->type == ENTITY_TYPE_BRIDGE ||
                                            blockedEntity->type == ENTITY_TYPE_WALL ||
                                            blockedEntity->type == ENTITY_TYPE_BLOCK && blockedEntity->actionState == FREEZE_STATE)
                                        {
                                            current.pushResult.state = PUSH_BLOCKED;
                                            current.pushResult.blockedEntity = target;
                                            return;
                                        }
                                    }
                                }
                                continue;
                            }
                            else if ((ent->type == ENTITY_TYPE_DOOR) &&
                                (!SameSide(ent, ent->tilePos, dirs[i]) || !DoorBlocked(ent, dirs[i])) ||
                                (ent->type == ENTITY_TYPE_GLASS && ent->broken) ||
                                (IsSlime(ent) && (!ent->attach || dirs[i] != current.pushDir)))
                            {
                                continue;
                            }
                            else if (dirs[i] != current.pushDir && 
                                     ent->type == ENTITY_TYPE_BRIDGE && !BridgeBlocked(ent, dirs[i]))
                            {
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
    // IMPORTANT: the order of the layers are important, for example, we don't want to check blocks before checking doors in the same tile
    EntityLayer checkLayers[] = { LAYER_WALL, LAYER_DOOR, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK, LAYER_LOCK };
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
                    TweenEvent * playEvent = GetPlayEventFromCheckThings(parent->parent);
                    #if 0
                    if (!parent->parent->pushEnt->tweenController.NoTweens())
                    {
                        int32 index = parent->parent->pushEnt->tweenController.endEvents.Add(TweenEvent{0});
                        playEvent = &parent->parent->pushEnt->tweenController.endEvents[index];
                    }
                    #endif
                    IVec2 pos = current.pushEnt->tilePos - current.pushDir;
                    MoveEntity(parent->pushEnt, nullptr, playEvent, pos, BLOCK_MOVE_FUNC, BOUNCE_SPEED, false);
                    
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

inline real32 GetCameraZoom(uint32 width, uint32 height)
{
    int32 newWidth = GetScreenWidth();
    int32 newHeight = GetScreenHeight();
    
    int32 camMax = (width > height) ? width : height;
    uint32 newRes = (newWidth < newHeight) ? newWidth : newHeight;
    real32 zoom = (real32)(newRes - 150) / (real32)camMax ;
    
    if (zoom > 5) zoom = 5;
    if (zoom < 0.5f) zoom = 0.1f;
    
    return zoom;
}

inline real32 GetCameraZoom(Map & currentMap)
{
    return GetCameraZoom(currentMap.width * MAP_TILE_SIZE, currentMap.height * MAP_TILE_SIZE);
}

inline void UpdateCameraToTileMapSmooth(Map & map, real32 (*MoveFunc)(real32), real32 (*ZoomFunc)(real32),
                                        real32 moveSpeed, real32 zoomSpeed)
{
    Vector2 startPos = gameState->camera.base.target;
    real32 startZoom = gameState->camera.base.zoom;
    Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, 
                                              map.height * 0.5f + map.tilePos.y + 0.5f);
    
    if (!gameState->camera.tweenController.NoTweens())
    {
        gameState->camera.tweenController.Reset();
        gameState->camera.base.target = startPos;
        gameState->camera.base.zoom = startZoom;
}
    
    if (!Vector2Equals(gameState->camera.base.target, pos))
    {
        TweenParams params = {};
    params.paramType = PARAM_TYPE_VECTOR2;
        params.startVec2 = startPos;
    params.endVec2 = pos;
    params.realVec2  = &gameState->camera.base.target;
    AddTweenUnique(gameState->camera.tweenController, CreateTween(params, MoveFunc, moveSpeed));
    }
    
    real32 oldZoom = gameState->camera.base.zoom;
    real32 newZoom = GetCameraZoom(map);
    if (!FloatEquals(oldZoom, newZoom))
    {
        TweenParams params = {};
        params.paramType = PARAM_TYPE_FLOAT;
        params.startF = oldZoom;
        params.endF = newZoom;
        params.realF  = &gameState->camera.base.zoom;
        AddTweenUnique(gameState->camera.tweenController, CreateTween(params, ZoomFunc, zoomSpeed));
    }
    
    OnPlayEvent(&gameState->camera.tweenController);
    }

 FindTileMapResult FindTileMap(IVec2 tilePos)
{
    FindTileMapResult result = { 0 };
    for (uint32 i = 0; i < gameState->tileMapCount; i++)
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
    #if 0
    if (StarCollecting())
    {
        cam.followState = MyCamera::FOLLOW_STAR;
        return;
    }
    #endif
    if (gameState->zoomOut)
    {
        cam.followState = MyCamera::ZOOM_OUT;
        return;
    }
    
    if (followEnt->type == ENTITY_TYPE_LOCK)
    {
        cam.followState = MyCamera::FOLLOW_CENTER;
        return;
    }
    
    if (gameState->currentMapIndex >= 0)
    {
        if (gameState->prevMapIndex != gameState->currentMapIndex &&
            ((followEnt->tweenController.playing && (followEnt->tweenController.FindTweenByTweenProperty(PARAM_TYPE_VECTOR2, &followEnt->pivot)->dt == BOUNCE_SPEED)) ||
             !IsSlime(followEnt)))
    {
        cam.followState = MyCamera::FOLLOW_ALONG_AXIS;
        return;
            }
    }
    
     cam.followState = MyCamera::LOCK_TO_MAP;
    
    }

inline bool8 UpdateCamera(bool8 refocus = false)
{
    MyCamera & cam = gameState->camera;
    bool8 updated = false;
    
    Entity * followEnt = GetEntity(cam.followEntityIndex);
    if (!followEnt) return false;
    
    SetCamFollowState(cam, followEnt);
    
    Vector2 followPos = followEnt->pivot;
    
    switch (cam.followState)
    {
        case MyCamera::ZOOM_OUT:
        {
            if (cam.tweenController.NoTweens())
            {
                real32 oldZoom = gameState->camera.base.zoom;
                real32 newZoom =
                    GetCameraZoom(33 * MAP_TILE_SIZE,33 * MAP_TILE_SIZE);
                if (!FloatEquals(oldZoom, newZoom))
                {
                    TweenParams params = {};
                    params.paramType = PARAM_TYPE_FLOAT;
                    params.startF = oldZoom;
                    params.endF = newZoom;
                    params.realF  = &gameState->camera.base.zoom;
                    AddTweenUnique(gameState->camera.tweenController, CreateTween(params, 
                                                                                  CAMERA_ZOOM_FUNC, CAMERA_ZOOM_SPEED));
                }
                OnPlayEvent(&cam.tweenController);
            }
            
            break;
        }
        case MyCamera::LOCK_TO_MAP:
        {
        if (gameState->currentMapIndex == -1)
            {
                SM_ASSERT(IsSlime(followEnt), "Initial follow entity must be player");
                gameState->screenWidth = GetScreenWidth();
                gameState->screenHeight = GetScreenHeight();
                
                FindTileMapResult result = FindTileMap(followEnt->tilePos);
                SM_ASSERT(result.map, "start position must be in a tileMap");
                
                gameState->currentMapIndex = result.mapIndex;
                gameState->prevMapIndex = result.mapIndex;
                gameState->playerMapIndex = result.mapIndex;
                
                Vector2 pos = TilePositionToPixelPosition(result.map->width * 0.5f + result.map->tilePos.x + 0.5f, 
                                                          result.map->height * 0.5f + result.map->tilePos.y + 0.5f);
                cam.base.rotation = 0.0f;
                cam.base.target = pos;
                cam.base.zoom = GetCameraZoom(*result.map);
                cam.base.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
                
                break;
            }
            
            if (gameState->camera.tweenController.NoTweens())
            {
                FindTileMapResult result = FindTileMap(followEnt->tilePos);
                if (result.map) 
                {
                    UpdateCameraToTileMapSmooth(*result.map,
                                                            CAMERA_MOVE_FUNC, CAMERA_ZOOM_FUNC,
                                                            CAMERA_MOVE_SPEED, CAMERA_ZOOM_SPEED);
                }
            }
            
            
            break;
            }
        case MyCamera::FOLLOW_CENTER:
        {
            FindTileMapResult result = FindTileMap(followEnt->tilePos);
            if (cam.tweenController.NoTweens())
            {
                Vector2 center = followEnt->pivot +
                                            Vector2 { followEnt->tileSize.x * 0.5f, followEnt->tileSize.y * 0.5f };
                
                real32 dist = Vector2Distance(center, cam.base.target) / (real32)MAP_TILE_SIZE;
                
                TweenParams params = {};
                params.paramType = PARAM_TYPE_VECTOR2;
                params.startVec2 = cam.base.target;
                params.endVec2 = center;
                params.realVec2  = &cam.base.target;
                AddTweenUnique(cam.tweenController, CreateTween(params, 
                                                                CAMERA_MOVE_FUNC, 
                                                                               CAMERA_MOVE_SPEED * 5.0f,
                                                                               dist));
                
                SM_ASSERT(followEnt->type == ENTITY_TYPE_LOCK, "camera behaviour for lock only!");
                if (!followEnt->tweenController.NoTweens() && !followEnt->tweenController.playing)
                {
                TweenEvent playEvent;
                playEvent.controller = &followEnt->tweenController;
                cam.tweenController.endEvents.Add(playEvent);
                    // cam.base.target = Vector2Lerp(cam.base.target, center, 5 * GetFrameTime());
                }
                OnPlayEvent(&cam.tweenController);
            }
            break;
        }
        #if 0
        case MyCamera::FOLLOW_STAR:
        {
            
            if (!gameState->camera.tweenController.NoTweens())
            {
                Vector2 startPos = gameState->camera.base.target;
                real32 startZoom = gameState->camera.base.zoom;
                gameState->camera.tweenController.Reset();
                gameState->camera.base.target = startPos;
                gameState->camera.base.zoom = startZoom;
            }
            
            Entity * star = GetCollectingStar();
            
            if (star)
            {
                gameState->camera.base.target = Vector2Lerp(gameState->camera.base.target, star->pivot, 6 * GetFrameTime());
            }
            
            break;
        }
        #endif
        case MyCamera::FOLLOW_ALONG_AXIS:
        {
            
            if (gameState->prevMapIndex == gameState->currentMapIndex) break;
            
            
            Map * currentMap = &gameState->tileMaps[gameState->currentMapIndex];
            Vector2 finalPos = TilePositionToPixelPosition(currentMap->width * 0.5f + currentMap->tilePos.x + 0.5f, 
                                                           currentMap->height * 0.5f + currentMap->tilePos.y + 0.5f);
            
             real32 speed = 6;
            
            
            if (followEnt->tweenController.NoTweens())
            {
                
                if (cam.tweenController.NoTweens())
                {
                    
                    UpdateCameraToTileMapSmooth(*currentMap, 
                                                                                CAMERA_MOVE_FUNC, CAMERA_ZOOM_FUNC,
                                                                                CAMERA_MOVE_SPEED, CAMERA_ZOOM_SPEED);
                }
                break;
            }
            
            IVec2 moveDir = PivotToTilePos(GetTilePivot(followEnt), followEnt->tileSize) - PivotToTilePos(followEnt->pivot, followEnt->tileSize);
            
            Vector2 center = followEnt->pivot + followEnt->tileSize * 0.5f;
            Vector2 nextPos = cam.base.target;
            
            
            Map * prevMap = &gameState->tileMaps[gameState->prevMapIndex];
            Vector2 initPos = TilePositionToPixelPosition(prevMap->width * 0.5f + prevMap->tilePos.x + 0.5f, 
                                                           prevMap->height * 0.5f + prevMap->tilePos.y + 0.5f);
            
            bool8 moveX = Abs(moveDir.x) > 0 &&
            (Sign(center.x - cam.base.target.x) == Sign(moveDir.x));
            
            bool8 moveY = Abs(moveDir.y) > 0 && 
            (Sign(center.y - cam.base.target.y) == Sign(moveDir.y));
            
            
            if (moveX)
            {
                nextPos.y = Lerp(nextPos.y, initPos.y, speed * GetFrameTime());
                
                if (Sign(finalPos.x - nextPos.x) != Sign(moveDir.x))
                {
                    nextPos.x = Lerp(nextPos.x, finalPos.x, 0.1f * speed * GetFrameTime());
                }
                else
                {
                    nextPos.x = Lerp(nextPos.x, center.x, speed * GetFrameTime());
                }
            }
            else if (moveY)
            {
                nextPos.x = Lerp(nextPos.x, initPos.x, speed * GetFrameTime());
                
                if (Sign(finalPos.y - nextPos.y) != Sign(moveDir.y))
                {
                    nextPos.y = Lerp(nextPos.y, finalPos.y, 0.1f * speed * GetFrameTime());
                }
                else
                {
                    nextPos.y = Lerp(nextPos.y, center.y, speed * GetFrameTime());
                }
            }
            cam.base.target = nextPos;
            
            break;
        }
        }
    
    if (refocus)
    {
        if (GetPlayer())
        {
            FindTileMapResult result = FindTileMap(GetPlayer()->tilePos);
            if (result.map) UpdateCameraToTileMapSmooth(*result.map,
                                                        CAMERA_MOVE_FUNC, CAMERA_ZOOM_FUNC, 
                                                        CAMERA_MOVE_SPEED, CAMERA_ZOOM_SPEED);
            
            cam.followEntityIndex = GetPlayer()->entityIndex;
            
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


void SetUndoEntities(std::vector<Entity> & undoEntities)
{
    if (gameState->entities.count < (uint32)undoEntities.size())
        gameState->entities.count = (uint32)undoEntities.size();
    
    for (int32 i = 0; i < undoEntities.size(); i++)
    {
        Entity & e = undoEntities[i];
        
        if (e.type == ENTITY_TYPE_GLASS && e.broken)
        {
            SetGlassBeBroken(&e);
        }
        
        SM_ASSERT(e.entityIndex < gameState->entities.count, "out of index");
        if (e.entityIndex < gameState->entities.count)
        {
        gameState->entities[e.entityIndex] = e;
        gameState->entities[e.entityIndex].tweenController.Reset();
        gameState->entities[e.entityIndex].pivot = GetTilePivot(&e);
        
        if (IsSlime(&e) && (e.actionState == ANIMATE_STATE))
        {
            gameState->entities[e.entityIndex].actionState = MOVE_STATE;
            }
            }
    }
    }

#if 0
void SetUndoMapInfos(std::vector<UndoState::MapUndoInfo> & undoMapInfos)
{
    for (auto info : undoMapInfos)
    {
        gameState->tileMaps[info.mapIndex].stateInitilized = info.initilized;
    }
}
#endif

inline void SetGameState(UndoState & undoState)
{
    
    gameState->playerEntityIndex = undoState.playerIndex;
    gameState->camera.followEntityIndex = gameState->playerEntityIndex;
    gameState->starCount = (uint16)undoState.starCount;
    std::vector<Entity> & undoEntities = undoState.undoEntities;
    SetUndoEntities(undoEntities);
    
    // TODO : SetUndoMapInfos(undoState.undoMapInfos);
    }

inline void Undo()
{
    // ResetStars();
    if (!gameState->undoStack.IsEmpty())
    {
    SetGameState(gameState->undoStack.back());
        gameState->undoStack.pop_back();
    }
    }


inline void Restart()
{
    // ResetStars();
    DynamicArray<Entity> ea = GetCurrentStateEntities();
    gameState->undoStack.push_back(gameState->playerEntityIndex, 
                                   gameState->starCount, 
                                   ea);
    
    SetGameState(gameState->restartState);
    
#if 0
    Map & currentMap = gameState->tileMaps[gameState->playerMapIndex];
    if (currentMap.stateInitilized)
    {
    DynamicArray<Entity> ea = GetCurrentStateEntities();
    DynamicArray<UndoState::MapUndoInfo> mapUndoInfos = GetCurrentMapUndoInfos();
    
    gameState->undoStack.push_back(gameState->playerEntityIndex, 
                                   gameState->starCount, 
                                   ea, mapUndoInfos);
        SetGameState(currentMap.resetState);
    }
    #endif
    }

bool8 MoveAction(IVec2 actionDir)
{
    Entity * player = GetEntity(gameState->playerEntityIndex);
    SM_ASSERT(player, "player is not active");
    
    if (!player->attach || player->attachDir == actionDir)
    {
        return false;
    }
    
    
    real32 moveSpeed = 4.0f;
    IVec2 currentPos = player->tilePos;
    IVec2 actionTilePos = currentPos + actionDir;
    
    EntityLayer doorLayer[] = { LAYER_DOOR };
        Entity * door = FindEntityByLocationAndLayers(currentPos, doorLayer, ArrayCount(doorLayer));
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
        
        MoveEntity(player, door, nullptr, player->tilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
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
                    if (!IsDoor(GetEntity(player->attachedEntityIndex))) 
                    {
                        PushResult reversePushResult = ActionCheck(player, player->attachDir, CHECK_MOVE);
                        if (reversePushResult.state == PUSH_BLOCKED)
                        {
                            StretchEntity(player, actionDir, player->attachDir, player->attachDir,
                                          player->tilePos, player->tilePos, 0.8f, PLAYER_MOVE_FUNC, GetStretchSpeed(player));
                            OnPlayEvent(&player->tweenController);
                        }
                    }
                    SetSlimeSprite(player, actionDir);
                    
                return true;
                }
                return false;
            }
            
            IVec2 actionTilePos = player->tilePos + actionDir;
            // NOTE: no obsticale, move player
            IVec2 standingPlatformPos = actionTilePos + player->attachDir;
            
            
            if (Entity * door = FindEntityByLocationAndLayers(actionTilePos, doorLayer, ArrayCount(doorLayer));
                door && DoorBlocked(door, -player->attachDir))
            {
                MoveEntity(player, door, nullptr, door->tilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
                }
            else if (FindAttachableResult findResult = FindAttachable(standingPlatformPos, player->attachDir);
                findResult.has)
            {
                Entity * resultEntity = findResult.entity;
                if (resultEntity->type == ENTITY_TYPE_DOOR &&
                    !SameSide(resultEntity, standingPlatformPos, player->attachDir))
                {
                    return false;
                }
                else if (resultEntity->type == ENTITY_TYPE_BRIDGE &&
                         !BridgeBlocked(resultEntity, player->attachDir))
                {
                    return false;
                }
                
                SetSlimeSprite(player, actionDir);
                MoveEntity(player, findResult.entity, nullptr, actionTilePos, PLAYER_MOVE_FUNC, MOVE_SPEED);
            }
            else if (Entity * door = FindEntityByLocationAndLayers(standingPlatformPos, doorLayer, 1);
                     door && DoorBlocked(door, actionDir))
            {
                return false;
            }
            else
            {
                EntityLayer layers[] = { LAYER_SLIME };
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
                    
                    if (attachedEntity && attachedEntity->type == ENTITY_TYPE_DOOR &&
                        !SameSide(attachedEntity, newTile, newAttach))
                    {
                        return false;
                    }
                    else if (attachedEntity->type == ENTITY_TYPE_BRIDGE && !BridgeBlocked(attachedEntity, newAttach))
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
                pushResult.blockedEntity->type == ENTITY_TYPE_DOOR &&
                !SameSide(pushResult.blockedEntity, player->tilePos + actionDir, actionDir);
            
            bool8 blockedByBridgeWrongSide = 
                pushResult.blockedEntity->type == ENTITY_TYPE_BRIDGE &&
                !BridgeBlocked(pushResult.blockedEntity, actionDir);
            
            if (blockedByPit || blockedByDoor || blockedByBridgeWrongSide)
            {
                return false;
            }
            
            if (!IsDoor(GetEntity(player->attachedEntityIndex)) && player->attachDir == -actionDir)
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
            // if (!door)
            {
                MergeSlimes(pushResult.mergeEntity, player);
                return true;
            }
            // return false;
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
    // player->attach = false;
    player->pivot = GetTilePivot(player);
    
    bool8 playerProjectable = true;
    bool8 cloneProjectable = true;
    
    EntityLayer layers[] = { LAYER_DOOR };
    Entity * door = FindEntityByLocationAndLayers(player->tilePos, layers, ArrayCount(layers));
    if (door)
    {
        if (DoorBlocked(door, -bounceDir))
        {
            playerProjectable = false;
        }
        else if (DoorBlocked(door, bounceDir))
        {
            cloneProjectable = false;
        }
    }
    
    if (playerProjectable) ActionCheck(player, bounceDir, CHECK_PROJECT);
    if (cloneProjectable) ActionCheck(clone, -bounceDir, CHECK_PROJECT);
    
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
            if (entity && entity->isVisible)
            {
                Color color = entity->color;
                if (entity->actionState == FREEZE_STATE)
                {
                    color = LIME;
                }
                
                if (entity->type == ENTITY_TYPE_GLASS)
                {
                    color = ColorAlpha(color, 0.6f);
                }
                
                if (entity->type == ENTITY_TYPE_SLIME_PORTAL)
                {
                    entity->tileSize = GetSlimeSize(GetPlayer());
                    entity->pivot = GetTilePivot(entity->tilePos, entity->tileSize, entity->attachDir);
                }
                
                Texture2D drawTexture = gameState->textureAltas;
                Sprite drawSprite = entity->sprite;
                Vector2 drawPivot = entity->pivot;
                
                if (IsSlime(entity))
                {
                BeginShaderMode(gameState->movableShader.shader);
                
                DrawSprite(gameState->camera.base, drawTexture, drawSprite, drawPivot, entity->tileSize, color);
                EndShaderMode();
                }
                else if (entity->type == ENTITY_TYPE_TUT_PORTAL)
                {
                    BeginShaderMode(gameState->portalShader.shader);
                    DrawSprite(gameState->camera.base, drawTexture, drawSprite, drawPivot, entity->tileSize, color);
                    EndShaderMode();
                }
                else
                {
                    DrawSprite(gameState->camera.base, drawTexture, drawSprite, drawPivot, entity->tileSize, color);
                    
                }
                
                if (layer == LAYER_NULL)
                {
                    DrawTile(entity->tilePos, RED);
                }
                
#if 0
                // NOTE: Draw Debug Informations
                else if (entity->type == ENTITY_TYPE_BLOCK)
                {
                    if (CheckProjectState(entity))
                    {
                        color = ColorTint(color, GRAY);
                    }
                }
                
                if (!IsSlime(entity) && FindAttachSlime(entity))
                {
                    color = BLUE;
                }
                
                if (IsSlime(entity))
                {
                    
                    DrawCircleV(entity->pivot + Vector2 {entity->tileSize/2, entity->tileSize/2}, 
                                3, ColorAlpha(YELLOW, 0.8f));
                    DrawTile(PivotToTilePos(entity->pivot, entity->tileSize), ColorAlpha(RED, 0.5f));
                    }

                if (IsSlime(entity))
                {
                    real32 halfSize = entity->tileSize/2;
                    Vector2 center = entity->pivot + Vector2 {halfSize, halfSize};
                    Vector2 attachPos = center + { halfSize * entity->attachDir.x, halfSize * entity->attachDir.y };
                    
                    IVec2 parallel = { entity->attachDir.y, entity->attachDir.x };
                    
                    Vector2 A = attachPos + { parallel.x * halfSize, parallel.y * halfSize };
                    Vector2 B = attachPos - { parallel.x * halfSize, parallel.y * halfSize };
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

void SimulateInputs(DynamicArray<Entity> & prevEntState, uint32 & prevPlayerIndex)
{
    
    if (UpdateElectricDoor()) 
    {
        return;
    }
    
    Entity * player = GetEntity(gameState->playerEntityIndex);
    
    if (Entity * followEnt = GetEntity(gameState->camera.followEntityIndex); followEnt != player && gameState->currentMapIndex != gameState->playerMapIndex)
    {
        if(IsActionKeyDown(gameState->input.keyMappings))
        {
            gameState->refocus = true;
        }
        return;
    }
    
    if (JustPressed(gameState->input.keyMappings, ZOOM_KEY))
    {
        gameState->zoomOut = !gameState->zoomOut;
        return;
    }
    
    
    if (!player) return;
    
    IVec2 dirs[] = { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };
    
    if (gameState->zoomOut)
    {
         real32 sceneDistance = 20.0f * MAP_TILE_SIZE;
        
        real32 minX = player->pivot.x - sceneDistance;
        real32 minY = player->pivot.y - sceneDistance;
        real32 maxX = player->pivot.x + sceneDistance;
        real32 maxY = player->pivot.y + sceneDistance;
        
        
        Rectangle moveRect = 
        {
            player->pivot.x - sceneDistance, player->pivot.y - sceneDistance,
            sceneDistance * 2, sceneDistance * 2
        };
        
        
        Vector2 movement = Vector2 { 0, 0 };
        
        real32 camSpeed = 300.0f;
        
        if (IsDown(gameState->input.keyMappings, LEFT_KEY))
        {
            movement -= Vector2 { camSpeed, 0 };
        }
        if (IsDown(gameState->input.keyMappings, RIGHT_KEY))
        {
            movement += Vector2 { camSpeed, 0 };
            
        }
        if (IsDown(gameState->input.keyMappings, UP_KEY))
        {
            movement -= Vector2 { 0, camSpeed };
        }
        if (IsDown(gameState->input.keyMappings, DOWN_KEY))
        {
            movement += Vector2 { 0, camSpeed };
        }
        
        Vector2 dest = gameState->camera.base.target + movement;
        
        #if 0
        if (dest.x > maxX) dest.x = maxX;
        if (dest.x < minX) dest.x = minX;
        if (dest.y > maxY) dest.y = maxY;
        if (dest.y < minY) dest.y = minY;
#endif
        
            gameState->camera.base.target = 
                Vector2Lerp(gameState->camera.base.target, dest, GetFrameTime());
        }
    else
        {
        
        if (IsActionKeyDown(gameState->input.keyMappings))
        {
            prevEntState = GetCurrentStateEntities();
            }
        
        if (!gameState->camera.tweenController.NoTweens())
        {
            return;
        }
        // NOTE: read input
        if (gameState->canSwitchSlime && 
            JustPressed(gameState->input.keyMappings, POSSES_KEY))
        {
    // NOTE SlimeSelection
            gameState->stateChanged = SelectNextAsPlayer(player);
            gameState->slimeSwitched = gameState->stateChanged;
        }
        
        switch(player->actionState)
        {
            case MOVE_STATE:
            {
                
                IVec2 actionDir = { 0 };
                if (gameState->canSplitSlime && JustPressed(gameState->input.keyMappings, SPLIT_KEY))
                {
                    gameState->isPressed = true;
                    actionDir= -player->attachDir;
                    gameState->stateChanged = 
                        gameState->stateChanged || SplitAction(player, actionDir);
                }
                else
                {
                    
                    GameInputType downedKey = GetDownedMoveKey(gameState->input.keyMappings);
                    
                    if (IsDown(gameState->input.keyMappings, gameState->lastMoveKey))
                    {
                        downedKey = gameState->lastMoveKey;
                    }
                    
                    if (downedKey != NO_INPUT)
                    {
                        uint32 index = downedKey - LEFT_KEY;
                        actionDir = dirs[index];
                        gameState->isPressed = true;
                    }
                    else if (gameState->moveBufferTimer > 0)
                    {
                        uint32 index = gameState->lastMoveKey - LEFT_KEY;
                        actionDir = dirs[index];
                        gameState->isPressed = true;
                    }
                    
                    if (gameState->isPressed)
                    {
                        bool8 moved = MoveAction(actionDir);
                        if (!moved && actionDir != player->attachDir)
                        {
                            
                            Entity * attachEntity = GetEntity(player->attachedEntityIndex);
                            
                            SetSlimeSprite(player, actionDir);
                            
                            StretchEntity(player, actionDir, player->attachDir, player->attachDir,
                                          player->tilePos, player->tilePos, 0.8f, PLAYER_MOVE_FUNC, GetStretchSpeed(player), false, attachEntity, attachEntity);
                            OnPlayEvent(&player->tweenController);
                            
                            gameState->aniSpeedAdjustable = true;
                            
                        }
                        else if (moved)
                        {
                            // TODO  saparate move sound and move by pushed sound
                            // NOTE: I might be more accurate to play sound when an tween event occur rather than logic
                            // NOTE: Add a play sound event might  be a good idea
                            PlayClip(&gameState->audioData, PLAYER_MOVE_SOUND, .2f);
                        }
                        gameState->stateChanged = gameState->stateChanged || moved;
                    }
                }
                if (gameState->stateChanged) 
                {
                    gameState->aniSpeedAdjustable = false;
                    if (player->spriteType == SPRITE_TYPE_SPRITE)
                    {
                        SetSlimeSprite(player, actionDir);
                    }
                    else if (player->spriteType == SPRITE_TYPE_ANIMATED)
                    {
                        SetSlimeAnimatedSprite(player, actionDir);
                    }
                }
                
                break;
            }
        }
        
        if (gameState->stateChanged)
        {
            gameState->undoStack.push_back(prevPlayerIndex,
                                           gameState->starCount,
                                           prevEntState);
        }
        
    }
    }

void GameplayUpdateAndRender()
{
    bool8 noPlayer = false;
    if (!gameState->switching)
    {
    // NOTE: Recored if State Changes
        gameState->stateChanged = false;
        gameState->isPressed = false;
        gameState->slimeSwitched = false;
        gameState->refocus = false;
        
        GameInputType lastMoveKey = GetPressedMoveKey(gameState->input.keyMappings);
        
        if (lastMoveKey != NO_INPUT)
        {
            gameState->lastMoveKey = lastMoveKey;
            gameState->moveBufferTimer = MOVE_BUFFER;
            SM_TRACE("Reset BUFFER");
            
            if (Entity * player = GetPlayer(); gameState->simulating && player && gameState->aniSpeedAdjustable)
            {
                gameState->aniSpeedAdjustable = false;
                player->tweenController.AdjustSpeed(2.0f);
            }
        }
        
    // NOTE: Actions
        if (GetPlayer() && !gameState->simulating) 
        {
            DynamicArray<Entity> prevEntState = { 0 };
            uint32 prevPlayerIndex = gameState->playerEntityIndex;
            
            SimulateInputs(prevEntState, prevPlayerIndex);
            // NOTE: Stars
            {
                auto & slimeIndexTable = gameState->entityTable[LAYER_SLIME];
                for (uint32 slimeIndex = 0; slimeIndex < slimeIndexTable.count; slimeIndex++)
                {
                    Entity * slime = GetEntity(slimeIndexTable[slimeIndex]);
                    if (slime)
                    {
                        auto & starTable = gameState->entityTable[LAYER_STAR];
                        for (uint32 starIndex = 0; starIndex < starTable.count; starIndex++)
                        {
                            Entity * star = GetEntity(starTable[starIndex]);
                            if (star && !star->starCollecting &&  (star->tilePos == slime->tilePos))
                            {
                                // ResetResetStates();
                                // DeleteEntity(star);
                                star->starCollecting = true;
                                star->collectStart = star->pivot;
                                star->collectEnd = GetStarDestPosition();
                                gameState->starCount++;
                                
                                SM_TRACE("reset player pos: (%d, %d)", GetPlayer()->tilePos.x, GetPlayer()->tilePos.y);
                                // NOTE: when star collected, clear undo stack and update reset state
                                DynamicArray<Entity> ea = GetCurrentStateEntities();
                                
                                gameState->undoStack.reset();
                                gameState->undoStack.push_back(gameState->playerEntityIndex, 
                                                               gameState->starCount,
                                                               ea);
                                InitUndoState(&gameState->restartState, gameState->playerEntityIndex, 
                                              gameState->starCount, ea);
                                break;
                            } // if
                        } // for 
                    } // if
                } // for
            } // star
        }

    static uint32 change_count = 0;
        if (gameState->stateChanged) change_count++;
    
    SetFreeze();
        UpdateSlimes();
        UpdateStars();
        
            gameState->moveBufferTimer -= GetFrameTime();
            
        // NOTE: Undo and Restart
        if (!gameState->zoomOut) 
        {
            static bool8 repeat = false;
            static real32 timeSinceLastPress = 0;
            
            timeSinceLastPress -= GetFrameTime();
            
        if (timeSinceLastPress < 0 && IsDown(gameState->input.keyMappings, UNDO_KEY) && !gameState->undoStack.IsEmpty())
            {
                // NOTE: Undo
                Undo();
                timeSinceLastPress = press_freq;
                repeat = false;
            }
        
            repeat = repeat && !gameState->stateChanged;
            if (JustPressed(gameState->input.keyMappings, RESET_KEY) && !repeat)
            {
                repeat = true;
                Restart();
        }
        }
        
            #if 0
        auto & lockTable = gameState->entityTable[LAYER_LOCK];
        for (uint32 lockIndex = 0; lockIndex < lockTable.count; lockIndex++)
        {
            Entity * lock = GetEntity(lockTable[lockIndex]);
            if (lock && !lock->open && (lock->unlockCount <= gameState->starCount))
            {
                SM_ASSERT(lock->unlockCount == gameState->starCount, "this lock should be unlocked earlier");
                lock->open = true;
                real32 delayTime = 1.0f;
                TweenParams params = { 0 };
                params.paramType = PARAM_TYPE_COLOR;
                params.startColor = WHITE;
                params.endColor = ColorAlpha(WHITE, 0.0f);
                params.realColor = &lock->color;
                AddTweenUnique(lock->tweenController, CreateTween(params, nullptr, 1.0f, delayTime));
                TweenEvent deleteEvent2;
                deleteEvent2.deleteEntity = lock;
                lock->tweenController.endEvents.Add(deleteEvent2);
                // OnPlayEvent(&lock->tweenController);
                
                 DynamicArray<int32> da = FindAllMapsWithStarCount(gameState->starCount);
                uint32 unlockedMapCount = da.count;
                int32 * unlockedMaps = da.elements;
                
                // NOTE: sort the map by their ids?
                
            }
        }
            
        
#endif
        
        
    
        // NOTE: Simulate
        Entity * lastFollowEnt = gameState->simulating ? nullptr : GetEntity(gameState->camera.followEntityIndex);
    {
        gameState->simulating = false;
        // NOTE: Update: Entity
        EntityLayer simulateLayers[] = { LAYER_SLIME, LAYER_BLOCK };
        for (uint32 idx = 0; idx < ArrayCount(simulateLayers); idx++)
        {
            uint32 layer = simulateLayers[idx];
            auto & entList = gameState->entityTable[layer];
            for (uint32 entIdx = 0; entIdx < entList.count; entIdx++)
            {
                Entity * entity = GetEntity(entList[entIdx]);
            if (entity)
                {
                    // NOTE: AnimatedSprite Updates
                    if (entity->spriteType == SPRITE_TYPE_ANIMATED)
                    {
                        PlayAnimatedSprite(&entity->animatedSprite);
                    }
                    
                    // NOTE: Tween updates
                if (!entity->tweenController.NoTweens())
                {
                    if (IsSlime(entity))
                    {
                        if (entity->actionState == MOVE_STATE) SetActionState(entity, ANIMATE_STATE);
                    }
                            entity->tweenController.Update();
                            
                            RevealEntity(entity, ENTITY_TILE_VISIBILITY);
                            
                        if (!IsDisappearing(entity))
                        {
                        gameState->simulating = true;
                        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
                        if (entity->tweenController.playing)
                        {
                            if (!followEnt)
                            {
                                gameState->camera.followEntityIndex = entity->entityIndex;
                            }
                            else if (IsSlime(followEnt))
                            {
                                if (FindAttachSlime(entity) == followEnt)
                            {
                                    gameState->camera.followEntityIndex = entity->entityIndex;
                                }
                                else if ((entity->tilePos - followEnt->tilePos).SqrMagnitude() > 1)
                                {
                                    gameState->camera.followEntityIndex = entity->entityIndex;
                                }
                                
                            }
                            else if (followEnt->tweenController.NoTweens())
                            {
                                gameState->camera.followEntityIndex = entity->entityIndex;
                            }
                        }
                        else if (layer == LAYER_LOCK)
                        {
                            gameState->camera.followEntityIndex = entity->entityIndex;
                            }
                        }
                        
                        if (layer == LAYER_BLOCK)
                        {
                            IVec2 pos = PivotToTilePos(entity->pivot, entity->tileSize);
                            EntityLayer glassLayer[] = { LAYER_GLASS };
                            Entity * glass = FindEntityByLocationAndLayers(pos, glassLayer, 1);
                                if (glass && SetGlassBeBroken(glass))
                            {
                                    SetShake(0.05f, 0.001f);
                                    }
                        }
                        
                        }
                else
                {
                    if (entity->actionState == ANIMATE_STATE) SetActionState(entity, MOVE_STATE);
                }
            } 
        }
        }
        }
    
    Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
    if (!followEnt || 
            (gameState->isPressed && (followEnt != GetPlayer()) && followEnt->tweenController.NoTweens()))
    {
        gameState->camera.followEntityIndex = gameState->playerEntityIndex;
        }
    
    // NOTE: update mapIndex
    {
        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
        if (followEnt)
        {
        Entity * prevFollowEnt = gameState->undoStack.back().GetByEntityIndex(followEnt->entityIndex);
        
        gameState->prevMapIndex = -1;
        
        if (prevFollowEnt && prevFollowEnt->active)
        {
        FindTileMapResult prevResult = FindTileMap(prevFollowEnt->tilePos);
        if (prevResult.map)
        {
            gameState->prevMapIndex = prevResult.mapIndex;
        }
            }
            
        FindTileMapResult result = FindTileMap(followEnt->tilePos);
            if (result.map && MapIsVisible(*result.map))
        {
                gameState->currentMapIndex = result.mapIndex;
                }
            
            if (gameState->prevMapIndex < 0)
            {
                gameState->prevMapIndex = gameState->currentMapIndex;
            }
            
            }
            
            if (Entity * player = GetPlayer(); 
                player && player->tweenController.NoTweens()) 
            {
                FindTileMapResult playerMap = FindTileMap(player->tilePos);
                if (playerMap.map && MapIsVisible(*playerMap.map))
                {
                    gameState->playerMapIndex = playerMap.mapIndex;
                RevealMap(gameState->tileMaps[gameState->playerMapIndex]);
                }
            }
            }
    
    
    // NOTE: Camera Updates
    {
        Vector2 oldTarget = gameState->camera.base.target;
            
            #if GAME_INTERNAL
        bool8 freeForm = IsKeyDown(KEY_T);
        // NOTE: Debug Camera Control
            if (freeForm)
            {
        // NOTE: CameraZoom
        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        real32 wheelDelta = (real32)GetMouseWheelMove();
        
        gameState->camera.base.zoom = expf(logf(gameState->camera.base.zoom) + (wheelDelta*0.1f));
        // NOTE: Camera Drag
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
            {
                Vector2 mouseDelta = GetMouseDelta();
            gameState->camera.base.target.x -= mouseDelta.x;
            gameState->camera.base.target.y -= mouseDelta.y;
        }
        //if (gameState->camera.base.zoom > 10.0f) gameState->camera.base.zoom = 10.0f;
        if (gameState->camera.base.zoom < 0.1f) gameState->camera.base.zoom = 0.1f;
        }
        if (!freeForm)
            #endif
        {
                if (gameState->slimeSwitched)//  || ((followEnt != lastFollowEnt) && (GetPlayer() == followEnt)))
                {
                    gameState->refocus = true;
    }
        UpdateCamera(gameState->refocus);
    }
        
    Vector2 newTarget = gameState->camera.base.target;
    gameState->camera.moveDir = { 0 };
    if (!Vector2Equals(oldTarget, newTarget))
    {
                gameState->camera.moveDir = (oldTarget - newTarget) * 0.1f * gameState->camera.base.zoom;
    }
    }
    
    if (!GetPlayer() && !SelectNextAsPlayer())
    {
        noPlayer = true;
    }
        
        // NOTE: UpdateFog
        UpdateFog();
    
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
                    gameState->canSwitchSlime = true;
                    gameState->canSplitSlime = true;
            gameState->lastState = gameState->undoStack.back();
            
            for (uint32 idx = 0; idx < gameState->lastState.undoEntities.size(); idx ++)
            {
                Entity * e = &gameState->lastState.undoEntities[idx];
                if (portal->entityIndex == e->entityIndex)
                {
                    portal = e;
                }
                }
                    
                    portal->sprite = GetBlockSprite(portal->mass);
                    
            portal->type = ENTITY_TYPE_BLOCK;
            portal->color = WHITE;
                gameState->lastTutBlockIndex = portal->entityIndex;
                ChangeScreen(GAME_TUT_SCREEN);
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
        
        if (IsKeyPressed(KEY_DELETE))
        {
            end = true;
        }
        
        if (end)
        {
            
            ChangeScreen(GAME_MAIN_SCREEN);
            return;
            }
        
    }
}
    
    
    static real32 contrast = -2.0f;
    static real32 saturation = 4.5f;
    static real32 brightness = 1.1f;
    static bool8 debugView = false;
    static bool8 showFog = true;
    
    #if GAME_INTERNAL
    // NOTE: Developer Debug Cheats
    {
        
        if (IsKeyPressed(KEY_EQUAL))
        {
            SetShake(0.05f, 0.001f);
            gameState->starCount++;
        }
        
        
        if (IsKeyPressed(KEY_NINE))
        {
            UnloadTexture(gameState->bgTexture);
            UnloadTexture(gameState->textureAltas);    // Unload render texture
            gameState->textureAltas = LoadTexture(TEXTURE_PATH);
            gameState->bgTexture = LoadTexture(BACKGROUND_PATH);
            }
        
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            debugView = !debugView;
        }
        
        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            showFog = !showFog;
        }
        
    }
    
    #if 0
    EntityLayer layers[] =  { LAYER_DOOR };
    Entity * block = FindEntityByLocationAndLayers(IVec2 { 69, 130 }, layers, 1);
    if (!block)
    {
        Entity addEntity = {};
        addEntity.type = ENTITY_TYPE_DOOR;
        addEntity.down = true;
        addEntity.up = true;
        addEntity.tilePos = IVec2 { 69,130 };
        addEntity.sprite = GetSprite(84);
        addEntity.active = true;
        addEntity.tileSize = DEFAULT_TILE_SIZE;
        addEntity.pivot = GetTilePivot(addEntity.tilePos, DEFAULT_TILE_SIZE);
        addEntity.color = WHITE;
        uint32 entityIndex = gameState->entities.Add(addEntity);
        GetEntity(entityIndex)->entityIndex = entityIndex;
        gameState->entityTable[LAYER_DOOR].Add(entityIndex);
    }
#endif
    
    #endif
    // NOTE: Render
    {
        
Fog & fog = gameState->fog;
        IVec2 fogDim = fog.tileMax - fog.tileMin;
        
        // NOTE: Draw Fog of War
        SM_ASSERT(IsRenderTextureValid(fog.fogRenderTex), "render texture not valid");
        SM_ASSERT(fog.fogRenderTex.texture.width == fogDim.x, "texture width not valid");
        SM_ASSERT(fog.fogRenderTex.texture.height == fogDim.y, "texture height not valid");
        
        if (gameState->currentScreen == GAME_MAIN_SCREEN)
        {
            UpdateTexture(fog.fogRenderTex.texture, fog.fogPixels);
        }
        
        
        UpdateShaderInfo(gameState->movableShader);
        UpdateShaderInfo(gameState->postShader);
        UpdateShaderInfo(gameState->portalShader);
        
        BeginTextureMode(gameState->renderTarget);
        ClearBackground(gameState->bgColor);
        
        int mn = Min(GetScreenWidth(), GetScreenHeight());
        UpdateAndDrawStarFieldBG(&gameState->starFields, 
                                 (GetScreenWidth() - mn) / 2,
                                 (GetScreenHeight() - mn) / 2,
                                 gameState->camera.moveDir);
        
        BeginMode2D(gameState->camera.base);
        SetDrawingEntities();
        
        EntityLayer orderedDrawLayers[] = 
        {
            LAYER_LINK,
            LAYER_SOURCE,
            LAYER_CABLE,
            LAYER_CONNECTION,
            LAYER_WALL, 
            LAYER_PIT,
            LAYER_PORTAL,
            LAYER_SLIME,
            LAYER_BLOCK,
            LAYER_GLASS,  
            LAYER_DOOR,
            LAYER_LOCK,
            LAYER_UI,
        };
        
        int32 count = ArrayCount(orderedDrawLayers);
        DrawSpriteLayers(orderedDrawLayers, count);
        Entity * followEnt = GetEntity(gameState->camera.followEntityIndex);
        
        if (debugView)
        {
        Color colors[] = {
            LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE,
        };
        
        int32 colorCount = ArrayCount(colors);
        
        static Color * mColors = (Color *)BumpAllocArray(gameMemory->persistentStorage,
                                                         gameState->tileMapCount,
                                                         sizeof(Color));
        
        static bool * set = (bool *)BumpAllocArray(gameMemory->persistentStorage,
                                                   gameState->tileMapCount,
                                                   sizeof(bool));
        
        for (uint32 mapIndex = 0; mapIndex < gameState->tileMapCount; mapIndex++)
        {
            Map * tileMap = gameState->tileMaps + mapIndex;
            
            if (!set[mapIndex])
            {
                set[mapIndex] = true;
                int32 colorIndex = GetRandomValue(0, colorCount-1);
                mColors[mapIndex] = colors[colorIndex];
            }
                
                // NOTE: DrawCenterTile
                IVec2 mapCenter = tileMap->tilePos + 
                    IVec2 { tileMap->width / 2 + 1, tileMap->height / 2 + 1 };
                
                DrawTile(mapCenter, WHITE);
            
            // NOTE: Could be a setting
            DrawTileMap(gameState->camera.base, tileMap->tilePos, 
                        IVec2{ tileMap->width, tileMap->height },
                        mColors[mapIndex], mColors[mapIndex]);
            Map & map = gameState->tileMaps[mapIndex];
            Rectangle mapRect = 
            { 
                (real32)map.tilePos.x * MAP_TILE_SIZE, (real32)map.tilePos.y * MAP_TILE_SIZE, 
                (real32)map.width * MAP_TILE_SIZE,  (real32)map.height * MAP_TILE_SIZE
            };
            
            DrawRectangleLinesEx(mapRect, 5, BLUE);
                }
            
            
            // NOTE: draw mouse tilePos
            Vector2 mousePos = GetMousePosition();
            Vector2 mouseWorld = GetScreenToWorld2D(mousePos, gameState->camera.base);
            
            IVec2 tilePos = PivotToTilePos(mouseWorld, Vector2{ 0 });
            DrawTile(tilePos, RED);
            DrawLineBezier(GetPlayer()->pivot, GetTilePivot(tilePos, DEFAULT_TILE_SIZE), 2, GREEN);
            
            Vector2 textWorld = GetScreenToWorld2D({ 1200.0f, 200.0f }, gameState->camera.base);
            DrawText(TextFormat("Mouse TilePos (%d, %d)", tilePos.x, tilePos.y), (int)textWorld.x, (int)textWorld.y, 10, GREEN);
            
            // Draw rectangle outline with extended parameters
            Rectangle cameraRect = GetCameraRect(gameState->camera.base);
            DrawRectangleLinesEx(cameraRect, 1, RED);
            
            if (followEnt)
            {
                Vector2 center = followEnt->pivot + Vector2 { followEnt->tileSize.x * 0.5f, followEnt->tileSize.y * 0.5f };
                DrawCircleV(center, 5,  RED);
                DrawCircleV(gameState->camera.base.target, 5, YELLOW);
            }
            
        }
        
        if (showFog && gameState->currentScreen == GAME_MAIN_SCREEN)
        {
        Vector2 pivot = GetTilePivot(fog.tileMin, DEFAULT_TILE_SIZE);
        DrawTexturePro(fog.fogRenderTex.texture, 
                       Rectangle { 0, 0, (real32)fog.fogRenderTex.texture.width, (real32)fog.fogRenderTex.texture.height },
                       Rectangle {
                           pivot.x,
                           pivot.y, 
                           (real32)MAP_TILE_SIZE * fog.dim.x,
                           (real32)MAP_TILE_SIZE * fog.dim.y
                       },
                       Vector2 { 0, 0 }, 0, WHITE);
        }
        
        EntityLayer layers[] = 
        {
            LAYER_STAR,
            LAYER_STAR_DEST,
            };
        DrawSpriteLayers(layers, ArrayCount(layers));
        
        EndMode2D();
        EndTextureMode();
        // NOTE: Draw
        
        BeginDrawing();
        ClearBackground(IntToRGBA(0x465a6f));
        
        DrawScrollingBackGround(gameState->bgTexture, BLUE, 2);
        
        gameState->time = (real32)GetTime();
        gameState->shakeTime -= GetFrameTime();
        if (gameState->shakeTime < 0)
        {
            gameState->shake = false;
            }
        
        {
            int32 contrastLoc =
                GetShaderLocation(gameState->postShader.shader, "contrast");
            int32 saturationLoc =
                GetShaderLocation(gameState->postShader.shader, "saturation");
            int32 brightnessLoc = 
                GetShaderLocation(gameState->postShader.shader, "brightness");
            
            
            SetShaderValue(gameState->postShader.shader, contrastLoc, &contrast, SHADER_UNIFORM_FLOAT);
            
            SetShaderValue(gameState->postShader.shader, saturationLoc, &saturation, SHADER_UNIFORM_FLOAT);
            
            SetShaderValue(gameState->postShader.shader, brightnessLoc, &brightness, SHADER_UNIFORM_FLOAT);
            
            PostProcessing(gameState->renderTarget, gameState->postShader, 
                                      gameState->shake, gameState->shakeStrength, gameState->time);
            }
        
        
        if (noPlayer)
        {
            DrawError();
        }
#if  GAME_INTERNAL
        if (debugView)
        {
        // NOTE: UI Draw Game Informations
        Entity * player = GetEntity(gameState->playerEntityIndex);
        if (player)
        {
        IVec2 centerPos = player->tilePos;
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera.base);
        }
        
        
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f\nCamera State: %s",
                            gameState->camera.base.target.x, gameState->camera.base.target.y,
                            gameState->camera.base.offset.x, gameState->camera.base.offset.y, gameState->camera.base.zoom,
                            GetCameraState(gameState->camera)), 10, 50, 20, RAYWHITE);
        
        if (followEnt)
        {
        DrawText(TextFormat("Follow Entity Type: %s(%d)", GetEntityType(followEnt), followEnt->entityIndex),
                 10, 200, 25, YELLOW);
        }
        
        real32 vert = 445;
        real32 gap = 100;
        GuiSliderBar(Rectangle{ 100, vert, 400, 30 }, "Contrast", TextFormat("%.1f", contrast), &contrast, -100.0f, 100.0f);
        GuiSliderBar(Rectangle{ 100, vert + gap, 400, 30 }, "Saturation", TextFormat("%.1f", saturation), &saturation, -100.0f, 100.0f);
        GuiSliderBar(Rectangle{ 100, vert + gap * 2, 400, 30 }, "Brightness", TextFormat("%.1f", brightness), &brightness, 0, 10.0f);
        
        DrawText(TextFormat("Stars Collected: %d",
                                     gameState->starCount),
                 10, 250, 25, ORANGE);
        
        
        
        if (player)
        {
            DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: (%.2f, %.2f),  Entity Count: %i",
                            player->tilePos.x, player->tilePos.y,
                            player->mass, player->tileSize.x , player->tileSize.y,  gameState->entities.count), 10, 140, 20, GREEN);
        }
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
        DrawText(TextFormat("Move buffer timer: %.2fs", gameState->moveBufferTimer), 10, 350, 20, GREEN);
        }    
}

void InitializeGame()
{
    SetRandomSeed((uint32)GetTime());
    // NOTE: Initialization
    gameState->initialized = true;
    IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} }; 
    
    // NOTE: Initilize silme portal
    {
        auto & portals = gameState->entityTable[LAYER_PORTAL];
        for (uint32 i = 0; i < portals.count; i++)
        {
            if (Entity * portal = GetEntity(portals[i]); 
                portal && portal->type == ENTITY_TYPE_SLIME_PORTAL)
            {
                for (int32 j = 0; j < 4; j++)
                {
                    IVec2 dir = directions[j];
                    
                    EntityLayer layers[] = { LAYER_PORTAL };
                    Entity * attach = FindEntityByLocationAndLayers(portal->tilePos + dir, layers, 1);
                    
                    if (attach && attach->type == ENTITY_TYPE_MAIN_PORTAL)
                    {
                        portal->attach = true;
                        portal->attachDir = dir;
                        break;
                        }
                    
                    }
                }
            
        }
    }
    
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
            
            for (int32 j = 0; j < 4; j++)
            {
                if (!slimeA->attach && AttachSlime(slimeA, directions[j])) break;
            }
            
            slimeA->pivot = GetTilePivot(slimeA);
            RevealEntity(slimeA, ENTITY_TILE_VISIBILITY);
            }
        
        if (slimeB)
        {
            for (int32 j = 0; j < 4; j++)
            {
                if (!slimeB->attach && AttachSlime(slimeB, directions[j])) break;
            }
            
            slimeB->pivot = GetTilePivot(slimeB);
            RevealEntity(slimeB, ENTITY_TILE_VISIBILITY);
        }
        
        if (slimeA && slimeB && slimeA->tilePos == slimeB->tilePos) 
        {
            slimeA->color = WHITE;
            slimeA->mass++;
            slimeA->tileSize = GetSlimeSize(slimeA);
            slimeA->pivot = GetTilePivot(slimeA);
            gameState->playerEntityIndex = slimeA->entityIndex;
            DeleteEntity(slimeB);
        }
    }
    
#if GAME_INTERNAL
     gameState->canSwitchSlime = true;
     gameState->canSplitSlime = true;
    #endif
    
    gameState->currentMapIndex = -1;
    gameState->simulating = false;
    
    gameState->camera.followEntityIndex = gameState->playerEntityIndex;
    gameState->camera.followState = MyCamera::LOCK_TO_MAP;
    
    gameState->shakeTime = 0.0f;
    
    UpdateCamera();
    // ResetResetStates();
    
    // NOTE: reveal last map
    if (!gameState->isTestLevel && gameState->lastMap) RevealMap(*gameState->lastMap);
    
    
    // NOTE: Initalize gameState->undoStack record
    gameState->undoStack.reset();
    
    // NOTE: push initial state as restart state
    DynamicArray<Entity> ea = GetCurrentStateEntities();
    InitUndoState(&gameState->restartState, gameState->playerEntityIndex, gameState->starCount,ea);
                  
}

void CleanUpGame()
{
    gameState->undoStack.reset();
    
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
        GuiSetStyle(DEFAULT, TEXT_SIZE, 50);
        }
    
    if (!gameState->input.initialized)
    {
        gameState->input.initialized = true;
        InitKeyMapping(gameState->input.keyMappings);
    }
    
    Color colorA = IntToRGBA(0x62345); // 0x163355 0x4545 0x62345
     
    gameState->bgColor = colorA;
    
    switch(gameState->currentScreen)
    {
        case TITLE_SCREEN:
        {
            
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            DrawScrollingBackGround(gameState->bgTexture, DARKGREEN);
            
            const char * Title = "TITLE SCREEN";
            int32 TitleTextX = (GetScreenWidth() - MeasureText(Title, 40)) / 2;
            int32 TitleTextY = (GetScreenHeight() - 40) / 2 - 100;
            DrawText(Title, TitleTextX, TitleTextY, 40, WHITE);
            
            const char * Instructions = "PRESS SPACE";
            int32 instX = (GetScreenWidth() - MeasureText(Instructions, 20)) / 2;
            int32 instY = (GetScreenHeight()) / 2;
            DrawText(Instructions, instX, instY, 20, WHITE);
            
            if (gameState->switching || JustPressed(gameState->input.keyMappings, SPLIT_KEY))
            {
                ChangeScreen(MENU_SCREEN);
            }
            
            break;
        }
        case MENU_SCREEN:
        {
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            DrawScrollingBackGround(gameState->bgTexture, PINK);
            
            uint32 numButtons = 2;
            real32 margin = 200.0f;
            
            real32 width = GetScreenWidth() - 600.0f;
            real32 height = 100.0f;
            width = Clamp(width, 300.0f, 400.0f);
            height = Clamp(height, 9.0f, 150.0f);
            
            real32 totalHeight = height + (numButtons - 1) * margin;
            
            real32 x = (GetScreenWidth() - width) * 0.5f;
            real32 y = (GetScreenHeight() - totalHeight) * 0.5f;
            
            real32 padding = 10.0f;
            if (x < padding) x = padding;
            if (y < padding) y = padding;
            
            const char * NewGameText = "New Game";
            
            
            Rectangle bounds =
            {
                x,
                y,
                width,
                height
            };
            
            if (GuiButton(bounds, NewGameText))
            {
                LoadTileMapsAndEntities(MAIN_PATH);
                gameState->isTestLevel = false;
                ChangeScreen(GAME_MAIN_SCREEN);
            }
            
            #if 0
            // TODO: Experimental features, Very breakable!!!
            const char * LoadGameText = "load game";
            bounds.y += margin;
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
                    ChangeScreen(GAME_MAIN_SCREEN);
                }
                else
                {
                    SM_ERROR("faile to open file %s", fileName);                    
                }
            };
            
#endif

#if GAME_INTERNAL
            
            bounds.y += margin;
            const char * TestLevel = "test level";
            if (GuiButton(bounds, TestLevel))
            {
                LoadTileMapsAndEntities(TEST_PATH);
                gameState->isTestLevel = true;
                ChangeScreen(GAME_MAIN_SCREEN);
            }
#endif
            
            bounds.y += margin;
            const char * QuitGame = "Quit Game";
            if (GuiButton(bounds, QuitGame))
            {
                *running = false;   
            }
            
            break;            
        }
        case PAUSE_MENU_SCREEN:
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                ChangeScreen(GAME_MAIN_SCREEN);
            }
            
            BeginDrawing();
            ClearBackground(gameState->bgColor);
            
            DrawScrollingBackGround(gameState->bgTexture, PURPLE);
            
            uint32 numButtons = 2;
            real32 margin = 200.0f;
            
            real32 width = GetScreenWidth() - 600.0f;
            real32 height = 100.0f;
            width = Clamp(width, 300.0f, 400.0f);
            height = Clamp(height, 9.0f, 150.0f);
            
            real32 totalHeight = height + (numButtons - 1) * margin;
            
            real32 x = (GetScreenWidth() - width) * 0.5f;
            real32 y = (GetScreenHeight() - totalHeight) * 0.5f;
            
            real32 padding = 10.0f;
            if (x < padding) x = padding;
            if (y < padding) y = padding;
            
            const char * ContinueGameText = "Continue";
            Rectangle bounds =
            {
                x,
                y,
                width,
                height
            };
            
            if (GuiButton(bounds, ContinueGameText))
            {
                ChangeScreen(GAME_MAIN_SCREEN);
            }
            
            #if 0
            // TODO: Experimental features, Very breakable!!!
            const char * SaveGameText = "save game";
            bounds.y += margin;
            if (GuiButton(bounds, SaveGameText))
            {
                SaveGame(*gameState, GAME_SAVE_PATH);
            }
#endif
            
            const char * QuitMenuText = "Main Menu";
            bounds.y += margin;
            if (GuiButton(bounds, QuitMenuText))
            {
                CleanUpGame();
                ChangeScreen(MENU_SCREEN);
                gameState->fog.initialized = false;
                UnloadRenderTexture(gameState->fog.fogRenderTex);
                // NOTE: check memory leak!
                }
            
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
                ChangeScreen(PAUSE_MENU_SCREEN);
            }
            break;
        }
    }
    
    if (gameState->switching)
    {
        real32 fadeSpeed = 5.0f;
        static real32 alpha = 0;
        static bool fadeIn = true;
        
        if (fadeIn)
        {
        // NOTE FadeIn
        alpha += fadeSpeed * GetFrameTime();
        
        if (alpha >= 1.0f)
            {
                alpha = 1.0f;
                fadeIn = false;
                
                if (gameState->currentScreen == GAME_MAIN_SCREEN &&
                    gameState->nextScreen == GAME_TUT_SCREEN)
                {
                    // NOTE: Load Level
                    CleanUpGame();
                    char * currentMapID = gameState->tileMaps[gameState->currentMapIndex].mapID;
                    const char * levelPath = GetDirectoryPath(MAIN_PATH);
                    
                    char worldPath[100];
                    sprintf(worldPath, "%s/%s_Tutorial.world", levelPath, currentMapID);
                    LoadTileMapsAndEntities(worldPath);
                    
                }
                else if (gameState->currentScreen == GAME_TUT_SCREEN &&
                         gameState->nextScreen == GAME_MAIN_SCREEN)
                {
                    Entity player = *GetPlayer();
                    CleanUpGame();
                    LoadTileMapsAndEntities(MAIN_PATH);
                    SetGameState(gameState->lastState);
                    // NOTE: Reset electric door sprite
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
                    
                }
                gameState->currentScreen = gameState->nextScreen;
                }
        }
        else
        {
            // NOTE FadeOut
            alpha -= fadeSpeed * GetFrameTime();
            
            if (alpha <= 0.0f)
            {
                alpha = 0.0f;
                fadeIn = true;
                gameState->switching = false;
            }
            
        }
        DrawRectangleRec(Rectangle{0,0,(real32)GetScreenWidth(), (real32)GetScreenHeight()},
                      Fade(BLACK, alpha));
        
        }
    
    EndDrawing();
    }
