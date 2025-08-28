/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "Game.h"
#include "assets.h"

#include <stdlib.h>                 // Required for: calloc(), free()
#include <cassert>
#include <stack>   
#include <sstream>
#include <iostream>


bool JustPressed(GameState & gameState, GameInputType type)
{
    KeyMapping mapping = gameState.keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx]) || IsMouseButtonPressed(mapping.keys[idx])) return true;
    }

    return false;
}

bool IsDown(GameState & gameState, GameInputType type)
{
    KeyMapping mapping = gameState.keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx]) || IsMouseButtonDown(mapping.keys[idx])) return true;
    }
    return false;
}

Vector2 TilePositionToPixelPosition(float tileX, float tileY, unsigned int playerTileSize)
{
    Vector2 result;
    result.x = (float)tileX * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;
    result.y = (float)tileY * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;

    return result;
}

IVec2 PixelPositionToTilePosition(float x, float y)
{
    IVec2 result;
    // result.x = (float)tileX * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;
    result.x = (int)(x / MAP_TILE_SIZE+1);

    // result.y = (float)tileY * MAP_TILE_SIZE - (MAP_TILE_SIZE) / 2.0f;
    result.y = (int)(y / MAP_TILE_SIZE+1);

    return result;

}


float GetTileSize(int maxTileSize, int mass, int maxMass)
{
    SM_ASSERT(mass <= maxMass, "Max mass is less than current mass");
    
    return maxTileSize * mass / maxMass; 
}

Vector2 GetTilePivot(int tileX, int tileY, int tileSize)
{
    Vector2 playerPos = TilePositionToPixelPosition((float)tileX, (float)tileY, tileSize);            

    return Vector2Subtract(playerPos, Vector2Scale(Vector2One(), (float)tileSize * 0.5f));
}


bool CheckOutOfBound(unsigned int tileX, unsigned int tileY, Map & tileMap)
{
    bool result =
           (tileX < 1)
        || (tileX > tileMap.tilesX)
        || (tileY < 1)
        || (tileY > tileMap.tilesY);

    return result;
}

bool CheckWalls(unsigned int tileX, unsigned int tileY, unsigned int wallCount, Wall * walls)
{
    bool result = false;

    for (unsigned int index = 0; index < wallCount; index++)
    {
        Wall & wall = walls[index];
        if (tileX == wall.wallX && tileY == wall.wallY)
        {
            result = true;
            break;
        }
    }

    return result;
}

SlimeAnimation CreateSlimeAnimation(Slime * refSlime,
                                    float maxTileSize,
                                    int startX, int startY,
                                    int endX, int endY,
                                    int startMass, int endMass)
{

    animationPlaying = true;

    SlimeAnimation result = { 0 };
    result.startTilePos = { startX, startY };
    result.endTilePos = {  endX, endY };
    result.currentSlime = refSlime;
    
    return result;
    
}

void BounceSlime(Player & player, int destX, int destY)
{
    
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to bounce");
    
    bool merge = false;
    int mergedIndex = -1;

    int prevX = player.mother.tileX;
    int prevY = player.mother.tileY;
          
    player.mother.tileX = destX;
    player.mother.tileY = destY;

    animateSlimes[animateSlimeCount++] =
        CreateSlimeAnimation(&player.mother,
                             player.maxTileSize,
                             prevX, prevY,
                             destX, destY,
                             player.mother.mass, player.mother.mass);
    
}

void SplitSlime(Player & player, int destX, int destY, int prevX, int prevY)
{
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to split");
        
    Slime * refSlime = nullptr;
    
    // NOTE: Create a small slime and shoot at the destination
    {
        Slime s = { 0 };
        s.tileX = destX;
        s.tileY = destY;
        s.mass = 1;
        s.show = false;
        s.split = true;

        player.children[player.childrenCount++] = s;
        refSlime = &player.children[player.childrenCount - 1];

        // NOTE: split Animation
        animateSlimes[animateSlimeCount++] =
            CreateSlimeAnimation(refSlime,
                                 player.maxTileSize,
                                 prevX, prevY,
                                 destX, destY,
                                 s.mass, s.mass);
        
    }

}

