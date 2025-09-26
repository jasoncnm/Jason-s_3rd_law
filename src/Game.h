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
#define MAX_ANIMATION 50

#define MAX_UNDO_RECORDS 2000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)
                               
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "engine_lib.h"
#include "assets.h"
#include "game_util.h"
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
    
    Vector2 topLeftPos;
    
    bool seletable = false;
    bool hover = false;
    bool preseed = false;
    bool show = true;
};

// NOTE: Map data type
struct Map
{
    std::vector<Entity> initEntities;

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
    
    Camera2D camera;

    ElectricDoorSystem * electricDoorSystem = nullptr;

    Array<int, 5000> entityTable[LAYER_COUNT];
        
    KeyMapping keyMappings[GAME_INPUT_COUNT];

    int tileMapCount = 0;
    Map * tileMaps;

    Array<Entity, 5000> entities;
    Array<Entity, 50> movedEntities;

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
static float timeSinceLastPress = 0.0f;
static const float pressFreq = 0.2f;
static std::vector<std::vector<Entity>> undoStack;

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

void BounceEntity(Entity * entity, IVec2 dir);

inline bool UpdateCameraPosition();

inline void Undo();

inline void Restart();

bool MoveAction(IVec2 actionDir);

bool SplitAction(IVec2 bounceDir);

inline void DrawSpriteLayer(EntityLayer layer);


#define GAME_H
#endif
