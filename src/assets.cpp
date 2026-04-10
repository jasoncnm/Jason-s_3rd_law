#include "assets.h"


//  ========================================================================
//              NOTE: Assets Functions
//  ========================================================================

bool8 PlaySpriteAnimation(SpriteAnimation * spriteAnimation)
{
    bool8 finished = false;
    spriteAnimation->frameTimer += GetFrameTime();
    if (spriteAnimation->frameTimer >= spriteAnimation->secondsPerFrame)
    {
        spriteAnimation->frameTimer = 0;
        if (spriteAnimation->playReverse)
        {
            spriteAnimation->currentFrame--;
            if (spriteAnimation->currentFrame < 0)
            {
                if (spriteAnimation->loop) 
                {
                    spriteAnimation->currentFrame = spriteAnimation->frameCount - 1;
                }
                else
                {
                    spriteAnimation->currentFrame = 0;
                    finished = true;
                }
            }
        }
        else
        {
            spriteAnimation->currentFrame++;
            if (spriteAnimation->currentFrame >= spriteAnimation->frameCount)
            {
                if (spriteAnimation->loop)
                {
                    spriteAnimation->currentFrame = 0;
                }
                else
                {
                    spriteAnimation->currentFrame = spriteAnimation->frameCount - 1;
                    finished = true;
                }
            }
        }
    }
    
    return finished;
}

void PlayAnimatedSprite(AnimatedSprite * animatedSprite)
{
    if (animatedSprite->playing)
    {
        SpriteAnimation * spriteAnimation = animatedSprite->spriteAnimation + animatedSprite->currentAnimation;
        bool8 end = PlaySpriteAnimation(spriteAnimation);
        animatedSprite->playing = !end;
    }
}

Sprite GetCurrentSpriteFrame(AnimatedSprite * animatedSprite)
{
    SpriteAnimation * sa = animatedSprite->spriteAnimation + animatedSprite->currentAnimation;
    return sa->frames[sa->currentFrame];
}

void BeginAnimation(AnimatedSprite * animatedSprite, uint32 animationID, real32 secondsPerFrame,
                    bool8 playReverse, bool8 loop)
{
    animatedSprite->playing = true;
    animatedSprite->currentAnimation = animationID;
    SpriteAnimation * animation = animatedSprite->spriteAnimation + animationID;
    animation->loop = loop;
    animation->playReverse = playReverse;
    animation->secondsPerFrame = secondsPerFrame;
    if (playReverse)
    {
        animation->currentFrame = animation->frameCount - 1;
    }
    else
    {
        animation->currentFrame = 0;
    }
}



AnimatedSprite CreateAnimatedSprite(CreateAnimatedSpriteParam param)
{
    AnimatedSprite animatedSprite = {};
    uint32 textureW = param.textureW;
    uint32 textureH = param.textureH;
    uint32 animationCount = param.animationCount;
    uint32 frames = param.numFrames;
    
    animatedSprite.animationCount = animationCount;
    animatedSprite.currentAnimation = 0;
    animatedSprite.spriteAnimation = 
    (SpriteAnimation *)BumpAllocArray(gameMemory->persistentStorage, animationCount, sizeof(SpriteAnimation));
    
    for (uint32 idx = 0; idx < animationCount; idx++)
    {
        // NOTE: Add Sprite Animation
        SpriteAnimation * spriteAnimation = animatedSprite.spriteAnimation + idx;
        spriteAnimation->frameCount = frames;
        spriteAnimation->frames = 
        (Sprite *)BumpAllocArray(gameMemory->persistentStorage, frames, sizeof(Sprite));
        for (uint32 frame = 0; frame < frames; frame++)
        {
            IVec2 altasOffset = { (int32)frame * (int32)textureW,
                (int32)idx * (int32)textureH };
            IVec2 spriteSize = { (int32)textureW, (int32)textureH };
            spriteAnimation->frames[frame] = { altasOffset, spriteSize };
        }
    }
    return animatedSprite;
}

Sprite GetBrokenGlassSprite()
{
    Sprite sprite = { 0 };
    sprite.altasOffset = {192,128};
    sprite.spriteSize = {32, 32};
    
    return sprite;
}

Sprite GetOpenDoorSprite(IVec2 openDir, bool8 isH)
{
    Sprite sprite = { 0 };
    sprite.spriteSize = { 32, 32 };
    if (isH)
    {
        if (openDir == IVec2 { 1, -1 })
        {
            sprite.altasOffset = { 96, 256 };
        }
        else if (openDir == IVec2 { -1, 1 })
        {
            sprite.altasOffset = { 128, 256 };
        }
        else if (openDir == IVec2 { 1, 1 })
        {
            sprite.altasOffset = { 128, 288 };
        }
        else if (openDir == IVec2 { -1, -1 })
        {
            sprite.altasOffset = { 96, 288 };
        }
        }
    else
    {
        if (openDir == IVec2 { 1, -1 })
        {
            sprite.altasOffset = { 64, 256 };
        }
        else if (openDir == IVec2 { -1, 1 })
        {
            sprite.altasOffset = { 32, 256 };
        }
        else if (openDir == IVec2 { 1, 1 })
        {
            sprite.altasOffset = { 64, 288 };
        }
        else if (openDir == IVec2 { -1, -1 })
        {
            sprite.altasOffset = { 32, 288 };
        }
        }
    
    return sprite;
}

Sprite GetBlockSprite(uint32 mass)
{
    Sprite sprite = { 0 };
    if (mass == 1)
    {
        sprite.altasOffset = { 64, 96 };
    }
    else 
    {
        sprite.altasOffset = { 32, 32 };
    }
    
    sprite.spriteSize = { 32, 32 };
    
    return sprite;
}

Sprite GetSlimeSprite(IVec2 dir)
{
    Sprite sprite = { 0 };
    sprite.spriteSize = { 32, 32 };
    if (dir == IVec2 { 0, 0 })
        {
        sprite.altasOffset = { 32, 320 };
    }
    
    if (dir == IVec2 { -1, 0 })
        {
        sprite.altasOffset = { 128, 320 };
    }
    
    if (dir == IVec2 { 1, 0 })
        {
        sprite.altasOffset = { 160, 320 };
    }
    
    if (dir == IVec2 { 0, -1 })
        {
        sprite.altasOffset = { 64, 320 };
    }
    
    if (dir == IVec2 { 0, 1 })
        {
        sprite.altasOffset = { 96, 320 };
    }
    
    return sprite;
}

Sprite GetSprite(uint32 tileID)
{
    int id = tileID - 1;
    int32 row = id / TileSetCols;
    int32 col = id % TileSetCols;
    
    Sprite sprite = { 0 };
    sprite.altasOffset = IVec2 { col * 32, row * 32 };
    sprite.spriteSize = IVec2 { 32, 32 };
    
    return sprite;
}
