#if !defined(GAME_H)

/*
  NOTE:
  GAME_INTERNAL
   0 - Build for developer only
   1 - Build for pubilc release
*/

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000

#define ENTITY_TILE_VISIBILITY  3   // Player can see 2 tiles around its position

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
    
    struct MapUndoInfo
    {
        uint32 mapIndex;
        bool8 initilized;
    };
    
    int32 playerIndex;
    int32 starCount;
    // NOTE: using vector for dynamic heap allocations
    std::vector<Entity> undoEntities;
    std::vector<MapUndoInfo> undoMapInfos; 
    
    Entity * GetByEntityIndex(uint32 entityIndex);
    };

struct UndoStack
{
    
    UndoState undoStack[MAX_UNDO];
    int32 last = 1;
    uint32 count = 0;
    
    UndoState & back();
    
    void pop_back();
    
    void push_back(uint32 playerIndex, uint32 starCount,
                   DynamicArray<Entity> & ea,
                   DynamicArray<UndoState::MapUndoInfo> & undoMapInfos);
        
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

struct Map
{
    char mapID[100];
    UndoState resetState;
    
    IVec2 tilePos;            // Top left tile position of the map
    int32   width;              // Number of tiles in X axis
    int32   height;             // Number of tiles in Y axis
    uint16 visibleStarCount = 0;
    
    bool8 stateInitilized = false;
};

struct Fog
{
    bool8 initialized = false;
    IVec2 tileMin;
    IVec2 tileMax;
    IVec2 dim;
    // NOTE: Render texture of Fog of War
    RenderTexture2D fogRenderTex;
    // NOTE: Render texture of one tile of Fog
    RenderTexture2D fogTex;
    
      uint8 * fogTiles;
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
    
    Array<uint32, MAX_ENTITIES> entityTable[LAYER_COUNT];
    Array<Entity, MAX_ENTITIES> entities;
    
    uint32 tileMapCount;
    Map tileMaps[500];
    Map * lv2Map;
    
    Fog fog;
    
    Input input;
    Color bgColor;
    
    IVec2 tileMin, tileMax;
    
    uint32 playerEntityIndex;
    
    // NOTE: map index of the map containing camera follow entity current tilePos
    int32 currentMapIndex;
    // NOTE: map index of the map containing camera follow entity prev tilePos
    int32 prevMapIndex;
    // NOTE: map index of the map containing player tilePos
    int32 playerMapIndex;
    int32 lastTutBlockIndex;
    int32 screenWidth = SCREEN_WIDTH;
    int32 screenHeight = SCREEN_HEIGHT;
    
    GameScreen currentScreen = TITLE_SCREEN;
    UndoState lastState;
    
    uint16 starCount = 0;
    
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
MoveActionResult MoveActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, uint32 accumulatedMass);
PushResult ActionCheck(Entity * startEnt, IVec2 pushDir, CheckType startState);
void CleanUpGame();
void SetShake(float duration);
FindTileMapResult FindTileMap(IVec2 tilePos);
void InitUndoState(UndoState * undoState, 
                   uint32 playerIndex, 
                   uint32 starCount,
                    DynamicArray<Entity> & entityArray,
                   DynamicArray<UndoState::MapUndoInfo> & undoMapInfos);



//  ========================================================================
//              NOTE: Game Functions (exposed)
//  ========================================================================
#define UPDATE_AND_RENDER(name) void name(GameState * gameStateIn, Memory * gameMemoryIn, bool8 * running)
typedef UPDATE_AND_RENDER(update_and_render);

#define GAME_H
#endif
