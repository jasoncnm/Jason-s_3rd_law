#if !defined(TWEEN_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_EVENT 5

struct TweenController;
struct TweenEvent
{
    TweenController * controller = nullptr;
    Entity * deleteEntity = nullptr;
    Entity * breakEntity = nullptr;
    
    void Reset()
    {
        breakEntity = nullptr;
        controller = nullptr;
        deleteEntity = nullptr;
    }
    
};

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
    
    Array<TweenEvent, MAX_EVENT> startEvents;
    Array<TweenEvent, MAX_EVENT> endEvents;

    bool8 End()
    {
        return t >= target_t;
    }

     void UpdateEntityVal();

    float (*Easing)(float);
    
    void Reset();
    
};

Tween CreateTween(TweenParams params, float (*Easing)(float) = nullptr, float animateSpeed = 5.0f, float target_t = 1.0f);


// NOTE: Handle event
void HandleEvents(Array<TweenEvent, MAX_EVENT> & events);
// NOTE: Get a play event to play    
void OnPlayEvent(TweenController * controller);

void OnDeleteEvent(Entity * deleteEntity);

void OnBreakGlass(Entity * breakEntity);

    
#define TWEEN_H
#endif