bool SlimeAction(GameState & gameState, int bounceDirX, int bounceDirY)
{
    
    bool stateChanged = false;

    Record & record = gameState.currentRecord;
    
    Player & player = record.player;

    int prevX = player.mother.tileX;
    int prevY = player.mother.tileY;

    bool bounce = false;
    int bounceX, bounceY;

    // NOTE: Bounce Position
    if (player.mother.mass > 1)
    {
        int x = player.mother.tileX + bounceDirX;
        int y = player.mother.tileY + bounceDirY;

        for ( ; !CheckOutOfBound(x, y, gameState.tileMap) ; x += bounceDirX, y += bounceDirY)
        {
            // TODO: check block not implimented
            if (!CheckWalls(x, y, record.wallCount, record.walls))
            {
                bounce = true;
                bounceX = x;
                bounceY = y;

                continue;
            }
            break;
        }
    }

    bool split = false;
    int splitX, splitY;

    // NOTE: Split Positions
    if (player.mother.mass > 1)
        {
        int splitDirX = - bounceDirX;
        int splitDirY = - bounceDirY;
        
        int x = player.mother.tileX + splitDirX;
        int y = player.mother.tileY + splitDirY;

        for ( ; !CheckOutOfBound(x, y, gameState.tileMap) ; x += splitDirX, y += splitDirY)
        {
            // TODO: check block not implimented
            if (!CheckWalls(x, y, record.wallCount, record.walls))
            {
                split = true;
                splitX = x;
                splitY = y;
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
            BounceSlime(player, bounceX, bounceY);
        }
        else if (split)
        {
            player.mother.tileSize = GetTileSize(player.maxTileSize, player.mother.mass, 4.0f);
            player.mother.pivot = GetTilePivot((float)player.mother.tileX,
                                               (float)player.mother.tileY,
                                               player.mother.tileSize);            
        }

        if (split)
        {
            SplitSlime(player, splitX, splitY, prevX, prevY);
        }
        else if (bounce)
        {
            SplitSlime(player, prevX, prevY, prevX, prevY);
        }
                
    }

    return stateChanged;
    
}

void LoadLevelToGameState(GameState & state, State loadState)
{
    int size;
    unsigned char * fileData = LoadFileData(LEVELS_PATH, &size);
    levelsTimestamp = GetTimestamp(LEVELS_PATH);
    // NOTE: Generate tile map    
    {

        state.currentRecord = { 0 };
        Record & currentRecord = state.currentRecord;

        // NOTE: initilized player
        {
            Player player = { 0 };
            player.moveSteps = 1;
            player.color = YELLOW;
            player.maxTileSize = MAP_TILE_SIZE;
            currentRecord.player = player;
        }

        animationPlaying = false;
        animateSlimeCount = 0;
        
        Map map = { 0 };

        unsigned int tileCountX = 0, tileCountY = 0;

        // NOTE: Create Level with string

        const char * level;

        state.state = loadState;
        bool loading = false;
        std::string l = "";

        int col = 0, row = 0;
            
        for (int i = 0; i < size; i++)
        {
            char c = fileData[i];

            if (loading)
            {
                if (c == ';') break;

                
                col++;

                switch(c)
                {
                    case '#':
                    {
                        // NOTE: Set Wall Inital States
                        currentRecord.wallSize = MAP_TILE_SIZE;
                        Wall wall1 = { 0 };
                        wall1.wallX = col;
                        wall1.wallY = row + 1;
                        currentRecord.walls[currentRecord.wallCount++] = wall1;
                        break;
                    }
                    case '@':
                    {
                        // NOTE: Set Player Inital States
                        Slime mother = { 0 };
                        
                        mother.tileX = col;
                        mother.tileY = row + 1;
                        mother.mass = 2;
                        mother.tileSize = GetTileSize(currentRecord.player.maxTileSize, mother.mass, 4.0f);
                        mother.pivot = GetTilePivot(mother.tileX, mother.tileY, mother.tileSize);
                        
                        currentRecord.player.mother = mother;
                        
                        break;
                    }
                    case '1':
                    {

                        Slime child = { 0 };

                        child.tileX = col;
                        child.tileY = row + 1;
                        child.mass = 1;
                        child.tileSize = GetTileSize(currentRecord.player.maxTileSize, child.mass, 4.0f);
                        child.pivot = GetTilePivot(child.tileX, child.tileY, child.tileSize);

                        SM_ASSERT(currentRecord.player.childrenCount < (MAX_SLIME - 1), "Out of child capacity!!");
                        currentRecord.player.children[currentRecord.player.childrenCount++] = child;
                        
                        break;
                    }
                    case 'g':
                    {
                        
                        // NOTE:Set Goals Inital States
                        currentRecord.goalSize = MAP_TILE_SIZE / 4;

                        Goal goal = { 0 };
                        goal.goalX = col;
                        goal.goalY = row + 1;

                        currentRecord.goals[currentRecord.goalCount++] = goal;
                        break;

                    }
                    case 't':
                    {

                        // NOTE:Set Goals Inital States
                        currentRecord.goalSize = MAP_TILE_SIZE / 2;

                        Goal goal = { 0 };
                        goal.goalX = col;
                        goal.goalY = row + 1;
                        goal.cover = true;
                        currentRecord.goals[currentRecord.goalCount++] = goal;
                        
                        // NOTE: Set Block Inital States
                        currentRecord.blockSize = MAP_TILE_SIZE;

                        Block block1 = { 0 };
                        block1.blockX = col;
                        block1.blockY = row + 1;
                        block1.color = DARKGREEN;
    
                        currentRecord.blocks[currentRecord.blockCount++] = block1;
                        break;
                    }
                    case 'b':
                    {
                        // NOTE: Set Block Inital States
                        currentRecord.blockSize = MAP_TILE_SIZE;

                        Block block1 = { 0 };
                        block1.blockX = col;
                        block1.blockY = row + 1;

                        block1.color = GREEN;
    
                        currentRecord.blocks[currentRecord.blockCount++] = block1;
                        break;
                    }
                    case '\r':
                    case '\n':
                    {
                        if (fileData[i + 1] == '\n') i++; 
                        tileCountX = col - 1;
                        tileCountY++;
                        row++;
                        col = 0;
                        break;
                    }
                }
                            
            }
            else
            {
                if (c == '\r')
                {
                    switch(loadState)
                    {
                        case STATE_TEST_LEVEL:
                        {
                            if (l == "TestLevel")
                            {
                                loading = true;
                            }
                            break;
                        }
                    }
                    if (fileData[i + 1] == '\n') i++; 
                    l = "";
                }
                else
                {
                    l += c;
                }
            }
        }

        UnloadFileData(fileData);

        map.tilesX = tileCountX;
        map.tilesY = tileCountY;

        state.tileMap = map;

        // NOTE: Camera Setup
        
        unsigned int tileX = tileCountX + 1;
        unsigned int tileY = tileCountY + 1;

        Vector2 pos = TilePositionToPixelPosition(tileX / 2.0f, tileY / 2.0f, MAP_TILE_SIZE);
        
        state.camera.target = { pos.x, pos.y };
        state.camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        state.camera.rotation = 0.0f;
        state.camera.zoom = 1.0f;

    }
}


void Merge(Record & record, SlimeAnimation & sa)
{

    for (int j = 0; j < record.player.childrenCount; j++)
    {
                            
        SM_ASSERT((sa.currentSlime == &record.player.mother)  ||
                  (sa.currentSlime != (record.player.children + record.player.childrenCount)),
                  "currentSlime should not be same as the deleted slime"); 
                            
        Slime * slime = record.player.children + j;

        if (sa.currentSlime != slime && !slime->split)
        {

            Rectangle rec1 = { sa.currentSlime->pivot.x, sa.currentSlime->pivot.y,
                sa.currentSlime->tileSize, sa.currentSlime->tileSize };
            Rectangle rec2 = { slime->pivot.x, slime->pivot.y, slime->tileSize, slime->tileSize };

            IVec2 tilePos = PixelPositionToTilePosition(sa.currentSlime->pivot.x, sa.currentSlime->pivot.y);

            // IMPORTANT: This is framerate dependent for not, to make it framerate independent,
            //            You need to implement fix timestep see:
            //              https://gafferongames.com/post/fix_your_timestep/ 
            //              https://www.youtube.com/watch?v=VpSWuywFlC8
            //  NOTE:    I think I should test if low frame rate actually affect the detection, then thinking of the above impl
            if ((tilePos.x == slime->tileX) && (tilePos.y == slime->tileY)) 
                // if (CheckCollisionRecs(rec1, rec2))
            {
                sa.currentSlime->mass += slime->mass;
                if (sa.currentSlime->mass > 4)
                {
                    sa.currentSlime->mass = 4;
                }

                Slime * last = record.player.children + (record.player.childrenCount - 1);
                                    
                *slime = record.player.children[--record.player.childrenCount];
                j--;

                if (sa.currentSlime == &record.player.mother)
                {
                    for (int k = 0; k < animateSlimeCount; k++)
                    {
                        if (last == animateSlimes[k].currentSlime)
                        {
                            animateSlimes[k].currentSlime = slime;
                        }
                    }
                }
                else if (last == sa.currentSlime)
                {
                    sa.currentSlime = slime;
                                            
                }
            }
        }
    }
}

// NOTE: Program main entry point
int main(void)
{
    // NOTE: Initialization
    GameState startState;

    // NOTE: Key Mappings
    {
        startState.keyMappings[MOUSE_LEFT].keys.Add(MOUSE_BUTTON_LEFT);
        startState.keyMappings[MOUSE_RIGHT].keys.Add(MOUSE_BUTTON_RIGHT);
    }
    
    LoadLevelToGameState(startState, STATE_TEST_LEVEL);

    // NOTE: Initalize undo record
    std::stack<Record> undoRecords;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jason's 3rd law");

    // NOTE: Arrow Buttons
    // UP
    startState.upArrow.sprite = GetSprite(SPRITE_ARROW_UP);
    startState.upArrow.id = SPRITE_ARROW_UP;
    startState.upArrow.tileSize = 32;

    // DOWN
    startState.downArrow.sprite = GetSprite(SPRITE_ARROW_DOWN);
    startState.downArrow.id = SPRITE_ARROW_DOWN;
    startState.downArrow.tileSize = 32;

    // LEFT
    startState.leftArrow.sprite = GetSprite(SPRITE_ARROW_LEFT);
    startState.leftArrow.id = SPRITE_ARROW_LEFT;
    startState.leftArrow.tileSize = 32;

    // RIGHT
    startState.rightArrow.sprite = GetSprite(SPRITE_ARROW_RIGHT);
    startState.rightArrow.id = SPRITE_ARROW_RIGHT;
    startState.rightArrow.tileSize = 32;
    
    GameState gameState = startState;

    undoRecords.push(gameState.currentRecord);

    Texture2D texture = LoadTexture(TEXTURE_PATH);        // Texture loading


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    
    // NOTE: Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // NOTE: Level Hot Reloading
        {
            long long currentTimeStamp = GetTimestamp(LEVELS_PATH);

            if (currentTimeStamp > levelsTimestamp)
            {
                LoadLevelToGameState(startState, startState.state);
                gameState = startState;
                undoRecords = std::stack<Record>();
            }

        }

        
        // NOTE: Update
        {

            // NOTE: Debug Camera Control
            {
                // NOTE: CameraZoom
                // Camera zoom controls
                // Uses log scaling to provide consistent zoom speed
                gameState.camera.zoom = expf(logf(gameState.camera.zoom) + ((float)GetMouseWheelMove()*0.1f));

                float moveSpeed = 500.0f;
                
                // NOTE: Camera Move
                if (IsKeyDown(KEY_UP))
                {
                    gameState.camera.target.y -= moveSpeed * GetFrameTime();
                }
                if (IsKeyDown(KEY_DOWN))
                {
                    gameState.camera.target.y += moveSpeed * GetFrameTime();

                }
                if (IsKeyDown(KEY_LEFT))
                {
                    gameState.camera.target.x -= moveSpeed * GetFrameTime();

                }
                if (IsKeyDown(KEY_RIGHT))
                {
                    gameState.camera.target.x += moveSpeed * GetFrameTime();
                }

                // NOTE: Camera Drag
                if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
                {
                    Vector2 mouseDelta = GetMouseDelta();
                    gameState.camera.target.x -= mouseDelta.x;
                    gameState.camera.target.y -= mouseDelta.y;
                }

                if (gameState.camera.zoom > 3.0f) gameState.camera.zoom = 3.0f;
                else if (gameState.camera.zoom < 0.1f) gameState.camera.zoom = 0.1f;

            }

            
            // NOTE: Recored if State Changes
            bool stateChanged = false;

            Record prevRecord = gameState.currentRecord;

            Record & record = gameState.currentRecord;
            
            // NOTE: Actions
            if (!animationPlaying) {
                // NOTE: read input

                
                // TODO NOTE: Move At edge of walls with keyboards
                {

                }

                // NOTE: Split Arrow Buttons
                {
                    gameState.leftArrow.show = gameState.rightArrow.show = gameState.upArrow.show = gameState.downArrow.show = true;
                    
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState.camera);
                
                    IVec2 centerPos = { (int)record.player.mother.tileX,  (int)record.player.mother.tileY };
        
                    Vector2 upPos    = { (float)centerPos.x, (float)(centerPos.y-1) };
                    Vector2 downPos  = { (float)centerPos.x, (float)(centerPos.y+1) };
                    Vector2 leftPos  = { (float)(centerPos.x-1), (float)(centerPos.y) };
                    Vector2 rightPos = { (float)(centerPos.x+1), (float)(centerPos.y) };

                    Vector2 dir[4] = { upPos, downPos, leftPos, rightPos };
                    ArrowButton * arrows[4] = { &gameState.upArrow, &gameState.downArrow, &gameState.leftArrow, &gameState.rightArrow };

                    for (int i = 0; i < 4; i++)
                    {
                        Vector2 tilePos = dir[i];

                        ArrowButton * arrow = arrows[i];
                                                                        
                        Vector2 pos = TilePositionToPixelPosition(tilePos.x, tilePos.y,  0);
                        Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));
                        
                        float tileSize = (float)arrow->tileSize;

                        Rectangle rect = { (float)topLeft.x, (float)topLeft.y, tileSize, tileSize };
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

                    
                    if (JustPressed(gameState, MOUSE_LEFT))
                    {
                        
                        if (gameState.leftArrow.hover)
                        {
                            // IMPORTANT shoot left and bounce right
                            stateChanged = SlimeAction(gameState, 1, 0);
                        }
                        
                        if (gameState.rightArrow.hover)
                        {
                            // IMPORTANT shoot right and bounce left
                            stateChanged = SlimeAction(gameState, -1, 0);
                        }
                        
                        if (gameState.upArrow.hover)
                        {
                            // IMPORTANT shoot up and bounce down
                            stateChanged = SlimeAction(gameState, 0, 1);
                        }
                        
                        if (gameState.downArrow.hover)
                        {
                            // IMPORTANT shoot down and bounce up
                            stateChanged = SlimeAction(gameState, 0, -1);
                        }
                    }
                }

                if (stateChanged)
                {
                    gameState.animateTime = 0;
                    undoRecords.push(prevRecord);
                    continue;
                }

            }
            else
            {
                // NOTE: Simulate

                if (gameState.animateTime <= gameState.duration)
                {
                    SM_ASSERT(gameState.duration > 0, "Animation time is zero!");

                    gameState.animateTime += GetFrameTime();

                    float x = gameState.animateTime / gameState.duration;

                    if (x > 1)  x = 1;
                    float t = EaseOutSine(x);

                    for (int i = 0; i < animateSlimeCount; i++)
                    {
                        SlimeAnimation & sa = animateSlimes[i];
                    
                        sa.currentSlime->tileSize = GetTileSize(record.player.maxTileSize, sa.currentSlime->mass, 4);

                        Vector2 startPivot = GetTilePivot(sa.startTilePos.x, sa.startTilePos.y, sa.currentSlime->tileSize);
                        Vector2 endPivot = GetTilePivot(sa.endTilePos.x, sa.endTilePos.y, sa.currentSlime->tileSize);
                        
                        sa.currentSlime->pivot = Vector2Lerp(startPivot, endPivot, t);

                        Merge(record, sa);
                    }
                    
                }
                else
                {
                    animateSlimeCount = 0;
                    gameState.animateTime = 0;
                    animationPlaying = false;

                    record.player.mother.split = false;

                    Slime * max = nullptr;
                    int maxMass = 0;
                    
                    for (int i = 0; i < record.player.childrenCount; i++)
                    {
                        Slime * child = record.player.children + i;
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
                        Slime tmp = record.player.mother;
                        record.player.mother = *max;
                        *max = tmp;
                    }
                    
                    continue;
                }

            }

            
            // NOTE: Undo and Restart
            {
                if (IsKeyPressed(KEY_Z) && !undoRecords.empty())
                {
                    // NOTE: Undo                
                    gameState.currentRecord = undoRecords.top();
                    undoRecords.pop();

                    animateSlimeCount = 0;
                    gameState.animateTime = 0;
                    animationPlaying = false;
                }

                // NOTE: Restart States        
                if (IsKeyPressed(KEY_R))
                {
                    undoRecords.push(gameState.currentRecord);
                    gameState.currentRecord = startState.currentRecord;

                    animateSlimeCount = 0;
                    gameState.animateTime = 0;
                    animationPlaying = false;

                }
            }

        }
        
    
        // NOTE: Draw
        BeginDrawing();
        ClearBackground(GRAY);

        BeginMode2D(gameState.camera);

        // NOTE: Draw Tile Maps
        for (unsigned int y = 0; y < gameState.tileMap.tilesY; y++)
        {
            for (unsigned int x = 0; x < gameState.tileMap.tilesX; x++)
            {
                // NOTE: Draw tiles from id (and tile borders)
                DrawRectangle(
                    x * MAP_TILE_SIZE,
                    y * MAP_TILE_SIZE,
                    MAP_TILE_SIZE,
                    MAP_TILE_SIZE,                        
                    BLUE);
                    
                DrawRectangleLines(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE, Fade(DARKBLUE, 0.5f));
                    
            }
        }

        Record & record = gameState.currentRecord;

        
        
        // NOTE: Draw Goal
        for (unsigned int index = 0; index < record.goalCount; index++)
        {
            Goal & goal = record.goals[index];
            Vector2 goalPos = TilePositionToPixelPosition((float)goal.goalX, (float)goal.goalY, record.goalSize);

            Color color = PINK;

            color.a = 200;
            
            DrawCircleV(goalPos, (float)record.goalSize, color);
            
        }
        

        // NOTE: Draw player

        Color pColor = record.player.color;
        
        // NOTE: Test
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState.camera);
        {

            Record & record = gameState.currentRecord;

            Rectangle playerRect = {
                (float)record.player.mother.pivot.x,
                (float)record.player.mother.pivot.y,
                (float)record.player.mother.tileSize,
                (float)record.player.mother.tileSize };

            if (CheckCollisionPointRec(mousePos, playerRect)) pColor = RED;
        }

        
        DrawRectangleV(record.player.mother.pivot, { (float)record.player.mother.tileSize, (float)record.player.mother.tileSize}, pColor);


        // NOTE: Draw children
        for (int i = 0; i < record.player.childrenCount; i++)
        {
            Slime child = record.player.children[i];

            DrawRectangleV(child.pivot, { (float)child.tileSize, (float)child.tileSize}, GREEN);
        }
        
        // NOTE: Draw Blocks
        for (unsigned int index = 0; index < record.blockCount; index++)
        {
            Block & block = record.blocks[index];
            Vector2 blockCenterPos = TilePositionToPixelPosition((float)block.blockX, (float)block.blockY, record.blockSize);
            Vector2 blockTopLeftPos = Vector2Subtract(blockCenterPos, Vector2Scale(Vector2One(), (float)record.blockSize * 0.5f));
            DrawRectangleV(blockTopLeftPos, {  (float)record.blockSize,  (float)record.blockSize }, block.color);
        }

        // NOTE: Draw Walls
        for (unsigned int index = 0; index < record.wallCount; index++)
        {
            Wall & wall = record.walls[index];
            Vector2 wallCenterPos = TilePositionToPixelPosition((float)wall.wallX, (float)wall.wallY, record.wallSize);
            Vector2 wallTopLeftPos = Vector2Subtract(wallCenterPos, Vector2Scale(Vector2One(), (float)record.wallSize * 0.5f));

            DrawRectangleV(wallTopLeftPos, {  (float)record.wallSize,  (float)record.wallSize }, BLACK);
        }

        
        if (!animationPlaying)
        {
            // NOTE: Arrow sprite
            // Left
            if (gameState.leftArrow.show)
            {
                Sprite & leftArrow = gameState.leftArrow.sprite;
                float tileSize = (float)gameState.leftArrow.tileSize;
            
                Rectangle source = {
                    (float)leftArrow.altasOffset.x, (float)leftArrow.altasOffset.y,
                    (float)leftArrow.spriteSize.x, (float)leftArrow.spriteSize.y
                };
            
                DrawTextureRec(texture, source, gameState.leftArrow.topLeftPos, WHITE);
            }


            // Right
            if (gameState.rightArrow.show)
            {
                Sprite rightArrow = gameState.rightArrow.sprite;
                float tileSize = (float)gameState.rightArrow.tileSize;
            
                Rectangle source = {
                    (float)rightArrow.altasOffset.x, (float)rightArrow.altasOffset.y,
                    (float)rightArrow.spriteSize.x, (float)rightArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, gameState.rightArrow.topLeftPos, WHITE);
            }

            // Up
            if (gameState.upArrow.show)
            {
                Sprite upArrow = gameState.upArrow.sprite;
                float tileSize = (float)gameState.upArrow.tileSize;
            
                Rectangle source = {
                    (float)upArrow.altasOffset.x, (float)upArrow.altasOffset.y,
                    (float)upArrow.spriteSize.x, (float)upArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, gameState.upArrow.topLeftPos, WHITE);
            }

            // Down
            if (gameState.downArrow.show)
            {
                Sprite downArrow = gameState.downArrow.sprite;
                float tileSize = (float)gameState.downArrow.tileSize;
            
                Rectangle source = {
                    (float)downArrow.altasOffset.x, (float)downArrow.altasOffset.y,
                    (float)downArrow.spriteSize.x, (float)downArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, gameState.downArrow.topLeftPos, WHITE);
            }
        }
        
        EndMode2D();
        
        // NOTE: UI Draw Game Informations

        IVec2 centerPos = { (int)record.player.mother.tileX, (int)record.player.mother.tileY };
        
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player Size: %.2f, Child Count: %i",
                            centerPos.x, centerPos.y,
                            record.player.mother.mass, record.player.mother.tileSize, record.player.childrenCount), 10, 140, 20, GREEN);
        DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f",
                            gameState.camera.target.x, gameState.camera.target.y,
                            gameState.camera.offset.x, gameState.camera.offset.y, gameState.camera.zoom), 10, 50, 20, RAYWHITE);
        DrawText("Arrow Direction to Shoot, R KEY to Restart, Z KEY to undo", 10, 10, 20, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadTexture(texture);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
