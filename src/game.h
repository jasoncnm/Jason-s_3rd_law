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

#define MAX_GAMEPAD 5

#define MAX_ENTITIES 5000

#define FIX_TIME_STEP_MS 20.0f

constexpr float zoom_per_tile = 19.0f / 600.0f;
constexpr float press_freq = 0.2f;


#include "vendor/raylib/raylib.h"
#include "vendor/raylib/raymath.h"

#include "engine_lib.h"
#include "assets.h"
#include "render_interface.h"

#include "electric_door.h"
#include "entity.h"
#include "tween_controller.h"

// ----------------------------------------------------
// NOTE: Game Structs
// ----------------------------------------------------

enum GameInputType 
{
    NO_INPUT,
    MOUSE_LEFT,
    MOUSE_RIGHT,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY,
    DOWN_KEY,

    POSSES_KEY,
    SPLIT_KEY,

    UNDO_KEY,
    RESET_KEY,

    ANY_KEY,
    GAME_INPUT_COUNT,
};


enum GameScreen
{
    TITLE_SCREEN,
    GAMEPLAY_SCREEN,
    ENDING_SCREEN,
};


struct KeyMapping
{
    Array<int, 3> keys;
    int gamepadButton;
    int gamepadAxis;
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
    Camera2D camera;
    TweenController cameraTweenController;

    Texture2D texture;

    ElectricDoorSystem * electricDoorSystem;

    Array<int, MAX_ENTITIES> entityTable[LAYER_COUNT];
    Array<Entity, MAX_ENTITIES> entities;
    
    Arrow upArrow, downArrow, leftArrow, rightArrow;

    Map * tileMaps;
    Map * lv2Map;

    KeyMapping keyMappings[GAME_INPUT_COUNT];

    IVec2 tileMin, tileMax;
    
    int tileMapCount;
    int playerEntityIndex;
    int currentMapIndex = -1;
    int screenWidth = SCREEN_WIDTH;
    int screenHeight = SCREEN_HEIGHT;
    
    bool initialized;
};


struct MoveActionResult
{
    bool blocked;
    bool pushed;
    bool merged;
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

inline void DrawSpriteLayers(EntityLayer * layers, int arrayCount);

#define GAME_H
#endif
