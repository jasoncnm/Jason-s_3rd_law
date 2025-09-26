/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "game.h"
#include "render_interface.h"
#include "entity.cpp"
#include "electric_door.cpp"
#include "level_loader.cpp"

//  ========================================================================
//              NOTE: Game Constants
//  ========================================================================

//  ========================================================================
//              NOTE: Game Structs
//  ========================================================================


//  ========================================================================
//              NOTE: Game Globals
//  ========================================================================


//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================

inline bool JustPressed(GameInputType type)
{
    KeyMapping & mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx]) || IsMouseButtonPressed(mapping.keys[idx])) return true;
    }
    
    return false;
}

inline bool IsDown(GameInputType type)
{
    KeyMapping mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx]) || IsMouseButtonDown(mapping.keys[idx])) return true;
    }
    return false;
}

inline bool CheckOutOfBound(int tileX, int tileY)
{
    bool result =
    (tileX < gameState->tileMin.x)
        || (tileX > gameState->tileMax.x)
        || (tileY < gameState->tileMin.y)
        || (tileY > gameState->tileMax.y);
    
    return result;
}

inline bool CheckOutOfBound(IVec2 tilePos)
{
    return CheckOutOfBound(tilePos.x, tilePos.y);
}

inline bool CheckBounce(IVec2 tilePos, IVec2 pushDir)
{
    IVec2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

    bool bounce = true;
    
    for (int i = 0; i < 4; i++)
    {
        if (dirs[i] == -pushDir) continue;

        for (int entityIndex = 0; entityIndex < gameState->entities.count; entityIndex++)
        {
            Entity * target = GetEntity(entityIndex);

            if (target->active && target->tilePos == tilePos + dirs[i])
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
    
    IVec2 start = entity->tilePos + dir;
    
    for (IVec2 pos = start;
         ;
         pos = pos + dir)
    {
        for (int i = 0; i < gameState->entities.count; i++)
        {
            Entity * target = GetEntity(i);
            bool isSlime = (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE);

            if (target->active && target->tilePos == pos)
            {
                switch(target->type)
                {
                    case ENTITY_TYPE_ELECTRIC_DOOR:
                    {
                        if (target->cableType == CABLE_TYPE_DOOR && SameSide(target, pos, dir))
                        {
                            // IMPORTANT: entity changed
                            SetEntityPosition(entity, target, pos - dir);
                            return;    
                        }
                        else if (isSlime && target->cableType == CABLE_TYPE_CONNECTION_POINT && target->conductive)
                        {
                            // IMPORTANT:  entity changed
                            SetActionState(entity, FREEZE_STATE);
                            SetEntityPosition(entity, nullptr, pos - dir);
                            return;
                        }
                        break;
                    }
                    case ENTITY_TYPE_WALL:
                    {
                        // IMPORTANT: entity changed
                        if (isSlime)
                        {
                            SetAttach(entity, target, dir);
                        }
                        SetEntityPosition(entity, target, pos - dir);
                        return;
                        SM_ASSERT(false, "WHAT?");
                        
                    }
                    case ENTITY_TYPE_GLASS:
                    {
                        if (isSlime)
                        {
                            // IMPORTANT: entity changed
                            SetAttach(entity, target, dir);
                            SetEntityPosition(entity, target, pos - dir);
                            return;
                        }

                        // IMPORTANT: target changed
                        SetGlassBeBroken(target);
                        break;                        
                    }
                    case ENTITY_TYPE_BLOCK:
                    {
                        // IMPORTANT: entity changed
                        PushActionResult result = 
                            PushActionCheck(entity, entity, pos, dir, entity->mass);

                        if (result.blocked)
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
                            // IMPORTANT: entity changed
                            SetAttach(target, entity, dir);
                            SetEntityPosition(entity, nullptr, pos - dir);
                            entity = target;
                            break;
                        }

                        if (target->type == ENTITY_TYPE_PLAYER)
                        {
                            // IMPORTANT: entity changed
                            entity = MergeSlimes(target, entity);
                        }
                        else
                        {
                            // IMPORTANT: entity changed
                            entity = MergeSlimes(entity, target);
                        }
                                                
                        break;
                    }
                }
                        
                break;
            }
            
        }
        
        if (CheckOutOfBound(pos))
        {
            // IMPORTANT: entity changed
            DeleteEntity(entity);
            return;
        }

    }
}

PushActionResult PushActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int accumulatedMass)
{
    SM_ASSERT(startEntity->movable, "Static entity cannot be pushing blocks!");

    PushActionResult result = { false, false, nullptr };

    for (int i = 0; i < gameState->entities.count; i++)
    {
        Entity * target = GetEntity(i);
        if (target->tilePos == blockNextPos)
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
                    // TODO: fix
                    if (target->cableType == CABLE_TYPE_DOOR) //  && SameSide(target, blockNextPos, pushDir))
                    {
                        result.blocked = true;
                        result.blockedEntity = target;
                        return result;
                    }
                    break;
                }
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
                        MergeSlimes(pushEntity, target);
                        return result;
                    }

                    if (pushEntity->type == ENTITY_TYPE_BLOCK)
                    {
                        SetAttach(target, pushEntity, -pushDir);
                    }
                    
                }
                case ENTITY_TYPE_BLOCK:
                {
                    int newAccumulatedMass = accumulatedMass + target->mass;
                    if (newAccumulatedMass > startEntity->mass)
                    {
                        // (Update, not understand this todo ?) TODO: NOT DONE if entity is slime then attach to wall

                        result.blockedEntity = target;
                        result.blocked = true;
                        return result;
                    }

                    result = PushActionCheck(startEntity, target, blockNextPos + pushDir, pushDir, newAccumulatedMass);
                    
                    if (!result.blocked)
                    {
                        result.pushed = true;
                        if (CheckBounce(target->tilePos, pushDir))
                        {
                            BounceEntity(target, pushDir);
                        }
                        else
                        {
                            // IMPORTANT: target Entity changed
                            SetEntityPosition(target, nullptr, blockNextPos + pushDir);
                        }
                        return result;
                    }

                    result.blockedEntity = target;
                    return result;
                    
                    break;
                }
            }
        }
    }

    // SM_ASSERT(false, "at leat one object is being pushed or blocked");
    return result;
}


