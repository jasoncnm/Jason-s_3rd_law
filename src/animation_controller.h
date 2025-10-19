#if !defined(ANIMATION_CONTROLLER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "move_animation.h"

struct AnimationController
{
    
    bool playing = false;
    Array<MoveAnimation, 10> moveAnimationQueue;

    template<typename T>
    void AddAnimation(T animation);

    // NOTE: UpdateAnimation Every frame
    void Update();

    // NOTE: Get a play event to play    
    void OnPlayEvent();

    // NOTE: publish event end of animation
    void PublishEndOfAnimation();
}


#define ANIMATION_CONTROLLER_H
#endif
