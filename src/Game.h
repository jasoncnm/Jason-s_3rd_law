#if !defined(GAME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#define MAP_TILE_SIZE 64       // Tiles size

#define MAX_SLIME 50
#define MAX_QUEUE 100
#define MAX_BLOCKS 100
#define MAX_WALLS 100
#define MAX_GOALS 100
#define MAX_ANIMATION 100

#define MAX_UNDO_RECORDS 1000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas.png"
#define LEVELS_PATH "Assets/Level/levels.lv"

#include "assets.h"
#include "animation.h"

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
    unsigned int goalX;
    unsigned int goalY;
};

// NOTE: PushBlock
struct Block
{
    unsigned int blockX;
    unsigned int blockY;
    Color color;
};

// NOTE: Wall
struct Wall
{
    unsigned int wallX;
    unsigned int wallY;
};

struct ArrowButton
{
    Sprite sprite;
    SpriteID id;
    int tileSize = 32;

    Vector2 topLeftPos;

    bool seletable = false;
    bool hover = false;
    bool preseed = false;
    bool show = true;
};


struct Slime
{
    int tileX;
    int tileY;

    Vector2 pivot;

    float tileSize;
    int mass = 3;

    bool show = true;
};

struct SlimeAnimation
{
    
    Slime startSlime;
    Slime endSlime;

    Slime * currentSlime;

    float mergetime = 1.0f;
    
    bool merge;
    int possessed;

    Slime * mergedSlime;
    Slime * possesserSlime;
};

struct Player
{
    
    int moveSteps;
    int childrenCount;

    float maxTileSize;
    Color color = YELLOW;

    Slime mother;
    Slime children[MAX_SLIME];
};

struct Record
{

    Player player;

    unsigned int blockCount;
    unsigned int blockSize;
    Block blocks[MAX_BLOCKS];

    unsigned int wallCount;
    unsigned int wallSize;
    Wall walls[MAX_WALLS];

    unsigned int goalCount;
    unsigned int goalSize;
    Goal goals[MAX_GOALS];

    ArrowButton upArrow, downArrow, leftArrow, rightArrow;
    
};


// NOTE: GameState
struct GameState
{
    State state;

    Camera2D camera;

    Map tileMap;

    Record currentRecord;

    KeyMapping keyMappings[GAME_INPUT_COUNT];


    float animateTime = 0.0f;
    float duration = 1.0f;
    
};

bool animationPlaying = false;
int animateSlimeCount = 0;
SlimeAnimation animateSlimes[MAX_SLIME] = { 0 };

long long levelsTimestamp;



#define GAME_H
#endif
