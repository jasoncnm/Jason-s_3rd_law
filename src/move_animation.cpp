/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "move_animation.h"

void AdjustAnimatingSpeed(Entity * entity, float ratio)
{
#if 0
    for (int animationIndex = 0; animationIndex < entity->moveAniQueue.count; animationIndex++)
    {
        MoveAnimation & ani = entity->moveAniQueue[animationIndex];
        ani.move_dt *= ratio;
    }
#endif    
}

MoveAnimation GetMoveAnimation(float (*Easing)(float), Vector2 moveStart, Vector2 moveEnd,
                               float animateSpeed = 5.0f, float target_t = 1.0f)
{
    
    MoveAnimation ani;
    ani.move_t = 0;
    ani.move_target_t = target_t;
    ani.move_dt = animateSpeed;
    ani.moveStart = moveStart;
    ani.moveEnd = moveEnd;
    ani.Easing = Easing;
    
    return ani;
}

Vector2 MoveAnimation::GetPosition()
{
    SM_ASSERT(move_target_t, "Divide by zero");
    
    float t = move_t / move_target_t;

    if (Easing)
    {
        t = Easing(t);
    }

    Vector2 result = Vector2Add(moveStart, Vector2Scale(Vector2Subtract(moveEnd, moveStart), t));
    
    return result;
}

bool MoveAnimation::Update()
{
    bool end = false;
    float delta = GetFrameTime() * move_dt;

    if (move_t < move_target_t)
    {
        move_t += delta;
    }

    if (move_t > move_target_t)
    {
        move_t = move_target_t;
        end = true;
    }

    return end;
}
