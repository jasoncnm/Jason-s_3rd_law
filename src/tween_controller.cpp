/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween_controller.h"


int TweenController::FindMovingChannel()
{
    int result = -1;
    for (int i = 0; i < MAX_CHANNEL; i++)
    {
        if (!channels[i].IsEmpty() && channels[i].last().params.paramType == PARAM_TYPE_VECTOR2)
        {
            result = i;
        }
    }
    
    return result;
}

void TweenController::Reset()
{
    start = playing = false;
    for (int i = 0; i < MAX_CHANNEL; i++)
    {
        auto & queue = channels[i];
        
        for (uint32 idx = 0; idx < queue.count; idx++)
        {
            queue[idx].Reset();
        }
        channels[i].Clear();
    }
    
    ResetEvents(endEvents);
    ResetEvents(startEvents);
    
}

// NOTE: Every frame
void TweenController::Update()
{
    if (start)
    {
        HandleEvents(startEvents);
        start = false;
            playing = true;
            }
        
        if (playing)
        {
            for (int channel = 0; channel < MAX_CHANNEL; channel++)
            {
            TweeningQueue & queue = channels[channel];
            
            bool end = true;
            for (uint32 queueIdx = 0; queueIdx < queue.count; queueIdx++)
            {
                if (!queue[queueIdx].End())
                {
                    end = false;
                    queue[queueIdx].UpdateEntityVal();
                     break;
                }
                }
            
            if (end)
            {
                queue.Clear();
            }
            
            }
        
        if (NoTweens())
        {
            HandleEvents(endEvents);
            Reset();
        }
        
    }
    
    }


void AddTween(TweenController & controller, Tween tween, int channel)
{
    controller.channels[channel].Add(tween);
}

 uint32 AddTweenUnique(TweenController & controller, Tween tween)
{
    bool added = false;
    for (int channel = 0; channel < MAX_CHANNEL; channel++)
    {
         auto & queue = controller.channels[channel];
        if (queue.IsEmpty())
        {
            added = true;
            AddTween(controller, tween, channel);
            return channel;
            }
    }
    
    SM_ERROR("Channel FULL!!");
    return 0;
    }
