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

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_ANIMATION 50
#define DIST_ONE_TILE MAP_TILE_SIZE
#define BLOCK_ANI_SPEED 20.0f
#define BOUNCE_SPEED 10.0f
#define SLIME_MOVE_SPEED 4.0f

#define MAX_GAMEPAD 5

#define MAX_ENTITIES 10000

#define STAR_COUNT 500

#define GAME_SAVE_PATH "data/save_data/"

#define BLOCK_MOVE_FUNC nullptr
#define PLAYER_MOVE_FUNC EaseOutCubic
#define CAMERA_MOVE_FUNC EaseOutCubic
#define CAMERA_ZOOM_FUNC EaseInOutCubic

constexpr float zoom_per_tile = 19.0f / 600.0f;
constexpr float press_freq = 0.2f;


#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "assets.h"
#include "render_interface.h"

#include "electric_door.h"
#include "entity.h"
#include "tween_controller.h"
#include "game_ui.h"


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
    MENU_SCREEN,
    GAME_MAIN_SCREEN,
    GAME_TUT_SCREEN,
    PAUSE_MENU_SCREEN,
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



struct UndoState
{
    int playerIndex;
    std::vector<Entity> undoEntities;    
};


struct Map
{
    char mapID[100];
    UndoState initUndoState;
    
    Entity playerEnter;
    
    IVec2 tilePos;            // Top left tile position of the map
    int   width;              // Number of tiles in X axis
    int   height;             // Number of tiles in Y axis
    
    bool8 firstEnter = false;
};

// NOTE: GameState
struct GameState
{
    Camera2D camera;
    TweenController cameraTweenController;
    
    Texture2D texture;
    
    ElectricDoorSystem electricDoorSystem;
    
    StarFields starFields;
    
    Array<int, MAX_ENTITIES> entityTable[LAYER_COUNT];
    Array<Entity, MAX_ENTITIES> entities;
    
    Arrow upArrow, downArrow, leftArrow, rightArrow;
    
    int tileMapCount;
    Map tileMaps[500];
    Map * lv2Map;
    
    KeyMapping keyMappings[GAME_INPUT_COUNT];
    
    Color bgColor;
    
    IVec2 tileMin, tileMax;
    
    int playerEntityIndex;
    int currentMapIndex;
    int screenWidth = SCREEN_WIDTH;
    int screenHeight = SCREEN_HEIGHT;
    GameScreen currentScreen = TITLE_SCREEN;
    UndoState lastState;
    
    bool8 initialized;
    bool8 simulating = false;
    
};

struct MoveActionResult
{
    bool8 blocked;
    bool8 pushed;
    bool8 merged;
    Entity * blockedEntity;
};
enum PushState
{
    PUSH_NONE,
    PUSH_MOVED,
    PUSH_BLOCKED,
    PUSH_MERGED,
    PROJECT_DEFERRED,
};

enum CheckType
{
    CHECK_NONE,
    CHECK_MOVE,
    CHECK_PROJECT,
};

struct PushResult
{
    bool8 pushing;
    PushState state;
    Entity * blockedEntity;
    Entity * mergeEntity;
};

struct CheckThings
{
    bool visited;
    IVec2 pushDir;
    CheckType checkType;
    Entity * pushEnt;
    PushResult pushResult;
    CheckThings * parent;
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
MoveActionResult MoveActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int accumulatedMass);
PushResult ActionCheck(Entity * startEnt, IVec2 pushDir, CheckType startState);
void CleanUpGame();



//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================
#define UPDATE_AND_RENDER(name) void name(GameState * gameStateIn, Memory * gameMemoryIn, bool8 * running)
typedef UPDATE_AND_RENDER(update_and_render);

#define GAME_H
#endif
