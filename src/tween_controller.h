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

struct TweenEvent
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

    bool start   = false;
    bool playing = false;
    
    TweeningQueue channels[MAX_CHANNEL];

    TweenEvent startEvent;
    TweenEvent endEvent;

    void Reset();
    
    // NOTE: Update Every frame
    void Update();

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

// NOTE: Handle event
void HandleEvent(TweenEvent & event);
    
void AddTween(TweenController & controller, Tween tween, int channel = 0);

// NOTE: Get a play event to play    
void OnPlayEvent(TweenController * controller);

void OnDeleteEvent(Entity * deleteEntity);


#define TWEEN_CONTROLLER_H
#endif
