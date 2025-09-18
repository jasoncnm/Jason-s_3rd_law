#if !defined(GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_BLOCKS 500
#define MAX_WALLS 1100
#define MAX_CABLE 200
#define MAX_ANIMATION 10

#define MAX_UNDO_RECORDS 1000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)

#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "assets.h"

#include "slime.h"
#include "electric_door.h"

// ----------------------------------------------------
// NOTE: Game Structs
// ----------------------------------------------------

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

struct Glass
{
    Sprite sprite;
    SpriteID id;
    int tileSize = 32;

    bool broken = false;

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

    ElectricDoorSystem electricDoorSystem;

    Array<Block, MAX_BLOCKS> blocks;
    Array<Wall, MAX_WALLS> walls;
    Array<Glass, MAX_BLOCKS> glasses;

    bool CheckWalls(IVec2 tilePos);
};


// NOTE: Map data type
struct Map
{
    Record resetRecord;
    
    IVec2 tilePos;            // Top left tile position of the map
    int   width;              // Number of tiles in X axis
    int   height;             // Number of tiles in Y axis

    bool firstEnter = false;
};


// NOTE: GameState
struct GameState
{
    State state;

    IVec2 tileMin, tileMax;
    
    Camera2D camera;

    Array<Map, 100> tileMaps;

    Record currentRecord;

    KeyMapping keyMappings[GAME_INPUT_COUNT];
    
    ArrowButton upArrow, downArrow, leftArrow, rightArrow;

    float animateTime = 0.0f;
    float duration = 1.0f;

    int currentMapIndex = 0;


    bool initialized = false;
};

// ----------------------------------------------------
// NOTE: Game Globals
// ----------------------------------------------------
static float timeSinceLastPress = 0.0f;
static const float pressFreq = 0.2f;
static GameState * gameState;
static std::stack<Record> undoRecords;


// ----------------------------------------------------
// NOTE: Game Functions
// ----------------------------------------------------

bool Record::CheckWalls(IVec2 tilePos)
{
    bool result = CheckTiles(tilePos, walls);

    for (int i = 0; i < electricDoorSystem.doorIndexes.count; i++)
    {
        Cable & door = electricDoorSystem.cables[electricDoorSystem.doorIndexes[i]];
        if (door.tile == tilePos)
        {
            result |= true;
        }
    }

    for (int i = 0; i < glasses.count; i++)
    {
        Glass & glass = glasses[i];
        if (!glass.broken && glass.tile == tilePos)
        {
            result |= true;
        }
    }
    
    return result;
}

bool CheckOutOfBound(int tileX, int tileY)
{
    bool result =
           (tileX < gameState->tileMin.x)
        || (tileX > gameState->tileMax.x)
        || (tileY < gameState->tileMin.y)
        || (tileY > gameState->tileMax.y);

    return result;
}

bool CheckOutOfBound(IVec2 tilePos)
{
    return CheckOutOfBound(tilePos.x, tilePos.y);
}

