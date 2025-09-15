/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "Game.h"
#include "level_loader.h"
#include "render_interface.h"

//  ========================================================================
//              NOTE: Game Constants
//  ========================================================================

//  ========================================================================
//              NOTE: Game Structs
//  ========================================================================


//  ========================================================================
//              NOTE: Game Globals
//  ========================================================================
static GameState startState;
static std::stack<Record> undoRecords;


//  ========================================================================
//              NOTE: Game Functions (internal)
//  ========================================================================

bool JustPressed(GameInputType type)
{
    KeyMapping & mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx]) || IsMouseButtonPressed(mapping.keys[idx])) return true;
    }

    return false;
}

bool IsDown(GameInputType type)
{
    KeyMapping mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx]) || IsMouseButtonDown(mapping.keys[idx])) return true;
    }
    return false;
}



bool SlimeAction(IVec2 bounceDir)
{
    
    bool stateChanged = false;

    Record & record = gameState->currentRecord;
    
    Player & player = record.player;

    IVec2 prevPos = player.mother.tile;

    bool bounce = false;
    IVec2 bouncePos;

    // NOTE: Bounce Position
    if (player.mother.mass > 1)
    {
        
        for ( IVec2 tilePos = player.mother.tile + bounceDir;
              !CheckOutOfBound(tilePos);
              tilePos = tilePos + bounceDir)
        {
            // TODO: check block not implimented
            if (!record.CheckWalls(tilePos))
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
    if (player.mother.mass > 1)
    {
        IVec2 splitDir = - bounceDir;

        for (IVec2 tilePos = player.mother.tile + splitDir;
             !CheckOutOfBound(tilePos);
             tilePos = tilePos + splitDir)
        {
            // TODO: check block not implimented
            if (!record.CheckWalls(tilePos))
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
        player.mother.mass--;

        int mass = player.mother.mass;

        if (bounce)
        {
            BounceSlime(player, bouncePos);
        }
        else if (split)
        {
            player.mother.pivot = GetTilePivot(player.mother.tile);            
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

    return stateChanged;
    
}

//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================

void GameUpdate(GameState * gameStateIn)
{

    if (gameState != gameStateIn) gameState = gameStateIn;
    
    if (!gameState->initialized)
    {
        // NOTE: Initialization
        startState.initialized = true;

        // NOTE: Key Mappings
        {
            startState.keyMappings[MOUSE_LEFT].keys.Add(MOUSE_BUTTON_LEFT);
            startState.keyMappings[MOUSE_RIGHT].keys.Add(MOUSE_BUTTON_RIGHT);

            startState.keyMappings[LEFT_KEY].keys.Add(KEY_A);
            startState.keyMappings[LEFT_KEY].keys.Add(KEY_LEFT);
            startState.keyMappings[RIGHT_KEY].keys.Add(KEY_D);
            startState.keyMappings[RIGHT_KEY].keys.Add(KEY_RIGHT);
            startState.keyMappings[UP_KEY].keys.Add(KEY_W);
            startState.keyMappings[UP_KEY].keys.Add(KEY_UP);
            startState.keyMappings[DOWN_KEY].keys.Add(KEY_S);
            startState.keyMappings[DOWN_KEY].keys.Add(KEY_DOWN);
            startState.keyMappings[SPACE_KEY].keys.Add(KEY_SPACE);

        }
    
        LoadLevelToGameState(startState, STATE_TEST_LEVEL);

        // NOTE: Initalize undo record
        undoRecords = std::stack<Record>();

        // NOTE: Arrow Buttons
        // UP
        startState.upArrow.sprite = GetSprite(SPRITE_ARROW_UP);
        startState.upArrow.id = SPRITE_ARROW_UP;

        // DOWN
        startState.downArrow.sprite = GetSprite(SPRITE_ARROW_DOWN);
        startState.downArrow.id = SPRITE_ARROW_DOWN;

        // LEFT
        startState.leftArrow.sprite = GetSprite(SPRITE_ARROW_LEFT);
        startState.leftArrow.id = SPRITE_ARROW_LEFT;

        // RIGHT
        startState.rightArrow.sprite = GetSprite(SPRITE_ARROW_RIGHT);
        startState.rightArrow.id = SPRITE_ARROW_RIGHT;
        
        *gameState = startState;

        undoRecords.push(gameState->currentRecord);
    }

    // NOTE: Level Hot Reloading
    {
        long long currentTimeStamp = GetTimestamp(LEVELS_PATH);

        if (currentTimeStamp > levelsTimestamp)
        {
            LoadLevelToGameState(startState, startState.state);
            *gameState = startState;
            undoRecords = std::stack<Record>();
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

            return;
        }
        else
        {
            if (UpdateCameraPosition())
            {
                startState = *gameState;
            }
            
        }

    }

            
    // NOTE: Recored if State Changes
    bool stateChanged = false;

    Record prevRecord = gameState->currentRecord;

    Record & record = gameState->currentRecord;

    // NOTE: Control Action State
    if (JustPressed(SPACE_KEY))
    {
        if (record.player.mother.state == SPLIT_STATE) record.player.mother.state = MOVE_STATE;
        else if (record.player.mother.state == MOVE_STATE) record.player.mother.state = SPLIT_STATE;
    }
    
    // NOTE: Actions
    if (!animationPlaying) {
        switch(record.player.mother.state)
        {
            case MOVE_STATE:
            {
                // NOTE: read input
                gameState->upArrow.show = gameState->downArrow.show = gameState->leftArrow.show = gameState->rightArrow.show = false;
                
                IVec2 actionDir = { 0 };
            
                timeSinceLastPress -= GetFrameTime();

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

                    if (isPressed) timeSinceLastPress = pressFreq;

                    stateChanged = isPressed;
                }

                if (stateChanged)
                {
                    Slime & mother =  record.player.mother;
                    // TODO: Control Children if their mass same as mother
                    if (mother.attach)
                    {

                        IVec2 currentPos = mother.tile;
                        
                        IVec2 actionTilePos = currentPos + actionDir;

                        if (mother.attachDir == actionDir)
                        {
                            stateChanged = false;
                            break;
                        }

                        bool pushed = false;
 
                        if (!CheckPushableBlocks(mother, actionTilePos, actionDir, 0, pushed))
                        {
                            if (mother.attachDir == -actionDir)
                            {
                                int index = GetTileIndex(mother.tile + mother.attachDir, record.blocks);
                                if (index != -1)
                                {
                                    Block & attachedBlock = record.blocks[index];
                                    // TODO: Bounce with the block
                                    if (!CheckPushableBlocks(mother, mother.tile + mother.attachDir, mother.attachDir, 0, pushed))
                                    {
                                        mother.attachDir = actionDir;
                                        break;
                                    } 

                                    SetSlimePosition(mother, attachedBlock.tile - mother.attachDir);
                                    break;
                                }
                            }

                            mother.attachDir = actionDir;

                            break;
                        }
                        
                        if (mother.attachDir == -actionDir)
                        {
                            int index = GetTileIndex(currentPos + mother.attachDir, record.blocks);
                            if (pushed && (index != -1))
                            {
                                // TODO: Bounce with the block
                                CheckPushableBlocks(mother, mother.tile + mother.attachDir, mother.attachDir, 0, pushed);

                                SetSlimePosition(mother, record.blocks[index].tile - mother.attachDir);
                                break;
                            }
                        }
                        // NOTE: no obsticale, move player
                        {
                            IVec2 standingPlatformPos = actionTilePos + mother.attachDir;
                            if (record.CheckWalls(standingPlatformPos) ||
                                CheckTiles(standingPlatformPos, record.blocks))
                            {
                                SetSlimePosition(mother, {actionTilePos.x, actionTilePos.y});
                            }
                            else if (Abs(mother.attachDir) != Abs(actionDir))
                            {
                                SetSlimePosition(mother, {standingPlatformPos.x, standingPlatformPos.y});

                                mother.attachDir = -actionDir;
                            }
                            else 
                            {
                                stateChanged = false || pushed;
                            }
                        }
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
                        stateChanged = SlimeAction({ 1, 0 });
                    }
                        
                    if (gameState->rightArrow.hover)
                    {
                        // IMPORTANT shoot right and bounce left
                        stateChanged = SlimeAction({ -1, 0 });
                    }
                        
                    if (gameState->upArrow.hover)
                    {
                        // IMPORTANT shoot up and bounce down
                        stateChanged = SlimeAction({ 0, 1 });
                    }
                        
                    if (gameState->downArrow.hover)
                    {
                        // IMPORTANT shoot down and bounce up
                        stateChanged = SlimeAction({ 0, -1 });
                    }
                }
                break;
            }
        }

        if (stateChanged)
        {
            gameState->animateTime = 0;
            undoRecords.push(prevRecord);
        }
 
    }
    else
    {
        // NOTE: Simulate

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

                Merge(record.player, sa);
            }
                    
        }
        else
        {
            animateSlimeCount = 0;
            gameState->animateTime = 0;
            animationPlaying = false;

            record.player.mother.split = false;

            Slime * max = nullptr;
            int maxMass = 0;
                    
            for (int i = 0; i < record.player.children.count; i++)
            {
                Slime * child = &record.player.children[i];
                child->split = false;
                if (child->mass > maxMass)
                {
                    maxMass = child->mass;
                    max = child;
                }
            }

            SM_ASSERT(max, "should not be nullptr");

            if (maxMass > record.player.mother.mass)
            {
                Posses(&record.player.mother, max);
            }
                   
        }

    }

    // NOTE: Check electric door
    {
        if (!record.electricDoorSystem.cables.IsEmpty())
        {
            bool change = false;
            // NOTE: Update Connection Point before checking connectivity
            
            for (int index = 0; index < record.electricDoorSystem.cables.count; index++)
            {
                Cable & cable = record.electricDoorSystem.cables[index];
                if (cable.type == CABLE_TYPE_CONNECTION_POINT && !cable.conductive)
                {
                    Slime * s = CheckSlime(cable.tile, record.player);
                    bool hasTiles = CheckTiles(cable.tile, record.blocks);
                    bool hasSlime = (s != nullptr);
                    if ( hasTiles || hasSlime )
                    {
                        int indexes[4] = { cable.leftIndex, cable.rightIndex, cable.upIndex, cable.downIndex };
                        for (int i = 0; i < 4; i++)
                        {
                            int id = indexes[i];
                            if (id >= 0 && record.electricDoorSystem.cables[id].conductive)
                            {
                                if (hasSlime) s->state = FREEZE_STATE;
                                cable.conductive = true;

                                std::vector<bool> visited(record.electricDoorSystem.cables.count);
                                OnSourcePowerOn(visited, cable.sourceIndex);

                                goto End;
                            } 
                        }
                    }
                }
            }
            
            for (int index = 0; index < record.electricDoorSystem.sourceIndexes.count; index++)
            {
                int id = record.electricDoorSystem.sourceIndexes[index];
            
                Cable & source = record.electricDoorSystem.cables[id];
                for (int i = 0; i < record.blocks.count; i++)
                {
                    if (record.blocks[i].tile == source.tile)
                    {
                        std::vector<bool> visited(record.electricDoorSystem.cables.count);
                        for (int i = 0; i < record.electricDoorSystem.cables.count; i++)
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
            
    // NOTE: Undo and Restart
    {
        if (IsKeyPressed(KEY_Z) && !undoRecords.empty())
        {
            // NOTE: Undo                
            gameState->currentRecord = undoRecords.top();
            undoRecords.pop();

            animateSlimeCount = 0;
            gameState->animateTime = 0;
            animationPlaying = false;
        }

        // NOTE: Restart States        
        if (IsKeyPressed(KEY_R))
        {
            // TODO: need to reset the tileMap state at the tilemap where the player located

            undoRecords.push(gameState->currentRecord);
            gameState->currentRecord = startState.currentRecord;

            animateSlimeCount = 0;
            gameState->animateTime = 0;
            animationPlaying = false;

        }
    }


    // NOTE: Arrow Setup
    {
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
        IVec2 centerPos = record.player.mother.tile;
        
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
    
}

void GameRender(GameState * gameState)
{

    BeginMode2D(gameState->camera);

    for (int i = 0; i < gameState->tileMaps.count; i++)
    {
        Map & map = gameState->tileMaps[i];
        
        DrawTileMap(map.tilePos, { map.width, map.height }, BLUE, Fade(DARKBLUE, 0.5f));
    }
    
    Record & record = gameState->currentRecord;

    // NOTE: Draw Walls
    for (unsigned int index = 0; index < record.walls.count; index++)
    {
        Wall & wall = record.walls[index];

        Vector2 wallTopLeftPos = GetTilePivot(wall.tile);
        DrawSprite(texture, wall.sprite, wallTopLeftPos);
    }

    // NOTE: Electric Door
    {
        for (int i = 0; i < record.electricDoorSystem.cables.count; i++)
        {
            Cable & cable = record.electricDoorSystem.cables[i];
            DrawSprite(texture, cable.sprite, GetTilePivot(cable.tile));
        }
    }
            
    // NOTE: Draw Blocks
    for (unsigned int index = 0; index < record.blocks.count; index++)
    {
        Block & block = record.blocks[index];

        Vector2 blockTopLeftPos = GetTilePivot(block.tile);

        DrawSprite(texture, block.sprite, blockTopLeftPos);
        // DrawRectangleV(blockTopLeftPos, {  (float)record.blockSize,  (float)record.blockSize }, block.color);
    }

    // NOTE: Draw Glasses
    for (int i = 0; i < record.glasses.count; i++)
    {
        Glass & g = record.glasses[i];
        
        Vector2 topLeft = GetTilePivot(g.tile);
        DrawSprite(texture, g.sprite, topLeft);

    }

    // NOTE: Draw player
    Color pColor = record.player.color;
    // NOTE: Test
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera);
    {

        Record & record = gameState->currentRecord;

        Rectangle playerRect = {
            (float)record.player.mother.pivot.x,
            (float)record.player.mother.pivot.y,
            MAP_TILE_SIZE, MAP_TILE_SIZE};

        if (CheckCollisionPointRec(mousePos, playerRect))
        {
            pColor = RED;
        }
    }
    //DrawRectangleV(record.player.mother.pivot, { (float)record.player.mother.tileSize, (float)record.player.mother.tileSize}, pColor);
    DrawSprite(texture, record.player.mother.sprite, record.player.mother.pivot, pColor);

    if (record.player.mother.attach)
    {
        IVec2 t = record.player.mother.tile + record.player.mother.attachDir;
    
        Vector2 pos = TilePositionToPixelPosition((float)t.x, (float)t.y);

        DrawCircleV(pos, 5.0f, RED);                              // Draw a color-filled circle
    }

    // NOTE: Draw children
    for (int i = 0; i < record.player.children.count; i++)
    {
        Slime child = record.player.children[i];
        //DrawRectangleV(child.pivot, { (float)child.tileSize, (float)child.tileSize}, GREEN);
        DrawSprite(texture, child.sprite, child.pivot);
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

    IVec2 centerPos = record.player.mother.tile;


    DrawText(TextFormat("Player pivot (%.2f, %.2f), Mouse World Position (%.2f, %.2f)",
                        record.player.mother.pivot.x, record.player.mother.pivot.y,
                        mousePos.x, mousePos.y), 10, 200, 20, GREEN);
    DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Child Count: %i",
                        centerPos.x, centerPos.y,
                        record.player.mother.mass, record.player.children.count), 10, 140, 20, GREEN);
    DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                        gameState->camera.target.x, gameState->camera.target.y,
                        gameState->camera.offset.x, gameState->camera.offset.y, gameState->camera.zoom), 10, 50, 20, RAYWHITE);
    DrawText("Arrow Direction to Shoot, R KEY to Restart, Z KEY to undo", 10, 10, 20, RAYWHITE);

}
