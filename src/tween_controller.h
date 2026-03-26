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
    using TweeningQueue = Array<Tween, 15>;

    bool8 start   = false;
    bool8 playing = false;
    
    TweeningQueue channels[MAX_CHANNEL];

    Array<TweenEvent, MAX_EVENT> startEvents;
    Array<TweenEvent, MAX_EVENT> endEvents;

    void Reset();
    
    // NOTE: Update Every frame
    void Update();
    
    int32 FindChannelByParamType(ParamType type);
    
    int32 FindChannelByTweenProperty(void * property);
        
    bool8 NoTweens()
    {
        bool8 result = true;

        for (int32 channel = 0; channel < MAX_CHANNEL; channel++)
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

void AddTween(TweenController & controller, Tween tween, int32 channel = 0);
uint32 AddTweenUnique(TweenController & controller, Tween tween);


#define TWEEN_CONTROLLER_H
#endif
