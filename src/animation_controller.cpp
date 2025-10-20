/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "animation_controller.h"

void AnimationController::Reset()
{
    endEvent.Reset();
    currentQueueIndex = 0;
    moveAnimationQueue.Clear();
}

// NOTE: UpdateAnimation Every frame
void AnimationController::Update()
{
    if (currentQueueIndex < moveAnimationQueue.count)
    {
        if (playing)
        {
            MoveAnimation & ani = moveAnimationQueue[currentQueueIndex];
            bool end = ani.Update();
            currentPosition = ani.GetPosition();
            if (end)
            {
                currentQueueIndex++;
            }
        }
    }
    else
    {
        playing = false;       
        HandleAnimationNotPlaying();      
    }
}

// NOTE: Handle event end of animation
void AnimationController::HandleAnimationNotPlaying()
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

void AddAnimation(AnimationController & controller, MoveAnimation animation)
{
    controller.moveAnimationQueue.Add(animation);
    controller.currentPosition = controller.moveAnimationQueue[controller.currentQueueIndex].GetPosition();
}

void OnPlayEvent(AnimationController * controller)
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