bool BounceBlock(IVec2 dir, int blockIndex)
{

    bool result = false;

    Record & record = gameState->currentRecord;
    
    IVec2 bp = record.blocks[blockIndex].tile;

    IVec2 start = bp + dir;
    
    for (IVec2 pos = start;
         ;
         pos = pos + dir)
    {
        if (record.electricDoorSystem.DoorBlocked(pos, dir) || CheckTiles(pos, record.walls))
        {
            break;
        }

        result = true;

        int glassIndex = GetTileIndex(pos, record.glasses);
        if (glassIndex >= 0)
        {
            record.glasses[glassIndex].broken = true;
            record.glasses[glassIndex].sprite = GetSprite(SPRITE_GLASS_BROKEN);
        }
        
        if (CheckOutOfBound(pos))
        {
            record.blocks.RemoveIdxAndSwap(blockIndex);
            return result;
        }
        
        int nIndex = GetTileIndex(pos + dir, record.blocks);
        
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

bool CheckPushableBlocks(Slime & slime,
                         IVec2 blockNextPos, IVec2 pushDir, 
                         int accumulatedMass, bool & pushed)
{

    Record & record = gameState->currentRecord;

    pushed = true;
    
    if (record.CheckWalls(blockNextPos))
    {
        return false;
    }
    
    int index = GetTileIndex(blockNextPos, record.blocks);

    if (index != -1)
    {
        Block & block = record.blocks[index];
        int newAccumulatedMass = accumulatedMass + block.mass;
        if (newAccumulatedMass > slime.mass)
        {
            //slime.attachDir = pushDir;
            
            return false;
        }
        
        if (CheckPushableBlocks(slime, blockNextPos + pushDir, pushDir, newAccumulatedMass, pushed))
        {
            IVec2 dirs[4] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

            for (int i = 0; i < 4; i++)
            {
                if (record.CheckWalls(block.tile + dirs[i]) || CheckTiles(block.tile + dirs[i], record.blocks))
                {
                    block.tile = blockNextPos + pushDir;

                    pushed |= true;
                    
                    return true;
                }
            }

            BounceBlock(pushDir, index);

            pushed |= true;
            
            return true;
        }
        return false;
    }

    pushed = false;
        
    return true;
}


inline void PowerOnCable(Cable & cable, bool & end)
{

    Record & record = gameState->currentRecord;
    if (cable.type == CABLE_TYPE_DOOR)
    {
        if (!cable.open)
        {

            // TODO: Need to reconsider the approache to un freeze slime when there are duplicates
            if (record.player.slimes[record.player.motherIndex].state == FREEZE_STATE)
            {
                record.player.slimes[record.player.motherIndex].state = MOVE_STATE;
            }
            cable.open = true;
            switch(cable.id)
            {
                case SPRITE_DOOR_LEFT_CLOSE:
                {
                    IVec2 offset = { 1, -1 };

                    IVec2 bounceDir = { 1, 0 };

                    int blockIndex = GetTileIndex(cable.tile + bounceDir, record.blocks);

                    if (blockIndex >= 0)
                    {
                        BounceBlock(bounceDir, blockIndex);
                    }

                    cable.tile = cable.tile + offset;
                    cable.sprite = GetSprite(SPRITE_DOOR_LEFT_OPEN);
                
                    break;
                }
                case SPRITE_DOOR_RIGHT_CLOSE:
                {
                    IVec2 offset = { -1, 1 };
                
                    IVec2 bounceDir = { -1, 0 };

                    int blockIndex = GetTileIndex(cable.tile + bounceDir, record.blocks);

                    if (blockIndex >= 0)
                    {
                        BounceBlock(bounceDir, blockIndex);
                    }

                    cable.tile = cable.tile + offset;
                    cable.sprite = GetSprite(SPRITE_DOOR_RIGHT_OPEN);
                
                    break;
                }
                case SPRITE_DOOR_TOP_CLOSE:
                {
                    IVec2 offset = { -1, 1 };

                    IVec2 bounceDir = { 0, 1 };

                    int blockIndex = GetTileIndex(cable.tile + bounceDir, record.blocks);

                    if (blockIndex >= 0)
                    {
                        BounceBlock(bounceDir, blockIndex);
                    }

                    cable.tile = cable.tile + offset;
                    cable.sprite = GetSprite(SPRITE_DOOR_TOP_OPEN);
                
                    break;
                }
                case SPRITE_DOOR_DOWN_CLOSE:
                {
                    IVec2 offset = { 1, -1 };
                
                    IVec2 bounceDir = { 0, -1 };

                    int blockIndex = GetTileIndex(cable.tile + bounceDir, record.blocks);

                    if (blockIndex >= 0)
                    {
                        BounceBlock(bounceDir, blockIndex);
                    }

                    cable.tile = cable.tile + offset;
                    cable.sprite = GetSprite(SPRITE_DOOR_DOWN_OPEN);

                    break;
                }

            }

        }
        end = true;
    }
    else if (cable.type == CABLE_TYPE_CONNECTION_POINT)
    {
        if (!cable.conductive)
        {
            end = true;
        }
        else
        {
            end = false;
        }
    }
    else
    {
        cable.conductive = true;
        cable.sprite = GetSprite(GetCablePowerONID(cable.id));
        end = false;
    }
    
}


void OnSourcePowerOn(std::vector<bool> & visited, int sourceIndex)
{
    bool end = false;

    visited[sourceIndex] = true;

    PowerOnCable(gameState->currentRecord.electricDoorSystem.cables[sourceIndex], end);

    if (end) return;
    
    int indexes[4] =
        {
            gameState->currentRecord.electricDoorSystem.cables[sourceIndex].leftIndex,
            gameState->currentRecord.electricDoorSystem.cables[sourceIndex].rightIndex,
            gameState->currentRecord.electricDoorSystem.cables[sourceIndex].upIndex,
            gameState->currentRecord.electricDoorSystem.cables[sourceIndex].downIndex
        };

    for (int i = 0; i < 4; i++)
    {
        int id = indexes[i];
        if (id >= 0 && !visited[id])
        {
            OnSourcePowerOn(visited, id);
        }
    }

    return;
    
}

bool UpdateCameraPosition()
{
    bool updated = false;
    
    for  (int i = 0; i < gameState->tileMaps.count; i++)
    {
        Map & map = gameState->tileMaps[i];
        IVec2 playerTile = gameState->currentRecord.player.slimes[gameState->currentRecord.player.motherIndex].tile;

        if (playerTile.x > map.tilePos.x && playerTile.x <= (map.tilePos.x + map.width) &&
            playerTile.y > map.tilePos.y && playerTile.y <= (map.tilePos.y + map.height))
        {

            Vector2 pos = TilePositionToPixelPosition(map.width * 0.5f + map.tilePos.x + 0.5f, map.height * 0.5f + map.tilePos.y + 0.5f);
            if (!Vector2Equals(pos, gameState->camera.target))
            {
                if (!map.firstEnter)
                {
                    map.firstEnter = true;
                    map.resetRecord = gameState->currentRecord;
                }
                gameState->currentMapIndex = i;
                gameState->camera.target = pos;
                updated =  true;
            }
            break;
        }
    }
    return updated;
}

void Restart()
{
    undoRecords.push(gameState->currentRecord);
    gameState->currentRecord = gameState->tileMaps[gameState->currentMapIndex].resetRecord;

    animateSlimeCount = 0;
    gameState->animateTime = 0;
    animationPlaying = false;

}



void Slime::Bounce(IVec2 bounceDir)
{
    IVec2 target = tile;

    IVec2 start = target + bounceDir;

    for (IVec2 pos = start; ; pos = pos + bounceDir)
    {
        if (gameState->currentRecord.CheckWalls(pos))
        {
            
            break;
        }
                
        if (CheckOutOfBound(pos))
        {
            Restart();
        }

                
        int blockIndex = GetTileIndex(pos + bounceDir, gameState->currentRecord.blocks);
        
        if (blockIndex != -1)
        {
            break;
        }

        target = pos;
    }

    attachDir = bounceDir;
    tile = target;
    
}


#define GAME_H
#endif
