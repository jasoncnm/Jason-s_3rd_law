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

struct TweenController
{
    using TweeningQueue = Array<Tween, 20>;

    bool8 start   = false;
    bool8 playing = false;
    
    TweeningQueue channels[MAX_CHANNEL];

    TweenEvent startEvent;
    TweenEvent endEvent;

    void Reset();
    
    // NOTE: Update Every frame
    void Update();
    
    int FindMovingChannel();

    bool8 NoTweens()
    {
        bool8 result = true;

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
uint32 AddTweenUnique(TweenController & controller, Tween tween);


#define TWEEN_CONTROLLER_H
#endif