inline bool UpdateCameraPosition()
{
    bool updated = false;
    
    for  (int i = 0; i < gameState->tileMapCount; i++)
    {
        Map & map = gameState->tileMaps[i];
        IVec2 playerTile = GetEntity(gameState->playerEntityIndex)->tilePos;
        
        if (playerTile.x > map.tilePos.x && playerTile.x <= (map.tilePos.x + map.width) &&
            playerTile.y > map.tilePos.y && playerTile.y <= (map.tilePos.y + map.height))
        {
            
            Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, map.height * 0.5f + map.tilePos.y + 0.5f);
            if (!Vector2Equals(pos, gameState->camera.target))
            {
                if (!map.firstEnter)
                {
                    map.initEntities = gameState->entities.GetVectorSTD();
                    map.firstEnter = true;
                }
                gameState->currentMapIndex = i;
                gameState->camera.target = pos;

                updated =  true;
            }
            break;
        }
    }
    
    if (IsWindowResized())
    {
        int newWidth = GetScreenWidth();
        int newHeight = GetScreenHeight();
        gameState->camera.offset = { newWidth / 2.0f, newHeight / 2.0f };
        gameState->camera.zoom = (newWidth < newHeight) ? newWidth * 1.75f / SCREEN_WIDTH : newHeight * 1.75 / SCREEN_WIDTH;
        
    }
    
    return updated;
}


inline void SetUndoEntities(std::vector<Entity> & undoEntities)
{
    for (int i = 0; i < undoEntities.size(); i++)
    {
        Entity e = undoEntities[i];
        *GetEntity(e.entityIndex) = e;
    }
}

inline void Undo()
{
    std::vector<Entity> & undoEntities = undoStack.back();
    SetUndoEntities(undoEntities);        
    undoStack.pop_back();

    // animateSlimeCount = 0;
    gameState->animateTime = 0;
    animationPlaying = false;
}


