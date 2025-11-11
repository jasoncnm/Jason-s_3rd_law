
#include "game.h"
#include "game_util.h"
#include "render_interface.h"
#include "action_input.h"

#include "entity.cpp"
#include "electric_door.cpp"
#include "level_loader.cpp"
#include "tween.cpp"
#include "tween_controller.cpp"

/*
TODO: Things that I can do beside arts and design I guess
  - background effects (try this: https://github.com/raysan5/raylib/blob/master/examples/shapes/shapes_starfield_effect.c)
  - smooth pixelperfect transition
  - top down lights / spotlight rendering
  - add particles
  - Dropdown console commands 
  - Texture filtering when zooming out (is mipmapping come handy here?)
  - Viewport scaling IMPORTANT: DO we really need this ?
  - BUGS:
  -  (UpdateElectricDoor) Connection point Logic needs to be refine. Check to TODO comments in the function 

NOTE: done
  - Gamepad supports
  - (MoveActionCheck) When Door and block are in the same tile, we should check if the door is blocked first, then check if we can push the block
  - Change animation controller into a tweening controller that is able to tween arbitarty types of values using easing functions
  - Basic Scene Manager

*/

//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================
MoveActionResult MoveActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int accumulatedMass)
{
    SM_ASSERT(startEntity->movable, "Static entity cannot be pushing blocks!");
    
    MoveActionResult result = { false, false, nullptr };
    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        auto & entityTable = gameState->entityTable[layer];
        for (int i = 0; i < entityTable.count; i++)
        {
            Entity * target = GetEntity(entityTable[i]);
            if (target && target->tilePos == blockNextPos)
            {
                switch(target->type)
                {
                    case ENTITY_TYPE_GLASS:
                    {
                        if (!target->broken)
                        {
                            result.blocked = true;
                            result.blockedEntity = target;
                            return result;
                        }
                        break;
                    }
                    case ENTITY_TYPE_ELECTRIC_DOOR:
                    {
                        if (target->cableType == CABLE_TYPE_DOOR && DoorBlocked(target, pushDir))
                        {
                            result.blocked = true;
                            result.blockedEntity = target;
                            return result;
                        }
                    
                        break;
                    }
                    case ENTITY_TYPE_PIT:
                    case ENTITY_TYPE_WALL:
                    {
                        result.blocked = true;
                        result.blockedEntity = target;
                        return result;
                    }
                    case ENTITY_TYPE_PLAYER:
                    case ENTITY_TYPE_CLONE:
                    {
                        if (pushEntity->type == ENTITY_TYPE_CLONE || pushEntity->type == ENTITY_TYPE_PLAYER)
                        {
                            if (pushDir == -pushEntity->attachDir)
                            {

                                result.pushed = false;
                                return result;
                            }
                            MergeSlimes( target, pushEntity);
                            return result;
                        }

                        if (pushEntity->type == ENTITY_TYPE_BLOCK)
                        {
                            FindAttachableResult attachResult = FindAttachable(target->tilePos + pushDir, pushDir);
                            if (attachResult.has)
                            {
                                SetAttach(target, attachResult.entity, pushDir);
                            }
                            else if (target->attach)
                            {
                                attachResult = FindAttachable(target->tilePos + target->attachDir, target->attachDir);
                                if (attachResult.has)
                                {
                                    SetAttach(target, attachResult.entity, target->attachDir);
                                }
                            }
                            // SetAttach(target, pushEntity, -pushDir);
                        }
                    
                    }
                    case ENTITY_TYPE_BLOCK:
                    {
                        Entity * door = FindEntityByLocationAndLayer(target->tilePos, LAYER_DOOR);
                        if (door && DoorBlocked(door, -pushDir))
                        {
                            result.pushed = false;
                            result.blocked = true;
                            result.blockedEntity = target;
                                
                            return result;
                        }

                        
                        if (CheckBounce(target->tilePos, pushDir))
                        {
                            result.pushed = true;

                            IVec2 startTile = target->tilePos;
                            Vector2 moveStart = GetTilePivot(target);
                        
                            BounceEntity(target, pushDir);

                            Vector2 moveEnd = GetTilePivot(target);
                            float dist = Vector2Distance(moveStart, moveEnd);
                            float tileDist = dist / MAP_TILE_SIZE;

                            if (!Vector2Equals(moveStart, moveEnd))
                            {
                                TweenParams params = {};
                                params.paramType = PARAM_TYPE_VECTOR2;
                                params.startVec2 = moveStart;
                                params.endVec2 = moveEnd;
                                params.realVec2  = &target->pivot;

                                AddTween(target->tweenController, CreateTween(params, nullptr, BOUNCE_SPEED, tileDist));
                        
                                if ((startTile - pushEntity->tilePos).SqrMagnitude() > 1)
                                {
                                    pushEntity->tweenController.endEvent.controller = &target->tweenController;
                                    pushEntity->tweenController.endEvent.OnPlayFunc = OnPlayEvent;
                                }
                                else
                                {
                                    OnPlayEvent(&target->tweenController);
                                }
                                                    
                                if (!target->active)
                                {
                                    target->active = true;
                                    target->tweenController.endEvent.deleteEntity = target;
                                    target->tweenController.endEvent.OnDeleteFunc = DeleteEntity;
                                }

                            }
                            else
                            {
                                result.pushed = false;
                                result.blocked = true;
                                result.blockedEntity = target;
                            }
                            return result;
                        }
                        else
                        {
                            
                            int newAccumulatedMass = accumulatedMass + target->mass;
                            if (newAccumulatedMass > startEntity->mass)
                            {
                                result.blockedEntity = target;
                                result.blocked = true;
                                return result;
                            }


                            result = MoveActionCheck(startEntity, target, blockNextPos + pushDir, pushDir, newAccumulatedMass);
                    
                            if (!result.blocked)
                            {
                                IVec2 startTile = target->tilePos;
                                result.pushed = true;

                                Vector2 moveStart = GetTilePivot(target);
                                SetEntityPosition(target, nullptr, blockNextPos + pushDir);

                                Vector2 moveEnd = GetTilePivot(target);
                                float dist = Vector2Distance(moveStart, moveEnd);
                                float iDist = dist / MAP_TILE_SIZE;

                                TweenParams params = {};
                                params.paramType = PARAM_TYPE_VECTOR2;
                                params.startVec2 = moveStart;
                                params.endVec2 = moveEnd;
                                params.realVec2  = &target->pivot;

                                AddTween(target->tweenController, CreateTween(params, nullptr, BOUNCE_SPEED, iDist));

                                if ((startTile - pushEntity->tilePos).SqrMagnitude() > 1)
                                {
                                    pushEntity->tweenController.endEvent.controller = &target->tweenController;
                                    pushEntity->tweenController.endEvent.OnPlayFunc = OnPlayEvent;
                                }
                                else
                                {
                                    OnPlayEvent(&target->tweenController);
                                }
                            }
                            else
                            {
                                result.blockedEntity = target;
                            }
                        
                            return result;
                        }
                    
                        break;
                    }
                }
            }
        }
    }
 
    // SM_ASSERT(false, "at leat one object is being pushed or blocked");
    return result;
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

