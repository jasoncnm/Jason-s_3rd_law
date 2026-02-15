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

#define BOUNCE_SPEED 10.0f
#define MOVE_SPEED 5.0f

#define CAMERA_MOVE_SPEED 2.1f
#define CAMERA_ZOOM_SPEED 1.7f

#define MAX_GAMEPAD 5

#define MAX_ENTITIES 9000

#define STAR_COUNT 100

#define GAME_SAVE_PATH "data/save_data/"

#define BLOCK_MOVE_FUNC  nullptr
#define PLAYER_MOVE_FUNC nullptr
#define CAMERA_MOVE_FUNC EaseInOutCubic
#define CAMERA_ZOOM_FUNC EaseInOutCubic

#define MAX_UNDO 500

constexpr float zoom_per_tile = 18.5f / 600.0f;
constexpr float press_freq = 0.2f;
constexpr float cameraSwitchTargetDelay = 1.0f;


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
    
    RECOVER_KEY,
    
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
    Array<int32, 3> keys;
    int32 gamepadButton;
    int32 gamepadAxis;
};


struct Memory
{
    BumpAllocator * transientStorage;
    BumpAllocator * persistentStorage;
};


struct UndoState
{
    struct EntityArray
    {
        uint32 entityCount;
        Entity * entities;
    };
    
    int32 playerIndex;
    // TODO: optimize this by allocate entities 
    //       into our own allocator aka EntityArray
    std::vector<Entity> undoEntities;
};

struct Map
{
    char mapID[100];
    UndoState initUndoState;
    
    IVec2 tilePos;            // Top left tile position of the map
    int32   width;              // Number of tiles in X axis
    int32   height;             // Number of tiles in Y axis
    
    bool8 firstEnter = false;
};

struct UndoStack
{
    
    UndoState undoStack[MAX_UNDO];
    int32 last = 1;
    uint32 count = 0;
    
    UndoState & back()
    {
        return undoStack[last - 1];
    }
    
    void pop_back()
    {
        if (count > 0)
        {
            last--;
            
            if (last <= 0) last = MAX_UNDO;
            
            count--;
            
        }
    }
    
    void push_back(uint32 playerIndex, UndoState::EntityArray & ea)
    {
        last++;
        count++;
        if (last > MAX_UNDO) last = 1;
        if (count > MAX_UNDO) count = MAX_UNDO;
        
        UndoState & state = undoStack[last - 1];
        state.playerIndex = playerIndex;
        state.undoEntities.clear();
        state.undoEntities.insert(state.undoEntities.begin(), &ea.entities[0], &ea.entities[ea.entityCount]);
        
         }
    
    bool empty()
    {
        return count == 0;
    }
    
    void reset()
    {
        count = 0;
        last = 1;
    }
    
};

struct MyCamera
{
    enum FollowState
    {
        LOCK_TO_MAP,
        FOLLOW_WITHIN_MAP,
        FOLLOW_CENTER,
        FOLLOW_ALONG_AXIS,
    };
    
    Camera2D base;
    TweenController tweenController;
    int32 followEntityIndex;
    FollowState followState;
};

// NOTE: GameState
struct GameState
{
    // TODO: Undo Stack is still too big and has very limited max undo steps
    UndoStack undoStack;
    
    MyCamera camera;
    Texture2D texture;
    RenderTexture2D renderTarget;
    
    StarFields starFields;
    
    bool8 enableFX = true;
    bool8 shake = false;
    real32 shakeTime = 0;
    real32 time = 0.0f;
    PostFX postFX[FX_COUNT];
    
    Array<uint16, MAX_ENTITIES> entityTable[LAYER_COUNT];
    Array<Entity, MAX_ENTITIES> entities;
    
    int32 tileMapCount;
    Map tileMaps[500];
    Map * lv2Map;
    
    KeyMapping keyMappings[GAME_INPUT_COUNT];
    
    Color bgColor;
    
    IVec2 tileMin, tileMax;
    
    int32 playerEntityIndex;
    int32 currentMapIndex;
    int32 playerMapIndex;
    int32 lastTutBlockIndex;
    
    int32 screenWidth = SCREEN_WIDTH;
    int32 screenHeight = SCREEN_HEIGHT;
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

struct FindTileMapResult
{
    Map * map;
    int32 mapIndex;
};

// ----------------------------------------------------
// NOTE: Game Globals
// ----------------------------------------------------

static GameState * gameState;
static Memory * gameMemory;

//  ========================================================================
//              NOTE: Game Functions 
//  ========================================================================
MoveActionResult MoveActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int32 accumulatedMass);
PushResult ActionCheck(Entity * startEnt, IVec2 pushDir, CheckType startState);
void CleanUpGame();
void SetShake(float duration);



//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================
#define UPDATE_AND_RENDER(name) void name(GameState * gameStateIn, Memory * gameMemoryIn, bool8 * running)
typedef UPDATE_AND_RENDER(update_and_render);

#define GAME_H
#endif
