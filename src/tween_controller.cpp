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
    endEvent.Reset();
    currentQueueIndex = 0;
    tweeningQueue.Clear();
}

// NOTE: Every frame
void TweenController::Update()
{
    if (currentQueueIndex < tweeningQueue.count)
    {
        if (playing)
        {
            Tween & tween = tweeningQueue[currentQueueIndex];
            bool end = tween.UpdateEntityVal();
            if (end)
            {
                currentQueueIndex++;
            }
        }
    }
    else
    {
        playing = false;       
        HandleEndOfTween();      
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

void AddTween(TweenController & controller, Tween tween)
{
    controller.tweeningQueue.Add(tween);
    controller.tweeningQueue[controller.currentQueueIndex].UpdateEntityVal();
}

void OnPlayEvent(TweenController * controller)
{
    SM_ASSERT(controller, "controller is null");
    
    controller->playing = true;
    controller->currentQueueIndex = 0;
}


void OnDeleteEvent(Entity * deleteEntity)
{
    SM_ASSERT(deleteEntity, "entity is null");
    DeleteEntity(deleteEntity);
}
