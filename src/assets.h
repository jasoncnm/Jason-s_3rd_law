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
    SPRITE_BLOCK_2,
    SPRITE_GLASS,
    SPRITE_GLASS_BROKEN,
    SPRITE_SLIME_1,
    SPRITE_PIT,

    SPRITE_CABLE_CONNECTION,

    SPRITE_CABLE_H_ON,  // IMPORTANT: Cable ON START DO NOT CHANGE ORDER
    SPRITE_CABLE_V_ON,
    SPRITE_CABLE_DOWN_RIGHT_ON,
    SPRITE_CABLE_DOWN_LEFT_ON,
    SPRITE_CABLE_UP_RIGHT_ON,
    SPRITE_CABLE_UP_LEFT_ON,
        
    SPRITE_SOURCE_H_ON,
    SPRITE_SOURCE_V_ON,
    SPRITE_SOURCE_UP_RIGHT_ON,
    SPRITE_SOURCE_UP_LEFT_ON,
    SPRITE_SOURCE_DOWN_RIGHT_ON,
    SPRITE_SOURCE_DOWN_LEFT_ON,

    SPRITE_SOURCE_RIGHT_ON,
    SPRITE_SOURCE_LEFT_ON,
    SPRITE_SOURCE_DOWN_ON,
    SPRITE_SOURCE_UP_ON, // IMPORTANT: Cable ON END


    SPRITE_CABLE_H_OFF,  // IMPORTANT: Cable OFF START DO NOT CHANGE ORDER
    SPRITE_CABLE_V_OFF,
    SPRITE_CABLE_DOWN_RIGHT_OFF,
    SPRITE_CABLE_DOWN_LEFT_OFF,
    SPRITE_CABLE_UP_RIGHT_OFF,
    SPRITE_CABLE_UP_LEFT_OFF,
        
    SPRITE_SOURCE_H_OFF,
    SPRITE_SOURCE_V_OFF,
    SPRITE_SOURCE_UP_RIGHT_OFF,
    SPRITE_SOURCE_UP_LEFT_OFF,
    SPRITE_SOURCE_DOWN_RIGHT_OFF,
    SPRITE_SOURCE_DOWN_LEFT_OFF,

    SPRITE_SOURCE_RIGHT_OFF,
    SPRITE_SOURCE_LEFT_OFF,
    SPRITE_SOURCE_DOWN_OFF,
    SPRITE_SOURCE_UP_OFF, // IMPORTANT: CABLE OFF END DO NOT CHANGE ORDER

    SPRITE_DOOR_LEFT_CLOSE,
    SPRITE_DOOR_RIGHT_CLOSE,
    SPRITE_DOOR_TOP_CLOSE,
    SPRITE_DOOR_DOWN_CLOSE,

    SPRITE_COUNT,

    
    SPRITE_DOOR_LEFT_OPEN = SPRITE_DOOR_TOP_CLOSE,
    SPRITE_DOOR_RIGHT_OPEN = SPRITE_DOOR_DOWN_CLOSE,
    SPRITE_DOOR_TOP_OPEN = SPRITE_DOOR_LEFT_CLOSE,
    SPRITE_DOOR_DOWN_OPEN = SPRITE_DOOR_RIGHT_CLOSE,

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

SpriteID GetCablePowerONID(SpriteID spriteID)
{
    SM_ASSERT((spriteID >= SPRITE_CABLE_H_OFF) && (spriteID <= SPRITE_SOURCE_UP_OFF),  "Given sprite is not a cable");

    return (SpriteID)(SPRITE_CABLE_H_ON + (spriteID - SPRITE_CABLE_H_OFF));
}