inline bool UpdateCamera()
{
    bool updated = false;
    Entity * player = GetEntity(gameState->playerEntityIndex);
    SM_ASSERT(player, "Player is not active");
            
    Vector2 playerTile = player->pivot;
    
    for (int i = 0; i < gameState->tileMapCount; i++)
    {
        Map & map = gameState->tileMaps[i];
        Vector2 mapMin = GetTilePivot(map.tilePos, MAP_TILE_SIZE);

        Rectangle playerRec = { player->pivot.x, player->pivot.y, player->tileSize, player->tileSize };
        Rectangle tileMapRec = { mapMin.x + MAP_TILE_SIZE, mapMin.y + MAP_TILE_SIZE, (float)map.width * (float)MAP_TILE_SIZE, (float)map.height * (float)MAP_TILE_SIZE };

        if( CheckCollisionRecs(playerRec, tileMapRec) )
        {
            
            Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, map.height * 0.5f + map.tilePos.y + 0.5f);
            if (!Vector2Equals(pos, gameState->camera.target))
            {
                if (!map.firstEnter)
                {
                    map.initUndoState = { gameState->playerEntityIndex, gameState->entities.GetVectorSTD() };
                    map.firstEnter = true;
                }

                if (gameState->currentMapIndex == -1)
                {
                    gameState->camera.rotation = 0.0f;
                    gameState->camera.target = pos;
                    gameState->camera.zoom = GetCameraZoom(map);
                    gameState->camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
                }
                
                if (gameState->currentMapIndex != i)
                {
                    gameState->cameraTweenController.Reset();
                    
                    TweenParams params = {};
                    params.paramType = PARAM_TYPE_VECTOR2;
                    params.startVec2 = gameState->camera.target;
                    params.endVec2 = pos;
                    params.realVec2  = &gameState->camera.target;
                    AddTween(gameState->cameraTweenController, CreateTween(params, EaseOutCubic, 1.7f), 0);

                    Map & lastMap = gameState->tileMaps[gameState->currentMapIndex];
                    float oldZoom = GetCameraZoom(lastMap);
                    float newZoom = GetCameraZoom(map);
                    if (!FloatEquals(oldZoom, newZoom))
                    {
                        TweenParams params = {};
                        params.paramType = PARAM_TYPE_FLOAT;
                        params.startF = oldZoom;
                        params.endF = newZoom;
                        params.realF  = &gameState->camera.zoom;
                        AddTween(gameState->cameraTweenController, CreateTween(params, EaseOutCubic, 1.7f), 1);
                    }
                    
                    OnPlayEvent(&gameState->cameraTweenController);
                    gameState->currentMapIndex = i;
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

        if (IsSlime(&e))
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

bool MoveAction(IVec2 actionDir)
{
    Entity * mother = GetEntity(gameState->playerEntityIndex);
    SM_ASSERT(mother, "player is not active");

    if (!mother->attach) return false;

    float moveSpeed = 4.0f;

    IVec2 currentPos = mother->tilePos;
    IVec2 actionTilePos = currentPos + actionDir;
    if (mother->attachDir == actionDir)
    {
        return false;
    }

    Entity * door = FindEntityByLocationAndLayer(currentPos, LAYER_DOOR);
    if (door && DoorBlocked(door, -actionDir))
    {
        return false;
    }
        
    //MoveActionResult moveResult = MoveActionCheck(mother, mother, actionTilePos, actionDir, 0);
    MoveActionResult moveResult = MoveActionCheck(mother, mother, actionTilePos, actionDir, 0);
        
    if (moveResult.blocked)
    {
        if (moveResult.blockedEntity->type == ENTITY_TYPE_PIT ||
            (moveResult.blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
             moveResult.blockedEntity->cableType == CABLE_TYPE_DOOR &&
             !SameSide(moveResult.blockedEntity, actionTilePos, actionDir)))
        {
            return false;
        }
            
        if (mother->attachDir == -actionDir)
        {
            // NOTE: Bounce with the block
            MoveActionResult result = MoveActionCheck(mother, mother, mother->tilePos + mother->attachDir, mother->attachDir, 0);
            if (result.blocked)
            {
                // IMPORTANT: mother entity changed
                Vector2 startPos = GetTilePivot(mother);
                SetAttach(mother, moveResult.blockedEntity, actionDir);
                Vector2 endPos = GetTilePivot(mother);
                    
                TweenParams params = {};
                params.paramType = PARAM_TYPE_VECTOR2;
                params.startVec2 = startPos;
                params.endVec2 = endPos;
                params.realVec2  = &mother->pivot;

                AddTween(mother->tweenController, CreateTween(params, EaseOutCubic, moveSpeed));
                OnPlayEvent(&mother->tweenController);
                    
                return true;
            }
            return true;
        }

        // IMPORTANT: mother entity changed
        Vector2 moveStart = GetTilePivot(mother);
        Vector2 moveMiddle =
            Vector2Add(moveStart, Vector2Scale({ (float)actionDir.x, (float)actionDir.y }, 0.5f *(MAP_TILE_SIZE - mother->tileSize)));
        SetAttach(mother, moveResult.blockedEntity, actionDir);
        Vector2 moveEnd = GetTilePivot(mother);
                    
        TweenParams params1 = {};
        params1.paramType = PARAM_TYPE_VECTOR2;
        params1.startVec2 = moveStart;
        params1.endVec2 = moveMiddle;
        params1.realVec2  = &mother->pivot;
            
                    
        TweenParams params2 = {};
        params2.paramType = PARAM_TYPE_VECTOR2;
        params2.startVec2 = moveMiddle;
        params2.endVec2 = moveEnd;
        params2.realVec2  = &mother->pivot;

        AddTween(mother->tweenController, CreateTween(params1, EaseOutCubic, moveSpeed * 2));
        AddTween(mother->tweenController, CreateTween(params2, EaseOutCubic, moveSpeed * 2));
        OnPlayEvent(&mother->tweenController);
            
        return true;
    }

    if (mother->attachDir == -actionDir)
    {
        return moveResult.pushed;
    }
        
    if (!mother->active) return false;

    
    // NOTE: no obsticale, move player
    IVec2 standingPlatformPos = actionTilePos + mother->attachDir;
    FindAttachableResult findResult = FindAttachable(standingPlatformPos, mother->attachDir);
    if (findResult.has)
    {
        Entity * resultEntity = findResult.entity;
        if (resultEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
            resultEntity->cableType == CABLE_TYPE_DOOR &&
            !SameSide(resultEntity, standingPlatformPos, mother->attachDir))
        {
            return false;
        }
                
        // IMPORTANT: mother entity changed
        Vector2 moveStart = GetTilePivot(mother);
        SetEntityPosition(mother, findResult.entity, actionTilePos);
        Vector2 moveEnd = GetTilePivot(mother);

        TweenParams params = {};
        params.paramType = PARAM_TYPE_VECTOR2;
        params.startVec2 = moveStart;
        params.endVec2 = moveEnd;
        params.realVec2  = &mother->pivot;
                
        AddTween(mother->tweenController, CreateTween(params, EaseOutCubic, moveSpeed));
        OnPlayEvent(&mother->tweenController);
                
    }
    else
    {
        Entity * slime = FindEntityByLocationAndLayer(standingPlatformPos, LAYER_SLIME);
        if (slime)
        {
            mother = MergeSlimes(slime, mother);
        }
        else if ((!findResult.entity || findResult.entity->type != ENTITY_TYPE_PIT) &&
                 Abs(mother->attachDir) != Abs(actionDir))
        {
            IVec2 newTile = standingPlatformPos;
            IVec2 newAttach = - actionDir;

            Entity * attachedEntity = GetEntity(mother->attachedEntityIndex);

            if (attachedEntity && attachedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                attachedEntity->cableType == CABLE_TYPE_DOOR &&
                !SameSide(attachedEntity, newTile, newAttach))
            {
                return false;
            }
                
            // IMPORTANT: mother entity changed
            Vector2 moveStart = GetTilePivot(mother);
            Vector2 movemiddle =
                Vector2Add(moveStart, Vector2Scale({ (float)actionDir.x, (float)actionDir.y }, 0.5f * (MAP_TILE_SIZE + mother->tileSize)));
                
            SetEntityPosition(mother, attachedEntity, newTile);
            Vector2 moveEnd = GetTilePivot(mother);

            TweenParams params1 = {};
            params1.paramType = PARAM_TYPE_VECTOR2;
            params1.startVec2 = moveStart;
            params1.endVec2 = movemiddle;
            params1.realVec2  = &mother->pivot;

            TweenParams params2 = {};
            params2.paramType = PARAM_TYPE_VECTOR2;
            params2.startVec2 = movemiddle;
            params2.endVec2 = moveEnd;
            params2.realVec2  = &mother->pivot;

            AddTween(mother->tweenController, CreateTween(params1, EaseOutCubic, moveSpeed * 1.5f));
            AddTween(mother->tweenController, CreateTween(params2, EaseOutCubic, moveSpeed * 1.5f));

            OnPlayEvent(&mother->tweenController);                

        }
        else 
        {
            return false;
        }

    }

    return true;
}

bool SplitAction(Entity * player, IVec2 bounceDir)
{

    if (player->mass < 2) return false;

    player->mass--;
    player->tileSize = GetSlimeSize(player);
    
    Entity * clone = CreateSlimeClone(player->tilePos);
    IVec2 playerStartTile = player->tilePos;
    IVec2 cloneStartTile = playerStartTile;
    Vector2 playerStart = GetTilePivot(player->tilePos, player->tileSize);
    Vector2 cloneStart = GetTilePivot(clone->tilePos, clone->tileSize);

    BounceEntity(player, bounceDir);
    BounceEntity(clone, -bounceDir);

    
    Vector2 playerEnd = player->attach ?
        GetTilePivot(player->tilePos, player->tileSize, player->attachDir) : GetTilePivot(player->tilePos, player->tileSize);
    
    Vector2 cloneEnd = clone->attach ?
        GetTilePivot(clone->tilePos, clone->tileSize, clone->attachDir) : GetTilePivot(clone->tilePos, clone->tileSize);
    
    if (player->tilePos == clone->tilePos)
    {
        player = MergeSlimes( clone, player);
        return false;
    }

    if (playerStartTile != player->tilePos)
    {
        float dist = Vector2Distance(playerStart, playerEnd);
        float tileDist = dist / MAP_TILE_SIZE;

        TweenParams params = {};
        params.paramType = PARAM_TYPE_VECTOR2;
        params.startVec2 = playerStart;
        params.endVec2 = playerEnd;
        params.realVec2  = &player->pivot;

        AddTween(player->tweenController, CreateTween(params, EaseInOutSine, BOUNCE_SPEED, tileDist));
        OnPlayEvent(&player->tweenController);
    }

    if (cloneStartTile != clone->tilePos)
    {
        float dist = Vector2Distance(cloneStart, cloneEnd);
        float tileDist = dist / MAP_TILE_SIZE;

        TweenParams params = {};
        params.paramType = PARAM_TYPE_VECTOR2;
        params.startVec2 = cloneStart;
        params.endVec2 = cloneEnd;
        params.realVec2  = &clone->pivot;

        AddTween(clone->tweenController, CreateTween(params, EaseInOutSine, BOUNCE_SPEED, tileDist));
        OnPlayEvent(&clone->tweenController);

    }
    return true;
}


inline void DrawSpriteLayer(EntityLayer layer)
{
    auto & entityIndexArray = gameState->entityTable[layer];

    for (int i = 0; i < entityIndexArray.count; i++)
    {
        Entity * entity = GetEntity(entityIndexArray[i]);
        if (entity)
        {
            DrawSprite(gameState->camera, gameState->texture, entity->sprite, entity->pivot, entity->tileSize, entity->color);
        }
    }
}

inline bool SlimeSelection(Entity * player)
{
    auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];

    bool stateChanged = false;

    if (JustPressed(POSSES_KEY))// TODO: UnComment // && gameState->lv2Map && gameState->lv2Map->firstEnter)
    {
        
        Entity * nextPlayerEntity = nullptr;
        for (int i = 0; i < slimeEntityIndices.count; i++)
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

// TODO: Use Tile Bitmasking
void UpdateSprite(EntityLayer layer)
{
    auto & entityIndexArray = gameState->entityTable[layer];
    IVec2 dir[4] = { {-1,0}, {1,0}, {0,-1}, {0,1} };

    for (int i = 0; i < entityIndexArray.count; i++)
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
        
        float moveSpeed = 200.0f;
        
        // NOTE: Camera Move
        if (IsDown(MOUSE_RIGHT))
        {
            if (IsDown(UP_KEY))
            {
                gameState->camera.target.y -= moveSpeed * GetFrameTime();
            }
            if (IsDown(DOWN_KEY))
            {
                gameState->camera.target.y += moveSpeed * GetFrameTime();
                
            }
            if (IsDown(LEFT_KEY))
            {
                gameState->camera.target.x -= moveSpeed * GetFrameTime();
                
            }
            if (IsDown(RIGHT_KEY))
            {
                gameState->camera.target.x += moveSpeed * GetFrameTime();
            }
            
        }
        else
        {
            UpdateCamera();
        }
        
    }

    // NOTE: Actions
    {
        // NOTE: Recored if State Changes
        bool stateChanged = false;

        Entity * player = GetEntity(gameState->playerEntityIndex);
    
        // NOTE: Control Action State
        if (JustPressed(SPLIT_KEY))
        {
            if (player->actionState == SPLIT_STATE)
            {
                player->actionState = MOVE_STATE;
            }
            else if (player->actionState == MOVE_STATE && player->mass > 1)
            {
                player->actionState = SPLIT_STATE;
            }
        }

        UndoState prevState = { gameState->playerEntityIndex, gameState->entities.GetVectorSTD() };

        // NOTE SlimeSelection
        stateChanged = SlimeSelection(player);

        switch(player->actionState)
        {
            case MOVE_STATE:
            {
                // NOTE: read input
                gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = false;
                
                IVec2 actionDir = { 0 };
                
                bool isPressed = false;
                    
                if (IsDown(LEFT_KEY))
                {
                    actionDir = {-1 , 0};                    
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
            case SPLIT_STATE:
            {
                // NOTE: Split Arrow Buttons
                gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = true;

                bool split = false;
                if (JustPressed(LEFT_KEY))
                {
                    // IMPORTANT shoot left and bounce right
                    split = SplitAction(player, { -1, 0 });
                }
                    
                if (JustPressed(RIGHT_KEY))
                {
                    // IMPORTANT shoot right and bounce left
                    split = SplitAction(player, { 1, 0 });
                }
                    
                if (JustPressed(UP_KEY))
                {
                    // IMPORTANT shoot up and bounce down
                    split = SplitAction(player, { 0, -1 });
                }
                    
                if (JustPressed(DOWN_KEY))
                {
                    // IMPORTANT shoot down and bounce up
                    split = SplitAction(player, { 0, 1 });
                }

                if (split)
                {
                    gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = false;
                    stateChanged = stateChanged || split;
                    player->actionState = MOVE_STATE;
                }
                break;
            }
        }
             
        if (gameState->electricDoorSystem)
        {
            UpdateElectricDoor();
        }

        UpdateSlimes();
            
        if (stateChanged)
        {
            undoStack.push_back(prevState);
        }
            
        // NOTE: Undo and Restart
        {
            static bool repeat = false;
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
        // NOTE: Update: Entity
        for (int i = 0; i < gameState->entities.count; i++)
        {
            Entity * entity = GetEntity(i);
            if (entity)
            {
                if (!entity->tweenController.NoTweens())
                {
                    SetActionState(entity, ANIMATE_STATE);
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

        ClearBackground(GRAY);
    
        BeginMode2D(gameState->camera);
    
        for (int i = 0; i < gameState->tileMapCount; i++)
        {
            Map & map = gameState->tileMaps[i];
            DrawTileMap(gameState->camera, map.tilePos, { map.width, map.height }, SKYBLUE, Fade(DARKGRAY, 0.2f));
        }
        

        DrawSpriteLayer(LAYER_BLOCK);
        
        DrawSpriteLayer(LAYER_WALL);

        DrawSpriteLayer(LAYER_CABLE);
        
        DrawSpriteLayer(LAYER_PIT);

        DrawSpriteLayer(LAYER_GLASS);

        DrawSpriteLayer(LAYER_DOOR);
        
        DrawSpriteLayer(LAYER_SLIME);


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

#if GAME_INTERNAL
        // NOTE: UI Draw Game Informations

        Entity * player = GetEntity(gameState->playerEntityIndex);
        IVec2 centerPos = player->tilePos;

        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);

        DrawText(TextFormat("Player pivot (%.2f, %.2f), mouse world (%.2f, %.2f)",
                            player->pivot.x, player->pivot.y, mousePos.x, mousePos.y ), 10, 200, 20, GREEN);
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Entity Count: %i",
                            centerPos.x, centerPos.y,
                            player->mass, gameState->entities.count), 10, 140, 20, GREEN);
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState->camera.target.x, gameState->camera.target.y,
                            gameState->camera.offset.x, gameState->camera.offset.y, gameState->camera.zoom), 10, 50, 20, RAYWHITE);
        DrawText("Arrow Direction to Shoot, R KEY to Restart, Z KEY to undo", 10, 10, 20, RAYWHITE);
        
        DrawText(TextFormat("%.2f ms\n%iFPS", 1000.0f / GetFPS(), GetFPS()), 10, 300, 20, GREEN);
#endif
        
        EndDrawing();
    }    
}

void InitializeGame()
{
    SM_TRACE("Initializing Game");
    // NOTE: Initialization
    gameState->initialized = true;

    InitKeyMapping();

    gameState->tileMaps = (Map *)BumpAllocArray(gameMemory->persistentStorage, 100, sizeof(Map));
        
    LoadLevelToGameState(*gameState);
        
    // NOTE: Arrow Buttons
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

    auto & entityIndexArray = gameState->entityTable[LAYER_SLIME];

    for (int i = 0; i < entityIndexArray.count; i++)
    {
        Entity * slime = GetEntity(entityIndexArray[i]);
        slime->pivot = GetTilePivot(slime);
    }
        
    // NOTE: Initalize undoStack record
    undoStack = std::vector<UndoState>();
    undoStack.push_back({ gameState->playerEntityIndex, gameState->entities.GetVectorSTD() });
    
}

//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================

// Called on every frame
void UpdateAndRender(GameState * gameStateIn, Memory * gameMemoryIn)
{
    
    if (gameState != gameStateIn) gameState = gameStateIn;
    if (gameMemory != gameMemoryIn) gameMemory = gameMemoryIn;

    static int currentScreen = TITLE_SCREEN;

    switch(currentScreen)
    {
        case TITLE_SCREEN:
        {

            if (JustPressed(ANY_KEY))
            {
                currentScreen = GAMEPLAY_SCREEN;
            }
            
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GREEN);

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
        case GAMEPLAY_SCREEN:
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                currentScreen = ENDING_SCREEN;
            }
                                
            if (!gameState->initialized)
            {
                InitializeGame();                    
            }

            GameplayUpdateAndRender();
            break;
        }
        case ENDING_SCREEN:
        {

            if (JustPressed(ANY_KEY))
            {
                currentScreen = TITLE_SCREEN;
            }
            
            BeginDrawing();
            ClearBackground(RAYWHITE);
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