#if 0
void SetRedo(std::vector<UndoEntities> & tmpRedos, UndoEntities redoEntities)
{
    UndoEntities tmp;
    for (int i = 0; i < redoEntities.count; i++)
    {
        tmp.Add(redoEntities[i]);
    }
    tmpRedos.insert(tmpRedos.begin(), tmp);        
}

UndoEntities ApplyRedo(std::vector<UndoEntities> & tmpRedos)
{
    UndoEntities result;
    for (int i = 0; i < tmpRedos.size(); i++)
    {
        UndoEntities & ue = tmpRedos[i];
        for (int j = 0; j < ue.count; j++)
        {
            result.Add(ue[j]);
        }
    }
    return result;
}
#endif


inline void Restart()
{

    undoStack.push_back(gameState->entities.GetVectorSTD());    
    
    Map & currentMap = gameState->tileMaps[gameState->currentMapIndex];
    std::vector<Entity> & initEntities = currentMap.initEntities;

    SetUndoEntities(initEntities);
    
    // animateSlimeCount = 0;
    gameState->animateTime = 0;
    animationPlaying = false;
    
}

bool MoveAction(IVec2 actionDir)
{
    Entity * mother = GetEntity(gameState->playerEntityIndex);

    // TODO: Control Children if their mass same as mother
    if (mother->attach)
    {

        IVec2 currentPos = mother->tilePos;
        IVec2 actionTilePos = currentPos + actionDir;
        if (mother->attachDir == actionDir)
        {
            return false;
        }

        PushActionResult pushResult = PushActionCheck(mother, mother, actionTilePos, actionDir, 0);
        if (pushResult.blocked)
        {
            if (pushResult.blockedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                pushResult.blockedEntity->cableType == CABLE_TYPE_DOOR &&
                !SameSide(pushResult.blockedEntity, actionTilePos, actionDir))
            {
                return false;
            }
            
            if (mother->attachDir == -actionDir)
            {
                // NOTE: Bounce with the block
                PushActionResult result = PushActionCheck(mother, mother, mother->tilePos + mother->attachDir, mother->attachDir, 0);
                if (result.blocked)
                {
                    // IMPORTANT: mother entity changed
                    SetAttach(mother, pushResult.blockedEntity, actionDir);
                    return true;
                }
                BounceEntity(mother, mother->attachDir);
                return true;
            }

            // IMPORTANT: mother entity changed
            SetAttach(mother, pushResult.blockedEntity, actionDir);
            return true;
        }

        if (mother->attachDir == -actionDir)
        {
            if (pushResult.pushed)
            {
                // NOTE: Bounce with the block
                PushActionResult result = PushActionCheck(mother, mother, mother->tilePos + mother->attachDir, mother->attachDir, 0);
                if (!result.blocked)
                {
                    BounceEntity(mother, mother->attachDir);
                }
                return true;
            }
            else
            {
                return false;
            }
        }
                        
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
                SetEntityPosition(mother, findResult.entity, actionTilePos);
            }
            else if (Abs(mother->attachDir) != Abs(actionDir))
            {
                IVec2 newTile = standingPlatformPos;
                IVec2 newAttach = - actionDir;

                Entity * attachedEntity = GetEntity(mother->attachedEntityIndex);

                if (attachedEntity->type == ENTITY_TYPE_ELECTRIC_DOOR &&
                    attachedEntity->cableType == CABLE_TYPE_DOOR &&
                    !SameSide(attachedEntity, newTile, newAttach))
                {
                    return false;
                }
                
                // IMPORTANT: mother entity changed
                SetEntityPosition(mother, attachedEntity, newTile);
            }
            else 
            {
                return false;
            }

            Entity * slime = FindSlime(mother->tilePos);
            if (slime && (slime != mother))
            {
                MergeSlimes(mother, slime);
            }
            
            return true;
        }
        
    }
    return false;
}

