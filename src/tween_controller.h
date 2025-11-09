#if !defined(TWEEN_CONTROLLER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween.h"

struct TweenController;
struct EndTweeningEvent
{
    TweenController * controller = nullptr;
    void (*OnPlayFunc)(TweenController * controller) = nullptr;

    Entity * deleteEntity = nullptr;
    void (*OnDeleteFunc)(Entity * deleteEntity) = nullptr;

    void Reset()
    {
        controller = nullptr;
        OnPlayFunc = nullptr;

        deleteEntity = nullptr;
        OnDeleteFunc = nullptr;
    }
    
};

struct TweenController
{
    
    bool playing = false;
    int currentQueueIndex = 0;
    Array<Tween, 10> tweeningQueue;
    
    EndTweeningEvent endEvent;

    void Reset();
    
    // NOTE: Update Every frame
    void Update();

    // NOTE: Handle event end
    void HandleEndOfTween();

};
    
void AddTween(TweenController & controller, Tween tween);

// NOTE: Get a play event to play    
void OnPlayEvent(TweenController * controller);

void OnDeleteEvent(Entity * deleteEntity);


#define TWEEN_CONTROLLER_H
#endif
