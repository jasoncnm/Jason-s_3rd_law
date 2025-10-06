#if !defined(GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_ANIMATION 50
#define DIST_ONE_TILE MAP_TILE_SIZE
#define BLOCK_ANI_SPEED 20.0f

#define MAX_ENTITIES 5000

#define MAX_UNDO_RECORDS 2000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)

#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "assets.h"
#include "electric_door.h"
#include "entity.h"

// ----------------------------------------------------
// NOTE: Game Structs
// ----------------------------------------------------

using UndoEntities = Array<Entity, MAX_UNDO_RECORDS>;

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

struct ArrowButton
{
    Sprite sprite;
    SpriteID id;

    int tileSize;
    
    Vector2 topLeftPos;
    
    bool seletable = false;
    bool hover = false;
    bool preseed = false;
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

    MoveAnimation cameraAnimation;
    Camera2D camera;

    ElectricDoorSystem * electricDoorSystem = nullptr;

    Array<int, MAX_ENTITIES> entityTable[LAYER_COUNT];
        
    KeyMapping keyMappings[GAME_INPUT_COUNT];

    int tileMapCount = 0;
    Map * tileMaps;
    Map lv2Map;

    Array<Entity, MAX_ENTITIES> entities;

    int playerEntityIndex;
    Array<int, 3> slimeEntityIndices;
    
    ArrowButton upArrow, downArrow, leftArrow, rightArrow;
    
    float animateTime = 0.0f;
    float duration = 1.0f;
    
    int currentMapIndex = 0;
    
    bool initialized = false;
};


struct PushActionResult
{
    bool blocked;
    bool pushed;
    Entity * blockedEntity;
};

// ----------------------------------------------------
// NOTE: Game Globals
// ----------------------------------------------------
static const float pressFreq = 0.2f;
static float timeSinceLastPress = 0;
static std::vector<UndoState> undoStack;

static GameState * gameState;
static Memory * gameMemory;

static bool animationPlaying = false;
static bool repeat = false;

//  ========================================================================
//              NOTE: Game Functions 
//  ========================================================================
inline bool JustPressed(GameInputType type);

inline bool IsDown(GameInputType type);

inline bool CheckOutOfBound(int tileX, int tileY);

inline bool CheckOutOfBound(IVec2 tilePos);

inline bool CheckBounce(IVec2 tilePos, IVec2 pushDir);

PushActionResult PushActionCheck(Entity * startEntity, Entity * pushEntity, IVec2 blockNextPos, IVec2 pushDir, int accumulatedMass);

void BounceEntity(Entity * startEntity, Entity * entity, IVec2 dir);

inline bool UpdateCamera();

inline void Undo();

inline void Restart();

bool MoveAction(IVec2 actionDir);

bool SplitAction(IVec2 bounceDir);

inline void DrawSpriteLayer(EntityLayer layer);


#define GAME_H
#endif