// TODO
bool SplitAction(IVec2 bounceDir)
{
    bool stateChanged = false;
#if 0    
    
    Player & player = gameState->entities.dynamicEnt.player;

    IVec2 prevPos = player.slimes[player.motherIndex].tile;

    bool bounce = false;
    IVec2 bouncePos;

    // NOTE: Bounce Position
    if (player.slimes[player.motherIndex].mass > 1)
    {
        
        for ( IVec2 tilePos = player.slimes[player.motherIndex].tile + bounceDir;
              !CheckOutOfBound(tilePos);
              tilePos = tilePos + bounceDir)
        {
            // TODO: check block not implimented
            if (!CheckWalls(tilePos))
            {
                bounce = true;
                bouncePos = tilePos;
                continue;
            }
            break;
        }
    }

    bool split = false;
    IVec2 splitPos;
    
    // NOTE: Split Positions
    if (player.slimes[player.motherIndex].mass > 1)
    {
        IVec2 splitDir = - bounceDir;

        for (IVec2 tilePos = player.slimes[player.motherIndex].tile + splitDir;
             !CheckOutOfBound(tilePos);
             tilePos = tilePos + splitDir)
        {
            // TODO: check block not implimented
            if (!CheckWalls(tilePos))
            {
                split = true;
                splitPos = tilePos;
                continue;
            }
            break;
        }
    }

    stateChanged = split || bounce;
    
    if (stateChanged)
    {
        player.slimes[player.motherIndex].mass--;

        int mass = player.slimes[player.motherIndex].mass;

        if (bounce)
        {
            BounceSlime(player, bouncePos);
        }
        else if (split)
        {
            player.slimes[player.motherIndex].pivot = GetTilePivot(player.slimes[player.motherIndex].tile);            
        }

        if (split)
        {
            SplitSlime(player, splitPos, prevPos);
        }
        else if (bounce)
        {
            SplitSlime(player, prevPos, prevPos);
        }
                
    }
#endif
    return stateChanged;
    
}


inline void DrawSpriteLayer(EntityLayer layer)
{
    auto & entityIndexArray = gameState->entityTable[layer];

    for (int i = 0; i < entityIndexArray.count; i++)
    {
    
        Entity * entity = GetEntity(entityIndexArray[i]);

        if (entity->active)
        {
            Vector2 topleft = GetTilePivot(entity->tilePos, entity->tileSize);
            DrawSprite(texture, entity->sprite, topleft, entity->tileSize);
        }            
    }
}


//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================

void GameUpdateAndRender(GameState * gameStateIn, Memory * gameMemoryIn)
{
    
    if (gameState != gameStateIn) gameState = gameStateIn;
    if (gameMemory != gameMemoryIn) gameMemory = gameMemoryIn;
    
    if (!gameState->initialized)
    {
        // NOTE: Initialization
        gameState->initialized = true;

        // NOTE: Key Mappings
        {
            gameState->keyMappings[MOUSE_LEFT].keys.Add(MOUSE_BUTTON_LEFT);
            gameState->keyMappings[MOUSE_RIGHT].keys.Add(MOUSE_BUTTON_RIGHT);
            gameState->keyMappings[LEFT_KEY].keys.Add(KEY_A);
            gameState->keyMappings[LEFT_KEY].keys.Add(KEY_LEFT);
            gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_D);
            gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_RIGHT);
            gameState->keyMappings[UP_KEY].keys.Add(KEY_W);
            gameState->keyMappings[UP_KEY].keys.Add(KEY_UP);
            gameState->keyMappings[DOWN_KEY].keys.Add(KEY_S);
            gameState->keyMappings[DOWN_KEY].keys.Add(KEY_DOWN);
            gameState->keyMappings[SPACE_KEY].keys.Add(KEY_SPACE);
            
        }

        gameState->tileMaps = (Map *)BumpAllocArray(gameMemory->persistentStorage, 100, sizeof(Map));
        
        LoadLevelToGameState(*gameState, STATE_TEST_LEVEL);
        
        // NOTE: Initalize undoStack record
        undoStack = std::vector<std::vector<Entity>>();
        undoStack.push_back(gameState->entities.GetVectorSTD());
        
        // NOTE: Arrow Buttons
        // UP
        gameState->upArrow.sprite = GetSprite(SPRITE_ARROW_UP);
        gameState->upArrow.id = SPRITE_ARROW_UP;
        
        // DOWN
        gameState->downArrow.sprite = GetSprite(SPRITE_ARROW_DOWN);
        gameState->downArrow.id = SPRITE_ARROW_DOWN;
        
        // LEFT
        gameState->leftArrow.sprite = GetSprite(SPRITE_ARROW_LEFT);
        gameState->leftArrow.id = SPRITE_ARROW_LEFT;
        
        // RIGHT
        gameState->rightArrow.sprite = GetSprite(SPRITE_ARROW_RIGHT);
        gameState->rightArrow.id = SPRITE_ARROW_RIGHT;
        
    }
    
