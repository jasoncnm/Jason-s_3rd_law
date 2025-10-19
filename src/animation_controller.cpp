/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "animation_controller.h"
c
template<typename T>
void AniAddAnimation::AddAnimation(T animation)
{
    
}

// NOTE: UpdateAnimation Every frame
void AnimationController::Update()
{
    if (playing)
    {
        playing = false;
        for (int i = 0; i < moveAnimationQueue.count; i++)
        {
            MoveAnimation & ani = moveAnimationQueue[i];
            if (ani.playing)
            {
                playing = true;
                ani.Update();
                return;
            }
        }
        
        if (!playing)
        {
            PublishEndOfAnimation();            
        }
    }
}

// NOTE: Get a play event to play    
void AnimationController::OnPlayEvent()
{
    playing = true;
}

// NOTE: publish event end of animation
void AnimationController::PublishEndOfAnimation()
{
    
}
