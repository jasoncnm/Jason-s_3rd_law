#if !defined(TWEEN_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

enum ParamType
{
    PARAM_TYPE_NONE,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_VECTOR2,
};

struct TweenParams
{
    ParamType paramType;

    union
    {
        struct
        {
            float startF;
            float endF;
            float * realF;
        };

        struct
        {
            Vector2 startVec2;
            Vector2 endVec2;
            Vector2 * realVec2;
        };
    };
};

struct Tween
{

    TweenParams params;
    
    float t = 0;
    float target_t = 1;
    float dt;

    bool Update()
    {
        bool end = false;
        float delta = GetFrameTime() * dt;

        if (t < target_t)
        {
            t += delta;
        }

        if (t > target_t)
        {
            t = target_t;
            end = true;
        }

        return end;
    }

    bool UpdateEntityVal();

    float (*Easing)(float);
    
};

Tween CreateTween(TweenParams params, float (*Easing)(float) = nullptr, float animateSpeed = 5.0f, float target_t = 1.0f);

    
#define TWEEN_H
#endif