Sprite GetSprite(SpriteID spriteID)
{
    SM_ASSERT((spriteID >= SPRITE_WHITE) && (spriteID < SPRITE_COUNT), "SpriteID out of range");
    Sprite sprite = {};

    switch (spriteID)
    {
        case SPRITE_WHITE:
        {
            sprite.altasOffset = { 0, 0 };
            sprite.spriteSize = { 1, 1 };
            break;
        }
        case SPRITE_PIT:
        {
            sprite.altasOffset = { 128, 96 };
            sprite.spriteSize = { 32, 32 };
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
        case SPRITE_BLOCK_2:
        {
            sprite.altasOffset = { 32, 32 };
            sprite.spriteSize  = { 32, 32 };

            break;
        }
        case SPRITE_SLIME_1:
        {
            sprite.altasOffset = { 32, 96 };
            sprite.spriteSize  = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_H_ON:
        {
            sprite.altasOffset = { 192, 0 };
            sprite.spriteSize = { 32, 32 };

            break;
        }
        case SPRITE_CABLE_V_ON:
        {
            sprite.altasOffset = { 160, 32 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_DOWN_RIGHT_ON:
        {
            sprite.altasOffset = { 160, 0 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_DOWN_LEFT_ON:
        {
            sprite.altasOffset = { 224, 0 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_UP_RIGHT_ON:
        {
            sprite.altasOffset = { 160, 64 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_UP_LEFT_ON:
        {
            sprite.altasOffset = { 224, 64 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_RIGHT_ON:
        {
            sprite.altasOffset = { 160, 96 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_LEFT_ON:
        {
            sprite.altasOffset = { 192, 96 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_ON:
        {
            sprite.altasOffset = { 224, 96 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_ON:
        {
            sprite.altasOffset = { 224, 128 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_DOWN_RIGHT_OFF:
        {
            sprite.altasOffset = { 160, 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_DOWN_LEFT_OFF:
        {
            sprite.altasOffset = { 224, 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_UP_RIGHT_OFF:
        {
            sprite.altasOffset = { 160, 64 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_UP_LEFT_OFF:
        {
            sprite.altasOffset = { 224, 64 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_RIGHT_OFF:
        {
            sprite.altasOffset = { 160, 96 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_LEFT_OFF:
        {
            sprite.altasOffset = { 192, 96 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_OFF:
        {
            sprite.altasOffset = { 224, 96 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_OFF:
        {
            sprite.altasOffset = { 224, 128 + 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_CONNECTION:
        {
            sprite.altasOffset = { 32, 224 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_CABLE_H_OFF:
        {
            sprite.altasOffset = { 192, 160 };
            sprite.spriteSize = { 32, 32 };

            break;
        }
        case SPRITE_CABLE_V_OFF:
        {
            sprite.altasOffset = { 160, 192 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_DOOR_LEFT_CLOSE:
        {
            sprite.altasOffset = { 32, 256 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_DOOR_RIGHT_CLOSE:
        {
            sprite.altasOffset = { 32 * 2, 256 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_DOOR_TOP_CLOSE:
        {
            sprite.altasOffset = { 32 * 3, 256 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_DOOR_DOWN_CLOSE:
        {
            sprite.altasOffset = { 32 * 4, 256 };
            sprite.spriteSize = { 32, 32 };
            break;

        }
        case SPRITE_SOURCE_H_ON:
        {
            sprite.altasOffset = { 288, 64 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_V_ON:
        {
            sprite.altasOffset = { 256, 64 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_RIGHT_ON:
        {
            sprite.altasOffset = { 256, 32 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_LEFT_ON:
        {
            sprite.altasOffset = { 288, 32 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_RIGHT_ON:
        {
            sprite.altasOffset = { 256, 0 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_LEFT_ON:
        {
            sprite.altasOffset = { 288, 0 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_H_OFF:
        {
            sprite.altasOffset = { 288, 224 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_V_OFF:
        {
            sprite.altasOffset = { 256, 224 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_RIGHT_OFF:
        {
            sprite.altasOffset = { 256, 192 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_UP_LEFT_OFF:
        {
            sprite.altasOffset = { 288, 192 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_RIGHT_OFF:
        {
            sprite.altasOffset = { 256, 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_SOURCE_DOWN_LEFT_OFF:
        {
            sprite.altasOffset = { 288, 160 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_GLASS:
        {
            sprite.altasOffset = { 160, 128 };
            sprite.spriteSize = { 32, 32 };
            break;
        }
        case SPRITE_GLASS_BROKEN:
        {
            sprite.altasOffset = { 192, 128 };
            sprite.spriteSize = { 32, 32 };
            break;
        }

    }

    return sprite;
}


#define ASSETS_H
#endif
