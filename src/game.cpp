
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
- Make The Save File Less buggy! (Serialize It, so that game won't break when level/code change)
- Try use particle systems to render Startfield background to impore performence
- Draw Tile Grid with texture to reduce draw call
- Create a load menu to choose save file
  - Drag and drop save file could be fun
  - smooth pixelperfect transition
  - top down lights / spotlight rendering
  - add particles
  - Dropdown console commands 
  - Texture filtering when zooming out (is mipmapping come handy here?)
  - Viewport scaling IMPORTANT: DO we really need this ? TODO: YES!!
  - Assets Managment
  - Bit masking with tile rules

  NOTE: done
        - Batch all sprite into a single draw call (raylib handled that internally, see https://www.raylib.com/examples/textures/loader.html?name=textures_bunnymark)

     - background effects (try this: https://github.com/raysan5/raylib/blob/master/examples/shapes/shapes_starfield_effect.c)
  - Implement save points (Since the state of our game is entirely based oneach state of the entity,
                           we can just read/write raw bytes of entities to a file)
  - Gamepad supports
  - (MoveActionCheck) When Door and block are in the same tile, we should check if the door is blocked first, then check if we can push the block
  - Change animation controller into a tweening controller that is able to tween arbitarty types of values using easing functions
  - Basic Scene Manager
  - (UpdateElectricDoor) Connection point Logic needs to be refine. Check comments in the function 
  - Need to refactor level_loader, we should have separate function to load entities, load tilemaps, and setup entity table.
*/
//  ========================================================================
//              NOTE: Game Structs (internal)
//  ========================================================================

//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================

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
            if (!c.NoTweens()) playEvent = &c.endEvent;
            
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
    if (!pushEnt->tweenController.NoTweens())
    {
        playEvent = &pushEnt->tweenController.endEvent;
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
                        
                        return;
                    }
                    
                    MoveEntity(projectedEnt, nullptr, playEvent, pos - pushDir,  BLOCK_MOVE_FUNC);
                        
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
                    newThings.parent = &newThings;
                    checkList.Add(newThings);
                    
                    Entity * attach = defered ? pushEnt : target;
                    
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
                    SetGlassBeBroken(target);
                    Entity * attachSlime = FindAttachSlime(target);
                    if (attachSlime) attachSlime->attach = false;
                    
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
                    // TODO
                    #if 0
                    CheckThings newThings = {};
                    newThings.visited = false;
                    newThings.pushDir = current.pushDir;
                    newThings.pushEnt = target;
                    newThings.pushResult = { false, PUSH_NONE, nullptr };
                    newThings.checkType = CHECK_MOVE;
                    newThings.parent = &current;
                    checkList.Add(newThings);
                    return;
                    #endif
                }
                }
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
                
                if (IsProjectable(target, current.pushDir))
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
    EntityLayer checkLayers[] = { LAYER_WALL, LAYER_DOOR, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK, LAYER_PIT };
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
                        playEvent = &parent->parent->pushEnt->tweenController.endEvent;
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

inline bool8 UpdateCamera()
{
    bool8 updated = false;
    Entity * player = GetPlayer();
    SM_ASSERT(player, "Player is not active");
    
    Vector2 playerTile = player->pivot;
    
    for (int i = 0; i < gameState->tileMapCount; i++)
    {
        Map & map = gameState->tileMaps[i];
        Vector2 mapMin = GetTilePivot(map.tilePos, MAP_TILE_SIZE);
        
        Rectangle playerRec = GetEntityRect(player);
        Rectangle tileMapRec =
        {
            mapMin.x + MAP_TILE_SIZE,
            mapMin.y + MAP_TILE_SIZE,
            (float)map.width  * (float)MAP_TILE_SIZE,
            (float)map.height * (float)MAP_TILE_SIZE
        };
        
        if( CheckCollisionRecs(playerRec, tileMapRec) )
        {
            
            Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, map.height * 0.5f + map.tilePos.y + 0.5f);
            
            if (IsKeyPressed(KEY_TAB))
            {
                UpdateCameraToTileMapSmooth(map, pos, i);
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
    for (int i = 0; i < undoEntities.size(); i++)
    {
        Entity e = undoEntities[i];
        gameState->entities[i] = e;
        gameState->entities[i].tweenController.Reset();
        
        if (IsSlime(&e) && (e.actionState == ANIMATE_STATE || e.actionState == SPLIT_STATE))
        {
            gameState->entities[i].actionState = MOVE_STATE;
        }
        
    }
}

inline void Undo()
{
    UndoState & undoState = undoStack.back();
    gameState->playerEntityIndex = undoState.playerIndex;
    
    std::vector<Entity> & undoEntities = undoState.undoEntities;
    SetUndoEntities(undoEntities);        
    undoStack.pop_back();
}


inline void Restart()
{
    undoStack.push_back({ gameState->playerEntityIndex, gameState->entities.GetVectorSTD() });    
    
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
    
    {
        EntityLayer layers[] = { LAYER_DOOR };
        Entity * door = FindEntityByLocationAndLayers(currentPos, layers, ArrayCount(layers));
        if (door && DoorBlocked(door, -actionDir))
        {
            return false;
        }
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
                ActionCheck(player, player->attachDir, CHECK_MOVE);
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
                EntityLayer layers[] = { LAYER_SLIME };
                Entity * slime = FindEntityByLocationAndLayers(standingPlatformPos, layers, ArrayCount(layers));
                if (slime)
                {
                    player = MergeSlimes(slime, player);
                }
                else if ((!findResult.entity || findResult.entity->type != ENTITY_TYPE_PIT) &&
                         Abs(player->attachDir) != Abs(actionDir))
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
            bool8 blockedByPit = pushResult.blockedEntity->type == ENTITY_TYPE_PIT;
            bool8 blockedByDoor = 
                pushResult.blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                pushResult.blockedEntity->cableType == CABLE_TYPE_DOOR &&
                !SameSide(pushResult.blockedEntity, player->tilePos + actionDir, actionDir);
            
            if (blockedByPit || blockedByDoor)
            {
                return false;
            }
            
            if (player->attachDir == -actionDir)
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
            
            MergeSlimes(pushResult.mergeEntity, player);
            return true;
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
    
    ActionCheck(player, bounceDir, CHECK_PROJECT);
    
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
            DeleteEntity(clone);
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
    
    if (JustPressed(POSSES_KEY)) //  && gameState->lv2Map && gameState->lv2Map->firstEnter)
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
    
    // NOTE: Actions
     {
        // NOTE: Recored if State Changes
        static bool8 stateChanged = false;
        
        Entity * player = GetEntity(gameState->playerEntityIndex);
        
        UndoState prevState = { gameState->playerEntityIndex, gameState->entities.GetVectorSTD() };
        
        // NOTE SlimeSelection
        stateChanged = SlimeSelection(player);
        
        {
            switch(player->actionState)
            {
                case MOVE_STATE:
                {
                    gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = false;
                    
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
            
            if (stateChanged && !gameState->simulating)
            {
                stateChanged = false;
                undoStack.push_back(prevState);
            }
        }
        
         UpdateElectricDoor();
        UpdateSlimes();
        
        // NOTE: Undo and Restart
        {
            static bool8 repeat = false;
            static float timeSinceLastPress = 0;
            
            timeSinceLastPress -= GetFrameTime();
            
            if (timeSinceLastPress < 0 && IsDown(UNDO_KEY) && !undoStack.empty())
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
        
        
    }
    
    // NOTE: Simulate
    {
        gameState->simulating = false;
        // NOTE: Update: Entity
        for (uint32 i = 0; i < gameState->entities.count; i++)
        {
            Entity * entity = GetEntity(i);
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
                    // entity->pivot = entity->tweenController.currentPosition;
                }
                else
                {
                    if (entity->actionState == ANIMATE_STATE) SetActionState(entity, MOVE_STATE);
                    entity->pivot = GetTilePivot(entity);
                    // entity->tweenController.HandleAnimationNotPlaying();
                }
            } 
        }
        
    } 
    
    
    // NOTE: Arrow Setup
    {
        IVec2 centerPos = GetPlayer()->tilePos;
        
        IVec2 upPos    = { centerPos.x, (centerPos.y-1) };
        IVec2 downPos  = { centerPos.x, (centerPos.y+1) };
        IVec2 leftPos  = { (centerPos.x-1), (centerPos.y) };
        IVec2 rightPos = { (centerPos.x+1), (centerPos.y) };
        
        IVec2 dir[4] = { upPos, downPos, leftPos, rightPos };
        Arrow * arrows[4] = { &gameState->upArrow, &gameState->downArrow, &gameState->leftArrow, &gameState->rightArrow };
        for (int i = 0; i < 4; i++)
        {
            Arrow * arrow = arrows[i];
            arrow->topLeftPos = GetTilePivot(dir[i], (float)arrow->tileSize);
        }
    }
    
    // NOTE: Render
    {
        
        // NOTE: Draw
        BeginDrawing();
        
        ClearBackground(gameState->bgColor);
        
        UpdateAndDrawStarFieldBG(&gameState->starFields);
        
        BeginMode2D(gameState->camera);
        
        for (int i = 0; i < gameState->tileMapCount; i++)
        {
            Map & map = gameState->tileMaps[i];
             DrawTileMap(gameState->camera, map.tilePos, { map.width, map.height }, SKYBLUE, Fade(DARKGRAY, 0.2f));
        }
        
        EntityLayer orderedDrawLayers[] = { LAYER_BLOCK, LAYER_WALL, LAYER_CABLE, LAYER_PIT,  LAYER_DOOR, LAYER_SLIME, LAYER_GLASS };
        
        int count = ArrayCount(orderedDrawLayers);
        DrawSpriteLayers(orderedDrawLayers, count);
        
        // Draw rectangle outline with extended parameters
        // Rectangle cameraRect = GetCameraRect(gameState->camera);
        // DrawRectangleLinesEx(cameraRect, 1, RED);
        
        // Left
        if (gameState->leftArrow.show)
        {
            DrawSprite(gameState->camera, gameState->texture, gameState->leftArrow.sprite, gameState->leftArrow.topLeftPos, (float)gameState->leftArrow.tileSize);
        }
        // Right
        if (gameState->rightArrow.show)
        {
            DrawSprite(gameState->camera, gameState->texture, gameState->rightArrow.sprite, gameState->rightArrow.topLeftPos, (float)gameState->rightArrow.tileSize);
        }
        
        // Up
        if (gameState->upArrow.show)
        {
            DrawSprite(gameState->camera, gameState->texture, gameState->upArrow.sprite, gameState->upArrow.topLeftPos, (float)gameState->upArrow.tileSize);
        }
        // Down
        if (gameState->downArrow.show)
        {
            DrawSprite(gameState->camera, gameState->texture, gameState->downArrow.sprite, gameState->downArrow.topLeftPos, (float)gameState->downArrow.tileSize);
        }
        
        EndMode2D();
        
#if 0 // GAME_INTERNAL
        // NOTE: UI Draw Game Informations
        
        Entity * player = GetEntity(gameState->playerEntityIndex);
        IVec2 centerPos = player->tilePos;
        
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
        
        DrawText(TextFormat("Player pivot (%.2f, %.2f), mouse world (%.2f, %.2f)",
                            player->pivot.x, player->pivot.y, mousePos.x, mousePos.y ), 10, 200, 20, GREEN);
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: %.2f,  Entity Count: %i",
                            centerPos.x, centerPos.y,
                            player->mass, player->tileSize,  gameState->entities.count), 10, 140, 20, GREEN);
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState->camera.target.x, gameState->camera.target.y,
                            gameState->camera.offset.x, gameState->camera.offset.y, gameState->camera.zoom), 10, 50, 20, RAYWHITE);
        DrawText("Arrow Direction to Shoot, R KEY to Restart, Z KEY to undo", 10, 10, 20, RAYWHITE);
        
        DrawText(TextFormat("%.2f ms\n%iFPS", 1000.0f / GetFPS(), GetFPS()), 10, 300, 20, GREEN);
        
        int posX = GetScreenWidth() - MeasureText("Entity Action State: FFFFFFFFFFFFFFFFFFFF", 20);
        DebugDrawPlayerActionState(player->actionState, posX, 50, 20, IntToRGBA(0x923eed));
        
        if (gameState->simulating)
        {
            DrawText("Game Simulating", gameState->screenWidth / 4, gameState->screenHeight / 4, 20, RED);
            }
        
#endif
        
        EndDrawing();
    }    
}

void InitializeGame()
{
    // NOTE: Initialization
    gameState->initialized = true;
    
    // NOTE: Initilize Arrows
    // UP
    gameState->upArrow.sprite = GetSprite(SPRITE_ARROW_UP);
    gameState->upArrow.id = SPRITE_ARROW_UP;
    gameState->upArrow.tileSize = 16;
    
    // DOWN
    gameState->downArrow.sprite = GetSprite(SPRITE_ARROW_DOWN);
    gameState->downArrow.id = SPRITE_ARROW_DOWN;
    gameState->downArrow.tileSize = 16;
    
    // LEFT
    gameState->leftArrow.sprite = GetSprite(SPRITE_ARROW_LEFT);
    gameState->leftArrow.id = SPRITE_ARROW_LEFT;
    gameState->leftArrow.tileSize = 16;
    
    // RIGHT
    gameState->rightArrow.sprite = GetSprite(SPRITE_ARROW_RIGHT);
    gameState->rightArrow.id = SPRITE_ARROW_RIGHT;
    gameState->rightArrow.tileSize = 16;
    
    InitKeyMapping();
    
    // NOTE: Initiaize slimes
    {
        auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
        for (uint32 i = 0; i < slimeEntityIndices.count; i++)
        {
            int index = slimeEntityIndices[i];
            Entity * entity = GetEntity(index);
            if (!entity) continue;
            
            IVec2 directions[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
            
            for (int j = 0; j < 4; j++)
            {
                if (AttachSlime(entity, directions[j])) break;
            }
        }
    }
    
    // NOTE: SetUp Electric Door
    SetUpElectricDoor();
    
    // NOTE: Initalize undoStack record
    undoStack = std::vector<UndoState>();
    undoStack.push_back({ gameState->playerEntityIndex, gameState->entities.GetVectorSTD() });
    
    gameState->currentMapIndex = -1;
    gameState->simulating = false;
    
}

void CleanUpGame()
{
    undoStack = std::vector<UndoState>();
    CleanUpKeyMapping();
    
    gameState->initialized = false;    
    gameState->cameraTweenController.Reset();
    for (int i = 0; i < LAYER_COUNT; i++)
    {
        gameState->entityTable[i].Clear();
    }
    gameState->electricDoorSystem.CleanUp();
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
    
    // TODO: Temp code
    static bool8 init = false;
    if (!init)
    {
        init = true;
        GuiLoadStyle(RAYLIB_GUI_STYLE_PATH);
    }
    
    Color colorA = IntToRGBA(0x222f);
    
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
                LoadTileMapsAndEntities(*gameState);
                gameState->currentScreen = GAMEPLAY_SCREEN;
            }
            
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
                    LoadTileMapsAndEntities(*gameState);
                    
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
                    gameState->currentScreen = GAMEPLAY_SCREEN;
                }
                else
                {
                    SM_ERROR("faile to open file %s", fileName);                    
                }
            };
            
            const char * TestLevel = "test level";
            bounds.y += 200;
            if (GuiButton(bounds, TestLevel))
            {
                LoadTestLevel(*gameState);
                gameState->currentScreen = GAMEPLAY_SCREEN;
                
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
                gameState->currentScreen = GAMEPLAY_SCREEN;
            }
            
            // TODO: Experimental features, Very breakable!!!
            const char * SaveGameText = "save game";
            bounds.y += 200;
            if (GuiButton(bounds, SaveGameText))
            {
                EntityLayer saveLayers[] = { LAYER_DOOR, LAYER_CABLE, LAYER_GLASS, LAYER_SLIME, LAYER_BLOCK };
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
        case GAMEPLAY_SCREEN:
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
