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
    SPRITE_COUNT,
};

struct Sprite
{
    IVec2 altasOffset;
    IVec2 spriteSize;
};

//  ========================================================================
//              NOTE: ender Functions
//  ========================================================================
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
            sprite.altasOffset = { 16, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_UP:
        {
            sprite.altasOffset = { 80, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_DOWN:
        {
            sprite.altasOffset = { 48, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_RIGHT:
        {
            sprite.altasOffset = { 112, 0 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_LEFT_HOVER:
        {
            sprite.altasOffset = { 16, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_UP_HOVER:
        {
            sprite.altasOffset = { 80, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_DOWN_HOVER:
        {
            sprite.altasOffset = { 48, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_ARROW_RIGHT_HOVER:
        {
            sprite.altasOffset = { 112, 64 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
    }

    return sprite;
}


#define ASSETS_H
#endif
