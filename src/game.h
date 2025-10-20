#if !defined(GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

/*
  NOTE:
  GAME_INTERNAL
   0 - Build for developer only
   1 - Build for pubilc release
*/

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_ANIMATION 50
#define DIST_ONE_TILE MAP_TILE_SIZE
#define BLOCK_ANI_SPEED 20.0f
#define BOUNCE_SPEED 10.0f

#define MAX_ENTITIES 5000

constexpr float zoom_per_tile = 19.0f / 600.0f;
constexpr float pressFreq = 0.2f;


#include "vendor/raylib/raylib.h"
#include "vendor/raylib/raymath.h"

#include "engine_lib.h"
#include "assets.h"
#include "render_interface.h"

#include "electric_door.h"
#include "entity.h"
#include "move_animation.h"

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
    POSSES_KEY,
    MOUSE_LEFT,
    MOUSE_RIGHT,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY,
    DOWN_KEY,
    SPACE_KEY,
    UNDO_KEY,
    RESET_KEY,
    GAME_INPUT_COUNT,
};

struct KeyMapping
{
    Array<int, 3> keys;
};

struct Memory
{
    BumpAllocator * transientStorage;
    BumpAllocator * persistentStorage;
};

struct Arrow
{
    Sprite sprite;
    SpriteID id;

    int tileSize;
    
    Vector2 topLeftPos;
    bool show = true;
};

struct UndoState
{
    int playerIndex;
    std::vector<Entity> undoEntities;    
};

// NOTE: Map data type
struct Map
{
    UndoState initUndoState;

    Entity playerEnter;
    
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

    AnimationController cameraAniController;
    Camera2D camera;

    Texture2D texture;

    ElectricDoorSystem * electricDoorSystem;

    Array<int, MAX_ENTITIES> entityTable[LAYER_COUNT];

    int playerEntityIndex;
        
    KeyMapping keyMappings[GAME_INPUT_COUNT];

    int tileMapCount;
    Map * tileMaps;
    Map * lv2Map;

    Array<Entity, MAX_ENTITIES> entities;
    
    Arrow upArrow, downArrow, leftArrow, rightArrow;
    
    int currentMapIndex = -1;
    
    bool initialized;
};


struct MoveActionResult
{
    bool blocked;
    bool pushed;
    Entity * blockedEntity;
};

// ----------------------------------------------------
// NOTE: Game Globals
// ----------------------------------------------------
// TODO: Very piggy undo stack, each action we push a copy of the entire game entities to the stack
static std::vector<UndoState> undoStack;

static GameState * gameState;
static Memory * gameMemory;

//  ========================================================================
//              NOTE: Game Functions 
//  ========================================================================
inline bool UpdateCamera();

inline void Undo();

inline void Restart();

MoveActionResult MoveActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int accumulatedMass);

bool MoveAction(IVec2 actionDir);

bool SplitAction(IVec2 bounceDir);

inline void DrawSpriteLayer(EntityLayer layer);

inline bool InstancePush(Vector2 pushStart, Vector2 pushedStart);

#define GAME_H
#endif