#if 0
    // NOTE: Level Hot Reloading
    for (int i = 0; i < tileMapSources.count; i++)
    {
        char * fileName = tileMapSources[i].fileName;
        long long currentTimeStamp = GetTimestamp(fileName);
        
        if (currentTimeStamp > tileMapSources[i].timestamp)
        {
            LoadLevelToGameState(*gameState, gameState->state);
            undoRecords = std::stack<Record>();
        }
    }
#endif
    
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
        
        if (gameState->camera.zoom > 10.0f) gameState->camera.zoom = 10.0f;
        else if (gameState->camera.zoom < 0.1f) gameState->camera.zoom = 0.1f;
        
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
            UpdateCameraPosition();
        }
        
    }
    
    
    // NOTE: Recored if State Changes
    bool stateChanged = false;

    Entity * player = GetEntity(gameState->playerEntityIndex);
    
    // NOTE: Control Action State
    if (JustPressed(SPACE_KEY))
    {
        if (player->actionState == SPLIT_STATE)
        {
            player->actionState = MOVE_STATE;
        }
        else if (player->actionState == MOVE_STATE)
        {
            player->actionState = SPLIT_STATE;
        }
    }
    
    timeSinceLastPress -= GetFrameTime();

    std::vector<Entity> prevState = gameState->entities.GetVectorSTD();
    
    // NOTE: Actions
    if (!animationPlaying) {
        switch(player->actionState)
        {
            case MOVE_STATE:
            {
                // NOTE: read input
                gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = false;
                
                IVec2 actionDir = { 0 };
                
                if (timeSinceLastPress < 0)
                {
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
                        timeSinceLastPress = pressFreq;
                        stateChanged = MoveAction(actionDir);
                    }
                }
                
                break;
            }
            case SPLIT_STATE:
            {
                // NOTE: Split Arrow Buttons
                gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = true;
                
                if (JustPressed(MOUSE_LEFT))
                {
                    
                    if (gameState->leftArrow.hover)
                    {
                        // IMPORTANT shoot left and bounce right
                        stateChanged = SplitAction({ 1, 0 });
                    }
                    
                    if (gameState->rightArrow.hover)
                    {
                        // IMPORTANT shoot right and bounce left
                        stateChanged = SplitAction({ -1, 0 });
                    }
                    
                    if (gameState->upArrow.hover)
                    {
                        // IMPORTANT shoot up and bounce down
                        stateChanged = SplitAction({ 0, 1 });
                    }
                    
                    if (gameState->downArrow.hover)
                    {
                        // IMPORTANT shoot down and bounce up
                        stateChanged = SplitAction({ 0, -1 });
                    }
                }
                break;
            }
        }

        if (gameState->electricDoorSystem)
        {
            gameState->electricDoorSystem->Update();
        }
        
        if (stateChanged)
        {
            gameState->animateTime = 0;
            undoStack.push_back(prevState);
        }
    }
    else
    {
        // TODO
        // NOTE: Simulate
#if 0
        if (gameState->animateTime <= gameState->duration)
        {
            SM_ASSERT(gameState->duration > 0, "Animation time is zero!");
            
            gameState->animateTime += GetFrameTime();
            
            float x = gameState->animateTime / gameState->duration;
            
            if (x > 1)  x = 1;
            float t = EaseOutSine(x);
            
            for (int i = 0; i < animateSlimeCount; i++)
            {
                SlimeAnimation & sa = animateSlimes[i];
                
                Vector2 startPivot = GetTilePivot(sa.startTilePos.x, sa.startTilePos.y);
                Vector2 endPivot = GetTilePivot(sa.endTilePos.x, sa.endTilePos.y);
                
                sa.currentSlime->pivot = Vector2Lerp(startPivot, endPivot, t);
                
                Merge(entities.dynamicEnt.player, sa);
            }
            
        }
        else
        {
            animateSlimeCount = 0;
            gameState->animateTime = 0;
            animationPlaying = false;
            
            entities.dynamicEnt.player.slimes[entities.dynamicEnt.player.motherIndex].split = false;
            
            int maxIndex = -1;
            int maxMass = 0;
            
            for (int i = 0; i < entities.dynamicEnt.player.slimes.count; i++)
            {
                Slime * child = &entities.dynamicEnt.player.slimes[i];
                child->split = false;
                if (child->mass > maxMass)
                {
                    maxMass = child->mass;
                    maxIndex = i;
                }
            }
            
            SM_ASSERT(maxIndex >= 0, "should not be nullptr");
            
            if (maxMass > entities.dynamicEnt.player.slimes[entities.dynamicEnt.player.motherIndex].mass)
            {
                Posses(entities.dynamicEnt.player, maxIndex);
            }
            
        }
#endif
    }

    // TODO
    // NOTE: Check electric door
