#if !defined(ASSETS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#include "engine_lib.h"

//  ========================================================================
//              NOTE: Assets Constants
//  ========================================================================


//  ========================================================================
//              NOTE: Assets Structs
//  ========================================================================
enum SpriteID
{
    SPRITE_WHITE,
    SPRITE_ARROW_UP,
    SPRITE_ARROW_DOWN,
    SPRITE_ARROW_LEFT,
    SPRITE_ARROW_RIGHT,
    SPRITE_ARROW_UP_HOVER,
    SPRITE_ARROW_DOWN_HOVER,
    SPRITE_ARROW_LEFT_HOVER,
    SPRITE_ARROW_RIGHT_HOVER,
    SPRITE_WALL,
    SPRITE_BLOCK,
    SPRITE_SLIME_1,
    SPRITE_COUNT,
};

struct Sprite
{
    IVec2 altasOffset;
    IVec2 spriteSize;
};

//  ========================================================================
//              NOTE: Assets Functions
//  ========================================================================
SpriteID GetArrowHoverSpriteID(SpriteID spriteID)
{
    SM_ASSERT((spriteID >= SPRITE_ARROW_UP) && (spriteID <= SPRITE_ARROW_RIGHT), "Given sprite is not an arrow");

    return (SpriteID)(SPRITE_ARROW_UP_HOVER + (spriteID - SPRITE_ARROW_UP));    

}

Sprite GetSprite(SpriteID spriteID)
{
    Sprite sprite = {};

    switch (spriteID)
    {
        case SPRITE_WHITE:
        {
            sprite.altasOffset = { 0, 0 };
            sprite.spriteSize = { 1, 1 };
            break;
        }
        case SPRITE_ARROW_LEFT:
        {
            sprite.altasOffset = { 32, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_UP:
        {
            sprite.altasOffset = { 96, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_DOWN:
        {
            sprite.altasOffset = { 64, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_RIGHT:
        {
            sprite.altasOffset = { 128, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_LEFT_HOVER:
        {
            sprite.altasOffset = { 32, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_UP_HOVER:
        {
            sprite.altasOffset = { 96, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_DOWN_HOVER:
        {
            sprite.altasOffset = { 64, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_RIGHT_HOVER:
        {
            sprite.altasOffset = { 128, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_WALL:
        {
            sprite.altasOffset = { 96, 96 };
            sprite.spriteSize  = { 32, 32 };
            break;
        }
        case SPRITE_BLOCK:
        {
            sprite.altasOffset = { 64, 96 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_SLIME_1:
        {
            sprite.altasOffset = { 32, 96 };
            sprite.spriteSize  = { 32, 32 };
            break;
        }

    }

    return sprite;
}


#define ASSETS_H
#endif
