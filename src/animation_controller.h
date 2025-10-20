#if !defined(ANIMATION_CONTROLLER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "move_animation.h"

struct AnimationController;

struct EndAnimationEvent
{
    AnimationController * controller = nullptr;
    void (*EndAnimationFunc)(AnimationController * controller) = nullptr;

    void Reset()
    {
        controller = nullptr;
        EndAnimationFunc = nullptr;
    }
    
};

struct AnimationController
{
    
    bool playing = false;
    int currentQueueIndex = 0;
    Array<MoveAnimation, 10> moveAnimationQueue;
    Vector2 currentPosition;
    
    EndAnimationEvent endEvent;

    inline MoveAnimation & GetMoveAni(int index)
    {
        return moveAnimationQueue[index];        
    }

    void Reset();
    
    // NOTE: UpdateAnimation Every frame
    void Update();

    // NOTE: Handle event end of animation
    void HandleAnimationNotPlaying();
};
    
void AddAnimation(AnimationController & controller, MoveAnimation animation);

// NOTE: Get a play event to play    
void OnPlayEvent(AnimationController * controller);


#define ANIMATION_CONTROLLER_H
#endif
