/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "tween_controller.h"


void HandleTweenEvent(TweenEvent & event)
{
    if (event.controller)
    {
        OnPlayEvent(event.controller);
    }

    if (event.deleteEntity)
    {
        OnDeleteEvent(event.deleteEntity);
    }

    event.Reset();
}

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
        channels[i].Clear();
    }
    endEvent.Reset();
}

// NOTE: Every frame
void TweenController::Update()
{
    if (start)
    {
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
            
            #if 0
            if (!queue.IsEmpty())
            {
                if (queue[0].UpdateEntityVal())
                    {
                        queue.RemoveIdxAndSwap(0);
                    }
            }
            #endif
            }
        
        if (NoTweens())
        {
            HandleTweenEvent(endEvent);
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

void OnPlayEvent(TweenController * controller)
{
    SM_ASSERT(controller, "controller is null");
    controller->start   = true;
    HandleTweenEvent(controller->startEvent);
}


void OnDeleteEvent(Entity * deleteEntity)
{
    SM_ASSERT(deleteEntity, "entity is null");
    DeleteEntity(deleteEntity);
}