#if 0
    {
        if (!entities.dynamicEnt.electricDoorSystem.cables.IsEmpty())
        {
            // NOTE: Update Connection Point before checking connectivity
            
            for (int index = 0; index < entities.dynamicEnt.electricDoorSystem.cables.count; index++)
            {
                Cable & cable = entities.dynamicEnt.electricDoorSystem.cables[index];
                if (cable.type == CABLE_TYPE_CONNECTION_POINT && !cable.conductive)
                {
                    Slime * s = CheckSlime(cable.tile, entities.dynamicEnt.player);
                    bool hasTiles = CheckTiles(cable.tile, entities.dynamicEnt.blocks);
                    bool hasSlime = (s != nullptr);
                    if ( hasTiles || hasSlime )
                    {
                        int indexes[4] = { cable.leftIndex, cable.rightIndex, cable.upIndex, cable.downIndex };
                        for (int i = 0; i < 4; i++)
                        {
                            int id = indexes[i];
                            if (id >= 0 && entities.dynamicEnt.electricDoorSystem.cables[id].conductive)
                            {
                                if (hasSlime) s->state = FREEZE_STATE;
                                cable.conductive = true;
                                
                                std::vector<bool> visited(entities.dynamicEnt.electricDoorSystem.cables.count);
                                OnSourcePowerOn(visited, cable.sourceIndex);
                                
                                goto End;
                            } 
                        }
                    }
                }
            }
            
            for (int index = 0; index < entities.dynamicEnt.electricDoorSystem.sourceIndexes.count; index++)
            {
                int id = entities.dynamicEnt.electricDoorSystem.sourceIndexes[index];
                
                Cable & source = entities.dynamicEnt.electricDoorSystem.cables[id];
                for (int i = 0; i < entities.dynamicEnt.blocks.count; i++)
                {
                    if (entities.dynamicEnt.blocks[i].tile == source.tile)
                    {
                        std::vector<bool> visited(entities.dynamicEnt.electricDoorSystem.cables.count);
                        for (int i = 0; i < entities.dynamicEnt.electricDoorSystem.cables.count; i++)
                        {
                            visited[i] = false;   
                        }
                        
                        if (!source.conductive)
                        {
                            OnSourcePowerOn(visited, id);
                        }
                        
                        break;
                    }
                }
            }
        }
    } End:;

