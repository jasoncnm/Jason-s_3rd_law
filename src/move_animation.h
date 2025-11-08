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
    Vector2 moveStart;
    Vector2 moveEnd;

    float move_t = 0;
    float move_target_t = 1;
    float move_dt;

    bool Update();
    Vector2 GetPosition();

    float (*Easing)(float);
    
};

void AdjustAnimatingSpeed(Entity * entity, float ratio);

MoveAnimation GetMoveAnimation(float (*Easing)(float), Vector2 moveStart, Vector2 moveEnd,
                               float animateSpeed = 5.0f, float target_t = 1.0f);

    
#define MOVEANIMATION_H
#endif
