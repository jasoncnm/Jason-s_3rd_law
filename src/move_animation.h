#if !defined(MOVEANIMATION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

enum AnimationState
{
    ASTATE_NONE,
    ASTATE_START,
    ASTATE_PLAYING,
    ASTATE_END,
    ASTATE_COUNT,
};

struct MoveAnimation
{
    bool playing = false;
    Vector2 moveStart;
    Vector2 moveEnd;

    float move_t = 0;
    float move_target_t = 1;
    float move_dt;

    float delay;

    void Update();
    Vector2 GetPosition();

    float (*Easing)(float);
    
};

using MoveAnimationQueue = Array<MoveAnimation, 10>;

void AddMoveAnimateQueue(MoveAnimationQueue & queue, MoveAnimation ani);

bool IsAnimationPlaying(Entity * entity);

void AdjustAnimatingSpeed(Entity * entity, float ratio);

MoveAnimation GetMoveAnimation(float (*Easing)(float), Vector2 moveStart, Vector2 moveEnd,
                               float animateSpeed, float target_t, bool startPlay, float delay);

float GetDelay(Entity * pushedEntity, Entity * pushEntity, float speed, IVec2 pushDir);

    
#define MOVEANIMATION_H
#endif
