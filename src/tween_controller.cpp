/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween_controller.h"


void HandleTweenEvent(TweenEvent & event)
{
    if (event.controller && event.OnPlayFunc)
    {
        event.OnPlayFunc(event.controller);
    }

    if (event.deleteEntity && event.OnDeleteFunc)
    {
        event.OnDeleteFunc(event.deleteEntity);
    }

    event.Reset();
}

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
                HandleTweenEvent(endEvent);
                Reset();

            }
        }
    }
}


void AddTween(TweenController & controller, Tween tween, int channel)
{
    controller.channels[channel].Add(tween);
}

void OnPlayEvent(TweenController * controller)
{
    SM_ASSERT(controller, "controller is null");
    controller->playing = true;
    HandleTweenEvent(controller->startEvent);
}


void OnDeleteEvent(Entity * deleteEntity)
{
    SM_ASSERT(deleteEntity, "entity is null");
    DeleteEntity(deleteEntity);
}
