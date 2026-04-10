#if !defined(ASSETS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#include "engine_lib.h"

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas-10x.png"
#define ANIMATED_PLAYER_PATH "Assets/Texture/player_ani-10x.png"

#define TileSetCols 10
//  ========================================================================
//              NOTE: Assets Constants
//  ========================================================================

//  ========================================================================
//              NOTE: Assets Structs
//  ========================================================================
struct Sprite
{
    IVec2 altasOffset;
    IVec2 spriteSize;
};

struct SpriteAnimation
{
    bool8 playReverse = false;
    bool8 loop = false;
    real32 secondsPerFrame = 0.5f;
     real32 frameTimer = 0.0f;
    uint32 currentFrame = 0;
    uint32 frameCount;
    Sprite * frames;
};

struct AnimatedSprite
{
    bool8 playing = 0;
    uint32 animationCount;
    uint32 currentAnimation;
      SpriteAnimation * spriteAnimation;
};

struct CreateAnimatedSpriteParam
{
    uint32 textureW;
    uint32 textureH;
    uint32 animationCount;
    uint32 numFrames;
};

//  ========================================================================
//              NOTE: Assets Globals
//  ========================================================================

#define ASSETS_H
#endif
