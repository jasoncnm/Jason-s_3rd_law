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
        if (!channels[i].empty() && channels[i].back().params.paramType == type)
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
        result = &channels[ch].front();
    }
    return result;
}

int32 TweenController::FindChannelByTweenProperty(ParamType type, void * property)
{
     int32 result = -1;
    for (int32 i = 0; i < MAX_CHANNEL; i++)
    {
            if (!channels[i].empty())
        {
            Tween & tween = channels[i].back();
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
        result = &channels[ch].front();
    }
    
    return result;
}

void TweenController::Reset()
{
    start = playing = false;
    for (int32 i = 0; i < MAX_CHANNEL; i++)
    {
        auto & queue = channels[i];
        
        for (uint32 idx = 0; idx < queue.size(); idx++)
        {
            queue[idx].Reset();
        }
        channels[i].clear();
    }
    
    ResetEvents(endEvents);
    ResetEvents(startEvents);
    
}

void TweenController::EnableAddedTweens()
{
    for (uint32 ch = 0; ch < MAX_CHANNEL; ch++)
    {
        auto & queue = channels[ch];
        for (uint32 queueIdx = 0; queueIdx < queue.size(); queueIdx++)
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
            
            if (!queue.empty())
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
                        queue.erase(queue.begin());
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


void AddTween(TweenController & controller, Tween tween, int32 channel)
{
     auto & ch = controller.channels[channel];
    ch.push_back(tween);
}

 uint32 AddTweenUnique(TweenController & controller, Tween tween)
{
    bool added = false;
    for (int32 channel = 0; channel < MAX_CHANNEL; channel++)
    {
         auto & queue = controller.channels[channel];
        if (queue.empty())
        {
            added = true;
            AddTween(controller, tween, channel);
            return channel;
            }
    }
    
    SM_ERROR("Channel FULL!!");
    return 0;
    }
