/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween_controller.h"



void TweenController::Reset()
{
    for (int i = 0; i < MAX_CHANNEL; i++)
    {
        channels[i].Clear();
    }
    endEvent.Reset();
}

// NOTE: Every frame
void TweenController::Update()
{
    if (!NoTweens())
    {
        if (playing)
        {
            for (int channel = 0; channel < MAX_CHANNEL; channel++)
            {
                TweeningQueue & queue = channels[channel];
                if (!queue.IsEmpty())
                {
                    if (queue[0].UpdateEntityVal())
                    {
                        queue.RemoveIdxAndSwap(0);
                    }
                    
                }
            }

            if (NoTweens())
            {
                playing = false;
                HandleEndOfTween();                
            }
        }
    }
}

// NOTE: Handle event end of animation
void TweenController::HandleEndOfTween()
{
    if (endEvent.controller && endEvent.OnPlayFunc)
    {
        endEvent.OnPlayFunc(endEvent.controller);
    }

    if (endEvent.deleteEntity && endEvent.OnDeleteFunc)
    {
        endEvent.OnDeleteFunc(endEvent.deleteEntity);
    }

    endEvent.Reset();
    Reset();
}

void AddTween(TweenController & controller, Tween tween, int channel)
{
    controller.channels[channel].Add(tween);
}

void OnPlayEvent(TweenController * controller)
{
    SM_ASSERT(controller, "controller is null");
    controller->playing = true;
}


void OnDeleteEvent(Entity * deleteEntity)
{
    SM_ASSERT(deleteEntity, "entity is null");
    DeleteEntity(deleteEntity);
}
