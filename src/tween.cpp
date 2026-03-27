/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "tween.h"

Tween CreateTween(TweenParams params, float (*Easing)(float), float animateSpeed, float target_t)
{
    Tween tween;
    tween.params = params;
    tween.t = 0;
    tween.target_t = target_t;
    tween.dt = animateSpeed;
    tween.Easing = Easing;
    tween.play = false;

    return tween;
}

void Tween::Reset()
{
    t = target_t;
    UpdateEntityVal();
}

 void Tween::UpdateEntityVal()
{
    SM_ASSERT(target_t, "Divide by zero");
    if (t == 0)
    {
        HandleEvents(startEvents);
    }
    
    float delta = GetFrameTime() * dt;
    
    if (t < target_t)
    {
        t += delta;
    }
    
    if (t > target_t)
    {
        HandleEvents(endEvents);
        t = target_t;
    }
    
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
        case PARAM_TYPE_COLOR:
        {
            SM_ASSERT(params.realColor, "want to change realColor but is null");
            Color color = ColorLerp(params.startColor, params.endColor, current_t);
            *params.realColor = color;
            break;
        }
        case PARAM_TYPE_INT:
        {
            // TODO
        }
    }
    
}


void * Tween::GetTweeningValue()
{
    switch (params.paramType)
    {
        case PARAM_TYPE_FLOAT: 
        {
            return params.realF; 
            }
        case PARAM_TYPE_VECTOR2:
        {
            return params.realVec2;
            }
        case PARAM_TYPE_COLOR:
        {
            return params.realColor;
            }
        case PARAM_TYPE_INT:
        {
            // TODO
        }
    }
    return nullptr;
    }


void HandleEvents(Array<TweenEvent, MAX_EVENT> & events)
{
    for (uint32 i = 0; i < events.count; i++)
    {
        TweenEvent & event = events[i];
    if (event.controller)
    {
        OnPlayEvent(event.controller);
    }
    
    if (event.deleteEntity)
    {
        OnDeleteEvent(event.deleteEntity);
    }
    
    if (event.breakEntity)
    {
        OnBreakGlass(event.breakEntity);
    }
    
        event.Reset();
    }
}

void ResetEvents(Array<TweenEvent, MAX_EVENT> & events)
{
    for (uint32 i = 0; i < events.count; i++)
    {
        events[i].Reset();
    }
}

void OnPlayEvent(TweenController * controller)
{
    SM_ASSERT(controller, "controller is null");
    controller->EnableAddedTweens();
    controller->start = true;
    
    }


void OnDeleteEvent(Entity * deleteEntity)
{
    SM_ASSERT(deleteEntity, "entity is null");
    DeleteEntity(deleteEntity);
}

void OnBreakGlass(Entity * glass)
{
    SM_ASSERT(glass, "entity is null");
    SetGlassBeBroken(glass);
}
