
#include "game.h"
#include "game_util.h"
#include "render_interface.h"
#include "action_input.h"

#include "entity.cpp"
#include "electric_door.cpp"
#include "level_loader.cpp"
#include "move_animation.cpp"
#include "animation_controller.cpp"

/*
TODO: Things that I can do beside arts and design I guess
  - BUGS:
  - Change animation controller into a tweening controller that is able to tween arbitarty types of values using easing functions
  - Basic Scene Manager
  - smooth pixelperfect transition
  - Viewport scaling
  - top down lights / spotlight rendering
  - add particles
  - background effects (try this: https://github.com/raysan5/raylib/blob/master/examples/shapes/shapes_starfield_effect.c)
  - Texture filtering when zooming out (is mipmapping come handy here?)

NOTE: done
  - Gamepad supports
  - (MoveActionCheck) When Door and block are in the same tile, we should check if the door is blocked first, then check if we can push the block

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
                        if (door)
                        {
                            if (DoorBlocked(door, -pushDir))
                            {
                                result.pushed = false;
                                result.blocked = true;
                                result.blockedEntity = target;
                                
                                return result;
                            }
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
                                AddAnimation(target->aniController, GetMoveAnimation(nullptr, moveStart, moveEnd, BOUNCE_SPEED, tileDist));
                        
                                if ((startTile - pushEntity->tilePos).SqrMagnitude() > 1)
                                {
                                    pushEntity->aniController.endEvent.controller = &target->aniController;
                                    pushEntity->aniController.endEvent.OnPlayFunc = OnPlayEvent;
                                }
                                else
                                {
                                    OnPlayEvent(&target->aniController);
                                }
                                                    
                                if (!target->active)
                                {
                                    target->active = true;
                                    target->aniController.endEvent.deleteEntity = target;
                                    target->aniController.endEvent.OnDeleteFunc = DeleteEntity;
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

                                AddAnimation(target->aniController, GetMoveAnimation(nullptr, moveStart, moveEnd, BOUNCE_SPEED, iDist));

                                if ((startTile - pushEntity->tilePos).SqrMagnitude() > 1)
                                {
                                    pushEntity->aniController.endEvent.controller = &target->aniController;
                                    pushEntity->aniController.endEvent.OnPlayFunc = OnPlayEvent;
                                }
                                else
                                {
                                    OnPlayEvent(&target->aniController);
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

inline bool InstancePush(Vector2 pushStart, Vector2 pushedStart)
{
    float dist = Vector2Distance(pushStart, pushedStart);
    if (dist > MAP_TILE_SIZE) return true;
    return false;
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
                    gameState->camera.target = pos;                    
                }
                
                if (gameState->currentMapIndex != i)
                {
                    gameState->cameraAniController.Reset();
                    AddAnimation(gameState->cameraAniController, GetMoveAnimation(EaseOutCubic, gameState->camera.target, pos, 1.7f));
                    OnPlayEvent(&gameState->cameraAniController);
                    gameState->currentMapIndex = i;
                }
                
                updated = true;
            }
            break;
        }
    }
    
    if (updated || IsWindowResized())
    {
        float oldZoom = gameState->camera.zoom;
        
        int newWidth = GetScreenWidth();
        int newHeight = GetScreenHeight();
        Map & currentMap = gameState->tileMaps[gameState->currentMapIndex];
        int mapMax = (currentMap.width > currentMap.height) ? currentMap.width : currentMap.height;
        gameState->camera.zoom = (zoom_per_tile / mapMax);
        (newWidth < newHeight) ? gameState->camera.zoom *= newWidth : gameState->camera.zoom *= newHeight;         
        gameState->camera.offset = { newWidth / 2.0f, newHeight / 2.0f };
    }

    if (!gameState->cameraAniController.moveAnimationQueue.IsEmpty())
    {
        gameState->cameraAniController.Update();
        gameState->camera.target = gameState->cameraAniController.currentPosition;        
    }
    
    return updated;
}


inline void SetUndoEntities(std::vector<Entity> & undoEntities)
{
    for (int i = 0; i < undoEntities.size(); i++)
    {
        Entity e = undoEntities[i];
        gameState->entities[i] = e;
        gameState->entities[i].aniController.Reset();

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

    float moveSpeed = 4.0f;

    if (mother->attach)
    {

        IVec2 currentPos = mother->tilePos;
        IVec2 actionTilePos = currentPos + actionDir;
        if (mother->attachDir == actionDir)
        {
            return false;
        }

#if 1
        Entity * door = FindEntityByLocationAndLayer(currentPos, LAYER_DOOR);
        if (door)
        {
            if (DoorBlocked(door, -actionDir))
            {
                return false;
            }
        }
#endif
        
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

                    AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, startPos, endPos, moveSpeed));
                    OnPlayEvent(&mother->aniController);
                    
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

            AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, moveStart, moveMiddle, moveSpeed * 2));
            AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, moveMiddle, moveEnd, moveSpeed * 2));
            OnPlayEvent(&mother->aniController);
            
            return true;
        }

        if (mother->attachDir == -actionDir)
        {
            if (moveResult.pushed)
            {
                // NOTE: Bounce with the block
                MoveActionResult result = MoveActionCheck(mother, mother, mother->tilePos + mother->attachDir, mother->attachDir, 0);
                if (!result.blocked)
                {
                    // BounceEntity(mother, mother->attachDir);
                }
                return true;
            }
            else
            {
                return false;
            }
        }

#if 0
        for (int i = 0; i < CP_Indices.count; i++)
        {
            Entity * connection = GetEntity(Cable_Indices[CP_Indices[i]]);
            SM_ASSERT(connection, "Entity is not active");
        
            if (connection->tilePos == actionTilePos && CanFreezeSlime(connection))
            {
                SetActionState(mother, FREEZE_STATE);
                connection->conductive = true;
                Array<bool, MAX_CABLE> visited;
                for (int i = 0; i < Cable_Indices.count; i++) visited.Add(false);
                OnSourcePowerOn(visited, connection->sourceIndex);
                UpdateElectricDoor();

                break;
            }
        }

        if (mother->actionState == FREEZE_STATE)
        {
            FindAttachableResult result = FindAttachable(actionTilePos, mother->attachDir);
            if (result.has)
            {
                SetEntityPosition(mother, result.entity, actionTilePos);
            }
            else
            {
                SetEntityPosition(mother, nullptr, actionTilePos);
            }
            return true;
        }
#endif
        if (!mother->active) return false;
        
        // NOTE: no obsticale, move player
        {
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
                AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, moveStart, moveEnd, moveSpeed));
                OnPlayEvent(&mother->aniController);
                
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

                    AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, moveStart, movemiddle, moveSpeed * 1.5f));
                    AddAnimation(mother->aniController, GetMoveAnimation(EaseOutCubic, movemiddle, moveEnd, moveSpeed * 1.5f));

                    OnPlayEvent(&mother->aniController);                

                }
                else 
                {
                    return false;
                }

            }
            
            return true;
        }
        
    }
    return false;
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

        AddAnimation(player->aniController, GetMoveAnimation(EaseInOutSine, playerStart, playerEnd, BOUNCE_SPEED, tileDist));
        OnPlayEvent(&player->aniController);
    }

    if (cloneStartTile != clone->tilePos)
    {
        float dist = Vector2Distance(cloneStart, cloneEnd);
        float tileDist = dist / MAP_TILE_SIZE;

        AddAnimation(clone->aniController, GetMoveAnimation(EaseInOutSine, cloneStart, cloneEnd, BOUNCE_SPEED, tileDist));
        OnPlayEvent(&clone->aniController);

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

//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================

// Called on every frame
void UpdateAndRender(GameState * gameStateIn, Memory * gameMemoryIn)
{
    
    if (gameState != gameStateIn) gameState = gameStateIn;
    if (gameMemory != gameMemoryIn) gameMemory = gameMemoryIn;
    
    if (!gameState->initialized)
    {
        // NOTE: Initialization
        gameState->initialized = true;

        InitKeyMapping();

        gameState->tileMaps = (Map *)BumpAllocArray(gameMemory->persistentStorage, 100, sizeof(Map));
        
        LoadLevelToGameState(*gameState, STATE_TEST_LEVEL);
        
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

#if 0        
        UpdateSprite(LAYER_WALL);
        UpdateSprite(LAYER_GLASS);
#endif
        
        // NOTE: Initalize undoStack record
        undoStack = std::vector<UndoState>();
        undoStack.push_back({ gameState->playerEntityIndex, gameState->entities.GetVectorSTD() });
        
    }
    
#if 0
    // TODO:  Level Hot Reloading
    for (int i = 0; i < tileMapSources.count; i++)
    {
        char * fileName = tileMapSources[i].fileName;
        long long currentTimeStamp = GetTimestamp(fileName);
        
        if (currentTimeStamp > tileMapSources[i].timestamp)
        {
            Array<Entity, 3> slimes;
            for (int i = 0; i < slimeEntityIndices.count; i++)
            {
                slimes[i] = *GetEntity(slimeEntityIndices[i]);
            }
            
            LoadLevelToGameState(*gameState, gameState->state);
            undoRecords = std::stack<Record>();
        }
    }
#endif

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

        UpdateSlimes();                        
        if (gameState->electricDoorSystem)
        {
            UpdateElectricDoor();
        }
        
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

    // NOTE: Update: Entity
    for (int i = 0; i < gameState->entities.count; i++)
    {
        Entity * entity = GetEntity(i);
        if (entity)
        {
            if (!entity->aniController.moveAnimationQueue.IsEmpty())
            {
                SetActionState(entity, ANIMATE_STATE);
                entity->aniController.Update();
                entity->pivot = entity->aniController.currentPosition;
            }
            else
            {
                if (entity->actionState == ANIMATE_STATE) SetActionState(entity, MOVE_STATE);
                entity->pivot = GetTilePivot(entity);
                entity->aniController.HandleAnimationNotPlaying();
            }
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