#endif

    // NOTE: Undo and Restart
    {
        if (timeSinceLastPress < 0 && IsKeyDown(KEY_Z) && !undoStack.empty())
        {
            // NOTE: Undo
            Undo();
                        
            timeSinceLastPress = pressFreq;

        }
#if 1
        // NOTE: Restart States
        repeat &= !stateChanged;
        if (IsKeyPressed(KEY_R) && !repeat)
        {
            repeat = true;
            Restart();
            
        }
#endif
    }
    
    
    // NOTE: Arrow Setup
    {
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
        IVec2 centerPos = player->tilePos;
        
        Vector2 upPos    = { (float)centerPos.x, (float)(centerPos.y-1) };
        Vector2 downPos  = { (float)centerPos.x, (float)(centerPos.y+1) };
        Vector2 leftPos  = { (float)(centerPos.x-1), (float)(centerPos.y) };
        Vector2 rightPos = { (float)(centerPos.x+1), (float)(centerPos.y) };
        
        Vector2 dir[4] = { upPos, downPos, leftPos, rightPos };
        ArrowButton * arrows[4] = { &gameState->upArrow, &gameState->downArrow, &gameState->leftArrow, &gameState->rightArrow };
        
        for (int i = 0; i < 4; i++)
        {
            Vector2 tilePos = dir[i];
            
            ArrowButton * arrow = arrows[i];
            
            Vector2 pos = TilePositionToPixelPosition(tilePos.x, tilePos.y);
            Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));
            
            
            Rectangle rect = { (float)topLeft.x, (float)topLeft.y, MAP_TILE_SIZE, MAP_TILE_SIZE };
            if (CheckCollisionPointRec(mousePos, rect))
            {
                arrow->hover = true;
                arrow->sprite = GetSprite(GetArrowHoverSpriteID(arrow->id));
            }
            else
            {
                arrow->hover = false;
                arrow->sprite = GetSprite(arrow->id);
            }
            
            arrow->topLeftPos = topLeft;
            
        }
    }

    // NOTE: Render Step
    {
    
        // NOTE: Draw
        BeginDrawing();

        ClearBackground(GRAY);
    
        BeginMode2D(gameState->camera);
    
        for (int i = 0; i < gameState->tileMapCount; i++)
        {
            Map & map = gameState->tileMaps[i];
        
            DrawTileMap(map.tilePos, { map.width, map.height }, BLUE, Fade(DARKBLUE, 0.5f));
        }

        DrawSpriteLayer(LAYER_GROUND);
        
        DrawSpriteLayer(LAYER_OVERLAP);

        DrawSpriteLayer(LAYER_BLOCKS);
        
        DrawSpriteLayer(LAYER_PLAYER);

        for (int i = 0; i < gameState->slimeEntityIndices.count; i++)
        {
            Entity * entity = GetEntity(gameState->slimeEntityIndices[i]);
            if (entity->active && entity->attach)
            {
                IVec2 t = entity->tilePos + entity->attachDir;                        
                Vector2 pos = TilePositionToPixelPosition((float)t.x, (float)t.y);
                DrawCircleV(pos, 5.0f, RED);                              // Draw a color-filled circle
            }
        }                

    
        if (!animationPlaying)
        {
            // NOTE: Arrow sprite
            // Left
            if (gameState->leftArrow.show)
            {
                DrawSprite(texture, gameState->leftArrow.sprite, gameState->leftArrow.topLeftPos);
            }
        
            // Right
            if (gameState->rightArrow.show)
            {
                DrawSprite(texture, gameState->rightArrow.sprite, gameState->rightArrow.topLeftPos);
            }
        
            // Up
            if (gameState->upArrow.show)
            {
                DrawSprite(texture, gameState->upArrow.sprite, gameState->upArrow.topLeftPos);
            }
        
            // Down
            if (gameState->downArrow.show)
            {
                DrawSprite(texture, gameState->downArrow.sprite, gameState->downArrow.topLeftPos);
            }
        }
    
        EndMode2D();
    
        // NOTE: UI Draw Game Informations
    
        IVec2 centerPos = player->tilePos;
    
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
        IVec2 mouseTilePos = PixelPositionToTilePosition(mousePos.x, mousePos.y);
    
        DrawText(TextFormat("Player pivot (%.2f, %.2f), Mouse World Position (%.2f, %.2f)",
                            player->pivot.x, player->pivot.y,
                            mousePos.x, mousePos.y), 10, 200, 20, GREEN);
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Entity Count: %i, Mouse Tile (%i, %i)",
                            centerPos.x, centerPos.y,
                            player->mass, gameState->entities.count,
                            mouseTilePos.x, mouseTilePos.y), 10, 140, 20, GREEN);
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState->camera.target.x, gameState->camera.target.y,
                            gameState->camera.offset.x, gameState->camera.offset.y, gameState->camera.zoom), 10, 50, 20, RAYWHITE);
        DrawText("Arrow Direction to Shoot, R KEY to Restart, Z KEY to undo", 10, 10, 20, RAYWHITE);
        DrawFPS(10, 300);

        
        EndDrawing();

    }
}
