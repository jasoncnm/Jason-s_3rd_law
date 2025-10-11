/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
#include "move_animation.h"

void AddMoveAnimateQueue(MoveAnimationQueue & queue, MoveAnimation ani)
{
    MoveAnimationQueue tempQueue = queue;
    queue.Clear();
    for (int i = 0; i < tempQueue.count; i++)
    {
        MoveAnimation & a = tempQueue[i];
        if (a.playing)
        {
            queue.Add(a);
        }
    }
    queue.Add(ani);
}

bool IsAnimationPlaying(Entity * entity)
{
    bool result = false;
    for (int animationIndex = 0; animationIndex < entity->moveAniQueue.count; animationIndex++)
    {
        MoveAnimation & ani = entity->moveAniQueue[animationIndex];
        result |= ani.playing;
    }

    return result;
}

void AdjustAnimatingSpeed(Entity * entity, float ratio)
{
    for (int animationIndex = 0; animationIndex < entity->moveAniQueue.count; animationIndex++)
    {
        MoveAnimation & ani = entity->moveAniQueue[animationIndex];
        ani.move_dt *= ratio;
    }
    
}

MoveAnimation GetMoveAnimation(float (*Easing)(float), Vec2 moveStart, Vec2 moveEnd,
                               float animateSpeed = 5.0f, float target_t = 1.0f, bool startPlay = true, float delay = 0.0f)
{
    MoveAnimation ani;
    ani.playing = startPlay;
    ani.move_t = 0;
    ani.move_target_t = target_t;
    ani.move_dt = animateSpeed;
    ani.moveStart = moveStart;
    ani.moveEnd = moveEnd;
    ani.delay = delay;
    ani.Easing = Easing;
    
    return ani;
}

Vec2 MoveAnimation::GetPosition()
{
    SM_ASSERT(move_target_t, "Divide by zero");
    
    float t = move_t / move_target_t;

    if (Easing)
    {
        t = Easing(t);
    }

    Vec2 result = Vec2Add(moveStart, Vec2Scale(Vec2Subtract(moveEnd, moveStart), t));
    
    return result;
}

void MoveAnimation::Update()
{
    if (delay <= 0)
    {
        float delta = DeltaTime() * move_dt;

        if (move_t < move_target_t)
        {
            move_t += delta;
        }

        if (move_t > move_target_t)
        {
            move_t = move_target_t;
            playing = false;
        }
            
    }
    else
    {
        delay -= DeltaTime();
    }
}

float GetDelay(Entity * pushedEntity, Entity * pushEntity, float speed, IVec2 pushDir)
{
    float delay = 0;

    IVec2 attachDir = pushEntity->attach ? pushEntity->attachDir : (IVec2){ 0, 0 };
    Vec2 pointA = GetTilePivot(pushEntity);
    Vec2 pointB = GetTilePivot(pushedEntity->tilePos - pushDir, pushEntity->tileSize, attachDir);

    if (pushEntity->tilePos.x - pushedEntity->tilePos.x == 0)
    {

        float delayDist = Abs(pointA.y - pointB.y) / MAP_TILE_SIZE;
        
        delay = delayDist / speed;
        
    }
    else if (pushEntity->tilePos.y - pushedEntity->tilePos.y == 0)
    {
        float delayDist = Abs(pointA.x - pointB.x) / MAP_TILE_SIZE;
        delay = delayDist / speed;
    }

    return delay;
    
}
