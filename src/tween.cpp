/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "tween.h"

#if 0
void AdjustAnimatingSpeed(Entity * entity, float ratio)
{
    for (int animationIndex = 0; animationIndex < entity->moveAniQueue.count; animationIndex++)
    {
        MoveAnimation & ani = entity->moveAniQueue[animationIndex];
        ani.move_dt *= ratio;
    }
}
#endif

Tween CreateTween(TweenParams params, float (*Easing)(float), float animateSpeed, float target_t)
{
    Tween tween;
    tween.params = params;
    tween.t = 0;
    tween.target_t = target_t;
    tween.dt = animateSpeed;
    tween.Easing = Easing;

    return tween;
}


bool8 Tween::UpdateEntityVal()
{
    SM_ASSERT(target_t, "Divide by zero");

    bool8 end = Update();
    
    float current_t = t / target_t;

    if (Easing)
    {
        current_t = Easing(current_t);
    }

    // A + (B - A) * t
    switch (params.paramType)
    {
        case PARAM_TYPE_FLOAT: 
        {
            SM_ASSERT(params.realF, "want to change realF but is null");
            float value = params.startF + (params.endF - params.startF) * current_t;
            *params.realF = (value);
            break;
        }
        case PARAM_TYPE_VECTOR2:
        {
            SM_ASSERT(params.realVec2, "want to change realVec2 but is null");
            Vector2 value = Vector2Add(params.startVec2, Vector2Scale(Vector2Subtract(params.endVec2, params.startVec2), current_t));
            // value.x = floorf(value.x);
            // value.y = floorf(value.y);
            *params.realVec2 = value;
            break;
        }
    }
    
    return end;

}
