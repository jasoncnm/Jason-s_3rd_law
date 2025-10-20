/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "animation_controller.h"

void AddAnimation(AnimationController & controller, MoveAnimation animation)
{
    controller.moveAnimationQueue.Add(animation);
    controller.currentPosition = controller.moveAnimationQueue[controller.currentQueueIndex].GetPosition();
}

void AnimationController::Reset()
{
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
        HandleEndOfAnimation();      
    }
}

void OnPlayEvent(AnimationController * controller)
{
    SM_ASSERT(controller, "controller is null");
    
    controller->playing = true;
    controller->currentQueueIndex = 0;
}

// NOTE: Handle event end of animation
void AnimationController::HandleEndOfAnimation()
{
    if (endEvent.controller && endEvent.EndAnimationFunc)
    {
        endEvent.EndAnimationFunc(endEvent.controller);
        endEvent.Reset();
    }

    Reset();
}
