/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween_controller.h"

int32 TweenController::FindChannelByParamType(ParamType type)
{
    int32 result = -1;
    for (int32 i = 0; i < MAX_CHANNEL; i++)
    {
        if (!channels[i].IsEmpty() && channels[i].last().params.paramType == type)
        {
            result = i;
        }
    }
    
    return result;
}
Tween * TweenController::FindTweenByParamType(ParamType type)
{
    Tween * result = nullptr;
    int32 ch = FindChannelByParamType(type);
    if (ch >= 0)
    {
        result = &channels[ch].first();
    }
    return result;
}

int32 TweenController::FindChannelByTweenProperty(ParamType type, void * property)
{
     int32 result = -1;
    for (int32 i = 0; i < MAX_CHANNEL; i++)
    {
            if (!channels[i].IsEmpty())
        {
            Tween & tween = channels[i].last();
            if (tween.params.paramType == type)
            {
                if (property == tween.GetTweeningValue())
                {
                    result = i;
                    break;
                }
            }
    }
    }
    return result;
}

Tween * TweenController::FindTweenByTweenProperty(ParamType type, void * property)
{
    Tween * result = nullptr;
    int32 ch = FindChannelByTweenProperty(type, property);
    if (ch >= 0)
    {
        result = &channels[ch].first();
    }
    
    return result;
}

void TweenController::Reset()
{
    start = playing = false;
    for (int32 i = 0; i < MAX_CHANNEL; i++)
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

void TweenController::EnableAddedTweens()
{
    for (uint32 ch = 0; ch < MAX_CHANNEL; ch++)
    {
        auto & queue = channels[ch];
        for (uint32 queueIdx = 0; queueIdx < queue.count; queueIdx++)
        {
            queue[queueIdx].play = true;
        }
        }
}

// NOTE: Every frame
void TweenController::Update()
{
    // TODO: BUG, tweens that are waiting for event will play immediatley if the tweencontroller is playing
    if (start)
    {
        HandleEvents(startEvents);
        start = false;
            playing = true;
            }
    
        if (playing)
    {
        playing = false;
            for (int32 channel = 0; channel < MAX_CHANNEL; channel++)
            {
            auto & queue = channels[channel];
            
            if (!queue.IsEmpty())
            {
                Tween & tween = queue[0];
                if (tween.play)
                {
                    if (!tween.End())
                    {
                        tween.UpdateEntityVal();
                        playing = true;
                    }
                    else
                    {
                        queue.RemoveFront();
                        channel--;
                        continue;
                    }
                }
            }
        }
        
        if (!playing && NoTweens())
        {
            HandleEvents(endEvents);
        }
        
    }
    }

void TweenController::AdjustSpeed(real32 rate)
{
    for (uint32 channel = 0; channel < MAX_CHANNEL; channel++)
    {
        auto & queue = channels[channel];
        for (uint32 i = 0; i < queue.count; i++)
        {
            queue[i].dt *= rate;
        }
    }
}

void AddTween(TweenController & controller, Tween tween, int32 channel)
{
     auto & ch = controller.channels[channel];
    ch.Add(tween);
}

 uint32 AddTweenUnique(TweenController & controller, Tween tween)
{
    bool added = false;
    for (int32 channel = 0; channel < MAX_CHANNEL; channel++)
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
