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

void Merge(SlimeAnimation & sa, Player & player, Slime * currentSlime)
{
    SM_ASSERT(player.childrenCount > 0, "No child to merge");
    // SM_ASSERT(sa.mergedSlimeIndex < player.childrenCount , "Merged Slime Out of Bound!");
    // SM_ASSERT(!sa.possessed || (sa.possesserIndex < player.childrenCount), "Possesser Out of Bound!");
    // SM_ASSERT(!sa.possessed || (sa.possesserIndex != sa.mergedSlimeIndex), "You cannot possesse itself!!");
    
    currentSlime->mass += sa.mergedSlime->mass;
    currentSlime->tileSize = GetTileSize(player.maxTileSize, currentSlime->mass, 4.0f);
    currentSlime->pivot = GetTilePivot(currentSlime->tileX, currentSlime->tileY, currentSlime->tileSize);

    if (sa.possessed)
    {
        Slime tmp = player.mother;

        player.mother = *sa.possesserSlime;

        *sa.possesserSlime = tmp;
        
    }

    *sa.mergedSlime = player.children[--player.childrenCount];
    
}

SlimeAnimation CreateSlimeAnimation(Slime * refSlime,
                                    float maxTileSize,
                                    int startX, int startY,
                                    int endX, int endY,
                                    int startMass, int endMass,
                                    bool merge = false, Slime * mergedSlime = nullptr,
                                    bool possessed = false, Slime * possesserSlime = nullptr)
{
    Slime aniStart = { 0 };
    
    aniStart.tileSize = GetTileSize(maxTileSize, startMass, 4.0f);
    aniStart.pivot = GetTilePivot((float)startX, (float)startY, aniStart.tileSize);

    Slime aniEnd = { 0 };

    aniEnd.tileSize = GetTileSize(maxTileSize, endMass, 4.0f);
    aniEnd.pivot = GetTilePivot((float)endX, (float)endY, aniEnd.tileSize);

    SlimeAnimation result = { 0 };
    result.startSlime = aniStart;
    result.endSlime = aniEnd;
    result.currentSlime = refSlime;
    result.merge = merge;
    result.mergedSlime = mergedSlime;
    result.possessed = possessed;
    result.possesserSlime = possesserSlime;
    
    return result;
    
}

int BounceSlime(Player & player, int destX, int destY)
{
    
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to bounce");

    int mass = player.mother.mass;
    
    bool merge = false;
    int mergedIndex = -1;

    int prevX = player.mother.tileX;
    int prevY = player.mother.tileY;

    for (int i = 0; i < player.childrenCount;  i++)
    {
        Slime & child = player.children[i];

        // Rectangle [(prevX, prevY),  (destX, destY)]
        int dx = destX - prevX;
        int dy = destY - prevY;
        int dChildX = child.tileX - prevX;
        int dChildY = child.tileY - prevY;

        if ((abs(dx) >= abs(dChildX)) && (abs(dy) >= abs(dChildY)) &&
            SameSign(dx, dChildX) && SameSign(dy, dChildY))
        {
            // NOTE: merge
            // TODO: merge multiple children in single action
            merge = true;
            mergedIndex = i;

            mass += child.mass;

            animationPlaying = true;

            break;
        }
    }
          
    player.mother.tileX = destX;
    player.mother.tileY = destY;

    animateSlimes[animateSlimeCount++] =
        CreateSlimeAnimation(&player.mother,
                             player.maxTileSize,
                             prevX, prevY, destX, destY,
                             player.mother.mass, player.mother.mass,
                             merge, &player.children[mergedIndex]);

    return mass;    
    
}

