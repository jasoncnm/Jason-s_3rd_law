#if !defined(GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_QUEUE 100
#define MAX_BLOCKS 100
#define MAX_WALLS 100
#define MAX_GOALS 100
#define MAX_ANIMATION 100

#define MAX_UNDO_RECORDS 1000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)


#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "assets.h"

#include "slime.h"

enum State
{
    STATE_TEST_LEVEL,
    STATE_COUNT,
};

enum GameInputType 
{
    NO_INPUT,
    MOUSE_LEFT,
    MOUSE_RIGHT,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY,
    DOWN_KEY,
    SPACE_KEY,
    GAME_INPUT_COUNT,
};

struct KeyMapping
{
    Array<int, 3> keys;
};


// NOTE: Map data type
struct Map
{
    unsigned int tilesX;            // Number of tiles in X axis
    unsigned int tilesY;            // Number of tiles in Y axis
};

// NOTE: Goal spot
struct Goal
{
    bool cover = false;
    IVec2 tile;
};

// NOTE: PushBlock
struct Block
{
    Sprite sprite;
    SpriteID id;

    IVec2 tile;

    int mass = 1;
    
    Color color;
};

// NOTE: Wall
struct Wall
{
    Sprite sprite;
    SpriteID id;
    int tileSize = 32;

    IVec2 tile;
};

struct ArrowButton
{
    Sprite sprite;
    SpriteID id;

    Vector2 topLeftPos;

    bool seletable = false;
    bool hover = false;
    bool preseed = false;
    bool show = true;
};


struct Record
{

    Player player;

    Array<Block, MAX_BLOCKS> blocks;
    Array<Wall, MAX_WALLS> walls;
    Array<Goal, MAX_GOALS>  goals;
    
};


// NOTE: GameState
struct GameState
{
    State state;

    Camera2D camera;

    Map tileMap;

    Record currentRecord;

    KeyMapping keyMappings[GAME_INPUT_COUNT];
    
    ArrowButton upArrow, downArrow, leftArrow, rightArrow;

    float animateTime = 0.0f;
    float duration = 1.0f;

    bool initialized = false;
};


// NOTE: Globals

static float timeSinceLastPress = 0.0f;
static const float pressFreq = 0.2f;


bool CheckOutOfBound(unsigned int tileX, unsigned int tileY, Map & tileMap)
{
    bool result =
           (tileX < 1)
        || (tileX > tileMap.tilesX)
        || (tileY < 1)
        || (tileY > tileMap.tilesY);

    return result;
}

bool CheckOutOfBound(IVec2 tilePos, Map & tileMap)
{
    return CheckOutOfBound(tilePos.x, tilePos.y, tileMap);
}

template<typename T, int N>
bool CheckTiles(IVec2 tilePos, Array<T, N> arr)
{
    bool result = false;

    for (unsigned int index = 0; index < arr.count; index++)
    {
        if (arr[index].tile == tilePos)
        {
            result = true;
            break;
        }
    }

    return result;

}

int GetBlockIndex(IVec2 tile, Array<Block, MAX_BLOCKS> blocks)
{
    for (unsigned int index = 0; index < blocks.count; index++)
    {
        Block & block = blocks[index];
        if (tile == block.tile)
        {
            return index;
        }
    }
    return -1;
}

bool BounceBlock(GameState & gameState, IVec2 dir, int blockIndex)
{

    bool result = false;

    Record & record = gameState.currentRecord;
    
    IVec2 bp = record.blocks[blockIndex].tile;

    IVec2 start = bp + dir;
    
    for (IVec2 pos = start;
         !CheckTiles(pos, record.walls);
         pos = pos + dir)
    {
        result = true;

        if (CheckOutOfBound(pos, gameState.tileMap))
        {
            record.blocks.RemoveIdxAndSwap(blockIndex);
            return result;
        }
        
        int nIndex = GetBlockIndex(pos + dir, record.blocks);
        
        if (nIndex != -1 && blockIndex != nIndex)
        {
            record.blocks[blockIndex].tile = pos;
            blockIndex = nIndex;
        }
        bp = pos;
    }

    record.blocks[blockIndex].tile = bp;

    return result;

}

bool CheckPushableBlocks(GameState & gameState, Slime & slime,
                         IVec2 blockNextPos, IVec2 pushDir, 
                         int accumulatedMass, bool & pushed)
{

    Record & record = gameState.currentRecord;

    pushed = true;
    
    if (CheckTiles(blockNextPos, record.walls))
    {
        return false;
    }
    
    int index = GetBlockIndex(blockNextPos, record.blocks);

    if (index != -1)
    {
        Block & block = record.blocks[index];
        int newAccumulatedMass = accumulatedMass + block.mass;
        if (newAccumulatedMass > slime.mass)
        {
            //slime.attachDir = pushDir;
            
            return false;
        }
        
        if (CheckPushableBlocks(gameState, slime, blockNextPos + pushDir, pushDir, newAccumulatedMass, pushed))
        {
            IVec2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

            for (int i = 0; i < 4; i++)
            {
                if (CheckTiles(block.tile + dirs[i], record.walls))
                {
                    block.tile = blockNextPos + pushDir;

                    pushed |= true;
                    
                    return true;
                }
            }

            BounceBlock(gameState, pushDir, index);

            pushed |= true;
            
            return true;
        }
        return false;
    }

    pushed = false;
        
    return true;
}



#define GAME_H
#endif
