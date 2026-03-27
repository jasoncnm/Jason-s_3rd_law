#if !defined(GAME_H)

/*
  NOTE:
  GAME_INTERNAL
   0 - Build for developer only
   1 - Build for pubilc release
*/

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000

#define MAP_TILE_SIZE 32       // Tiles size
#define DIST_ONE_TILE MAP_TILE_SIZE

#define DEFAULT_TILE_SIZE Vector2 { MAP_TILE_SIZE, MAP_TILE_SIZE }

#define BOUNCE_SPEED 9.0f
#define MOVE_SPEED 3.3f

#define CAMERA_MOVE_SPEED 1.8f
#define CAMERA_ZOOM_SPEED 1.7f

#define MAX_ENTITIES 9000

#define GAME_SAVE_PATH "data/save_data/"

#define BLOCK_MOVE_FUNC  nullptr
#define PLAYER_MOVE_FUNC nullptr
#define CAMERA_MOVE_FUNC EaseInOutCubic
#define CAMERA_ZOOM_FUNC EaseInOutCubic

#define MAX_UNDO 500

constexpr float zoom_per_tile = 18.5f / 600.0f;
constexpr float press_freq = 0.2f;


#include "raylib.h"
#include "raymath.h"
#include "engine_lib.h"
#include "render_interface.h"
#include "electric_door.h"
#include "entity.h"
#include "tween_controller.h"
#include "game_ui.h"
#include "action_input.h"
#include "assets.h"

// ----------------------------------------------------
// NOTE: Game Structs
// ----------------------------------------------------


enum GameScreen
{
    TITLE_SCREEN,
    MENU_SCREEN,
    GAME_MAIN_SCREEN,
    GAME_TUT_SCREEN,
    PAUSE_MENU_SCREEN,
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
    int32 starCount;
    // NOTE: using vector for dynamic heap allocations
    std::vector<Entity> undoEntities;
    
    Entity * GetByEntityIndex(int32 entityIndex);
    };

struct Map
{
    char mapID[100];
    UndoState resetState;
    
    IVec2 tilePos;            // Top left tile position of the map
    int32   width;              // Number of tiles in X axis
    int32   height;             // Number of tiles in Y axis
    int32 visibleStarCount = 0;
    
    bool8 stateInitilized = false;
    };

struct UndoStack
{
    
    UndoState undoStack[MAX_UNDO];
    int32 last = 1;
    uint32 count = 0;
    
    UndoState & back();
    
    void pop_back();
    
    void push_back(uint32 playerIndex, uint32 starCount, UndoState::EntityArray & ea);
        
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
    
    Vector2 moveDir = { 0, 0 };
    Camera2D base;
    TweenController tweenController;
    int32 followEntityIndex;
    FollowState followState;
};

struct SceneData
{
    char * levelPath;
    UndoState entityState;
    };

// NOTE: GameState
struct GameState
{
    UndoStack undoStack;
    
    MyCamera camera;
    Texture2D texture;
    Texture2D playerTexture;
    
    RenderTexture2D renderTarget;
    
    StarFields starFields;
    
    bool8 shake = false;
    real32 shakeTime = 0;
    real32 time = 0.0f;
    //PostFX postFX[FX_COUNT];
    ShaderInfo postShader;
    ShaderInfo movableShader;
    
    Array<uint16, MAX_ENTITIES> entityTable[LAYER_COUNT];
    Array<Entity, MAX_ENTITIES> entities;
    
    uint32 tileMapCount;
    Map tileMaps[500];
    Map * lv2Map;
    
    Input input;
    Color bgColor;
    
    IVec2 tileMin, tileMax;
    
    int32 playerEntityIndex;
    
    // NOTE: map index of the map containing camera follow entity current tilePos
    int32 currentMapIndex;
    // NOTE: map index of the map containing camera follow entity prev tilePos
    int32 prevMapIndex;
    // NOTE: map index of the map containing player tilePos
    int32 playerMapIndex;
    int32 lastTutBlockIndex;
    int32 starCount = 0;
    
    int32 screenWidth = SCREEN_WIDTH;
    int32 screenHeight = SCREEN_HEIGHT;
    GameScreen currentScreen = TITLE_SCREEN;
    UndoState lastState;
    
    bool8 initialized;
    bool8 simulating = false;
    
    bool8 switching = false;
    GameScreen nextScreen;
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
FindTileMapResult FindTileMap(IVec2 tilePos);
void InitUndoState(UndoState * undoState, 
                   uint32 playerIndex, uint32 starCount, UndoState::EntityArray & ea);



//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================
#define UPDATE_AND_RENDER(name) void name(GameState * gameStateIn, Memory * gameMemoryIn, bool8 * running)
typedef UPDATE_AND_RENDER(update_and_render);

#define GAME_H
#endif
