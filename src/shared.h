#include "raylib/raylib.h"

#define MAP_TILE_SIZE 32       // Tiles size
#define MAX_ANIMATION 50
#define DIST_ONE_TILE MAP_TILE_SIZE
#define BLOCK_ANI_SPEED 20.0f

#define MAX_ENTITIES 5000

#define MAX_UNDO_RECORDS 2000    // IMPORTANT: This might need to be handle if released. I'm Using std::stack for now (dynamic alloc)


typedef struct GameState
{
    int veryCoolGameData;
} GameState;
