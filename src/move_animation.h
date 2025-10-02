#if !defined(MOVEANIMATION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

struct MoveAnimation
{
    bool playing = false;
    Vector2 moveStart;
    Vector2 moveEnd;

    float move_t = 0;
    float move_target_t = 1;
    float move_dt;

    void Update();
    Vector2 GetPosition(float (*Easing)(float));
    
};

Vector2 MoveAnimation::GetPosition(float (*Easing)(float))
{
    // TODO: Implement generic easing function
    
    float t = move_t;

    if (Easing)
    {
        t = easeOutCubic(t);
    }    
    return Vector2Scale(moveStart, 1.0f - t) + Vector2Scale(moveEnd, t);
}

void MoveAnimation::Update()
{
    float delta = GetFrameTime() * move_dt;

    if (move_t < move_target_t)
    {
        move_t += delta;
        if (move_t > move_target_t)
        {
            move_t = move_target_t;
            playing = false;
        }
    }
}



#define MOVEANIMATION_H
#endif