void SplitSlime(Player & player, int destX, int destY, int prevX, int prevY, int playerMass)
{
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to split");

    bool merge = false;
    bool possessed = false;

    int mergedIndex = -1;
    int possesserIndex = -1;
        
    Slime * refSlime = nullptr;

    
    // NOTE: Check for merge and possess 
    for (int i = 0; i < player.childrenCount; i++)
    {
        Slime * child = &player.children[i];

        // Rectangle [(prevX, prevY),  (destX, destY)]
        int dx = destX - prevX;
        int dy = destY - prevY;
        int dChildX = child->tileX - prevX;
        int dChildY = child->tileY - prevY;

        if ((abs(dx) >= abs(dChildX)) && (abs(dy) >= abs(dChildY)) &&
            SameSign(dx, dChildX) && SameSign(dy, dChildY))
        {
            // TODO: merge multiple children in single action

            merge = true;
            mergedIndex = i;
            
            int childMass = child->mass + 1;
            
            if (childMass > playerMass)
            {
                possessed = true;
                possesserIndex = player.childrenCount;
            }
            
            break;
        }
    }

    
    // NOTE: Create a small slime and shoot at the destination
    {
        Slime s = { 0 };
        s.tileX = destX;
        s.tileY = destY;
        s.mass = 1;
        s.show = false;

        player.children[player.childrenCount++] = s;
        refSlime = &player.children[player.childrenCount - 1];

        // NOTE: split Animation
        animationPlaying = true;
        animateSlimes[animateSlimeCount++] =
            CreateSlimeAnimation(refSlime, player.maxTileSize, prevX, prevY,
                                 destX, destY, 1, 1,  merge, &player.children[mergedIndex],
                                 possessed, &player.children[possesserIndex]);
        
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
            mass = BounceSlime(player, bounceX, bounceY);
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
            SplitSlime(player, splitX, splitY, prevX, prevY, mass);
        }
        else if (bounce)
        {
            SplitSlime(player, prevX, prevY, prevX, prevY, mass);
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
    startState.currentRecord.upArrow.sprite = GetSprite(SPRITE_ARROW_UP);
    startState.currentRecord.upArrow.id = SPRITE_ARROW_UP;
    startState.currentRecord.upArrow.tileSize = 32;

    // DOWN
    startState.currentRecord.downArrow.sprite = GetSprite(SPRITE_ARROW_DOWN);
    startState.currentRecord.downArrow.id = SPRITE_ARROW_DOWN;
    startState.currentRecord.downArrow.tileSize = 32;

    // LEFT
    startState.currentRecord.rightArrow.sprite = GetSprite(SPRITE_ARROW_LEFT);
    startState.currentRecord.rightArrow.id = SPRITE_ARROW_LEFT;
    startState.currentRecord.rightArrow.tileSize = 32;

    // RIGHT
    startState.currentRecord.rightArrow.sprite = GetSprite(SPRITE_ARROW_RIGHT);
    startState.currentRecord.rightArrow.id = SPRITE_ARROW_RIGHT;
    startState.currentRecord.rightArrow.tileSize = 32;
    
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

                // NOTE: Arrow Buttons
                {
                    record.leftArrow.show = record.rightArrow.show = record.upArrow.show = record.downArrow.show = true;
                    
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState.camera);
                
                    IVec2 centerPos = { (int)record.player.mother.tileX,  (int)record.player.mother.tileY };
        
                    Vector2 upPos    = { (float)centerPos.x, (float)(centerPos.y-1) };
                    Vector2 downPos  = { (float)centerPos.x, (float)(centerPos.y+1) };
                    Vector2 leftPos  = { (float)(centerPos.x-1), (float)(centerPos.y) };
                    Vector2 rightPos = { (float)(centerPos.x+1), (float)(centerPos.y) };

                    // LEFT
                    {
                        Vector2 pos = TilePositionToPixelPosition(leftPos.x, leftPos.y,  0);
                        Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));

                        Sprite & leftArrow = record.leftArrow.sprite;
                        float tileSize = (float)record.leftArrow.tileSize;
            
                        Rectangle rect = { (float)topLeft.x, (float)topLeft.y, tileSize, tileSize };
                        if (CheckCollisionPointRec(mousePos, rect))
                        {
                            record.leftArrow.hover = true;
                            leftArrow = GetSprite(SPRITE_ARROW_LEFT_HOVER);
                        }
                        else
                        {
                            record.leftArrow.hover = false;
                            leftArrow = GetSprite(SPRITE_ARROW_LEFT);
                        }
                        record.leftArrow.topLeftPos = topLeft;

                    }

                    // RIGHT
                    {
                        Vector2 pos = TilePositionToPixelPosition(rightPos.x, rightPos.y,  0);
                        Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));

                        Sprite & rightArrow = record.rightArrow.sprite;
                        float tileSize = (float)record.rightArrow.tileSize;
            
                        Rectangle rect = { (float)topLeft.x, (float)topLeft.y, tileSize, tileSize };
                        if (CheckCollisionPointRec(mousePos, rect))
                        {
                            record.rightArrow.hover = true;
                            rightArrow = GetSprite(SPRITE_ARROW_RIGHT_HOVER);
                        }
                        else
                        {
                            record.rightArrow.hover = false;
                            rightArrow = GetSprite(SPRITE_ARROW_RIGHT);
                        }
                        record.rightArrow.topLeftPos = topLeft;
                    }

                    // UP
                    {
                        Vector2 pos = TilePositionToPixelPosition(upPos.x, upPos.y,  0);
                        Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));

                        Sprite & upArrow = record.upArrow.sprite;
                        float tileSize = (float)record.upArrow.tileSize;
            
                        Rectangle rect = { (float)topLeft.x, (float)topLeft.y, tileSize, tileSize };
                        if (CheckCollisionPointRec(mousePos, rect))
                        {
                            record.upArrow.hover = true;
                            upArrow = GetSprite(SPRITE_ARROW_UP_HOVER);
                        }
                        else
                        {
                            record.upArrow.hover = false;
                            upArrow = GetSprite(SPRITE_ARROW_UP);
                        }
                        record.upArrow.topLeftPos = topLeft;
                    }


                    // DOWN
                    {
                        Vector2 pos = TilePositionToPixelPosition(downPos.x, downPos.y,  0);
                        Vector2 topLeft  = Vector2Subtract(pos, Vector2Scale(Vector2One(), (float)32 * 0.5f));

                        Sprite & downArrow = record.downArrow.sprite;
                        float tileSize = (float)record.downArrow.tileSize;
            
                        Rectangle rect = { (float)topLeft.x, (float)topLeft.y, tileSize, tileSize };
                        if (CheckCollisionPointRec(mousePos, rect))
                        {
                            record.downArrow.hover = true;
                            downArrow = GetSprite(SPRITE_ARROW_DOWN_HOVER);
                        }
                        else
                        {
                            record.downArrow.hover = false;
                            downArrow = GetSprite(SPRITE_ARROW_DOWN);
                        }
                        record.downArrow.topLeftPos = topLeft;
                    }

                    if (JustPressed(gameState, MOUSE_LEFT))
                    {
                        
                        if (record.leftArrow.hover)
                        {
                            // IMPORTANT shoot left and bounce right
                            stateChanged = SlimeAction(gameState, 1, 0);
                        }
                        
                        if (record.rightArrow.hover)
                        {
                            // IMPORTANT shoot right and bounce left
                            stateChanged = SlimeAction(gameState, -1, 0);
                        }
                        
                        if (record.upArrow.hover)
                        {
                            // IMPORTANT shoot up and bounce down
                            stateChanged = SlimeAction(gameState, 0, 1);
                        }
                        
                        if (record.downArrow.hover)
                        {
                            // IMPORTANT shoot down and bounce up
                            stateChanged = SlimeAction(gameState, 0, -1);
                        }
                    }
                }

                // NOTE: Simulate
                {
                    if (stateChanged)
                    {
                        gameState.animateTime = 0;
                        undoRecords.push(prevRecord);
                        continue;
                    }
                }

            }
            else
            {

                if (gameState.animateTime <= gameState.duration)
                {
                    SM_ASSERT(gameState.duration > 0, "Animation time is zero!");

                    gameState.animateTime += GetFrameTime();

                    float x = gameState.animateTime / gameState.duration;

                    if (x > 1)  x = 1;

                    for (int i = 0; i < animateSlimeCount; i++)
                    {

                        SlimeAnimation & sa = animateSlimes[i];
                        
                        Slime & startSlime = sa.startSlime;
                        Slime & endSlime = sa.endSlime;
                        Slime * currentSlime = sa.currentSlime;

                        float t = EaseOutSine(x);

                        currentSlime->pivot = Vector2Lerp(startSlime.pivot, endSlime.pivot, t);

                        if (sa.merge && t >= sa.mergetime)
                        {
                            Merge(sa, record.player, currentSlime);                            
                        }
                        else
                        {
                            currentSlime->tileSize = (1 - t) * startSlime.tileSize + t * endSlime.tileSize;
                        }
                    }
                    
                }
                else
                {
                    animateSlimeCount = 0;
                    gameState.animateTime = 0;
                    animationPlaying = false;
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
                }

                // NOTE: Restart States        
                if (IsKeyPressed(KEY_R))
                {
                    undoRecords.push(gameState.currentRecord);
                    gameState.currentRecord = startState.currentRecord;
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
        
        if (!animationPlaying)
        {
            // NOTE: Arrow sprite
            // Left
            if (record.leftArrow.show)
            {
                Sprite & leftArrow = record.leftArrow.sprite;
                float tileSize = (float)record.leftArrow.tileSize;
            
                Rectangle source = {
                    (float)leftArrow.altasOffset.x, (float)leftArrow.altasOffset.y,
                    (float)leftArrow.spriteSize.x, (float)leftArrow.spriteSize.y
                };
            
                DrawTextureRec(texture, source, record.leftArrow.topLeftPos, WHITE);
            }


            // Right
            if (record.rightArrow.show)
            {
                Sprite rightArrow = record.rightArrow.sprite;
                float tileSize = (float)record.rightArrow.tileSize;
            
                Rectangle source = {
                    (float)rightArrow.altasOffset.x, (float)rightArrow.altasOffset.y,
                    (float)rightArrow.spriteSize.x, (float)rightArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, record.rightArrow.topLeftPos, WHITE);
            }

            // Up
            if (record.upArrow.show)
            {
                Sprite upArrow = record.upArrow.sprite;
                float tileSize = (float)record.upArrow.tileSize;
            
                Rectangle source = {
                    (float)upArrow.altasOffset.x, (float)upArrow.altasOffset.y,
                    (float)upArrow.spriteSize.x, (float)upArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, record.upArrow.topLeftPos, WHITE);
            }

            // Down
            if (record.downArrow.show)
            {
                Sprite downArrow = record.downArrow.sprite;
                float tileSize = (float)record.downArrow.tileSize;
            
                Rectangle source = {
                    (float)downArrow.altasOffset.x, (float)downArrow.altasOffset.y,
                    (float)downArrow.spriteSize.x, (float)downArrow.spriteSize.y
                };
                DrawTextureRec(texture, source, record.downArrow.topLeftPos, WHITE);
            }
        }
        
        EndMode2D();
        
        // NOTE: UI Draw Game Informations

        IVec2 centerPos = { (int)record.player.mother.tileX, (int)record.player.mother.tileY };
        
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player Size: %.2f", centerPos.x, centerPos.y,
                            record.player.mother.mass, record.player.mother.tileSize), 10, 140, 20, GREEN);
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
