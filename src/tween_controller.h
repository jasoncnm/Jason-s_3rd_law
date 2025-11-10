#if !defined(TWEEN_CONTROLLER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween.h"

#define MAX_CHANNEL 5

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
    using TweeningQueue = Array<Tween, 10>;
    
    bool playing = false;
    int currentQueueIndex = 0;
    
    TweeningQueue channels[MAX_CHANNEL];
    
    EndTweeningEvent endEvent;

    void Reset();
    
    // NOTE: Update Every frame
    void Update();

    // NOTE: Handle event end
    void HandleEndOfTween();

    bool NoTweens()
    {
        bool result = true;

        for (int channel = 0; channel < MAX_CHANNEL; channel++)
        {
            TweeningQueue & queue = channels[channel];
            if (!queue.IsEmpty())
            {
                result = false;
            }
        }

        return result;
    }
    
};
    
void AddTween(TweenController & controller, Tween tween, int channel = 0);

// NOTE: Get a play event to play    
void OnPlayEvent(TweenController * controller);

void OnDeleteEvent(Entity * deleteEntity);


#define TWEEN_CONTROLLER_H
#endif
