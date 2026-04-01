#if !defined(TWEEN_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_EVENT 5

struct Entity;
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
    PARAM_TYPE_INT,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_VECTOR2,
    PARAM_TYPE_COLOR,
};

struct TweenParams
{
    ParamType paramType;
    union
    {
        struct
        {
            int32 startI;
            int32 endI;
             int32 * realI;
        };
        struct
        {
            real32 startF;
            real32 endF;
            real32 * realF;
        };
struct
        {
            Vector2 startVec2;
            Vector2 endVec2;
            Vector2 * realVec2;
        };
        struct 
        {
            Color startColor;
            Color endColor;
             Color * realColor;
        };
    };
    
};

struct Tween
{

    TweenParams params;
    bool8 play;
     real32 t = 0;
    real32 target_t = 1;
    real32 dt;
    
    Array<TweenEvent, MAX_EVENT> startEvents;
    Array<TweenEvent, MAX_EVENT> endEvents;

    bool8 End()
    {
        return t >= target_t;
    }
    
    void UpdateEntityVal();
    
    void * GetTweeningValue();

    real32 (*Easing)(real32);
    
    void Reset();
    
};

Tween CreateTween(TweenParams params, real32 (*Easing)(real32) = nullptr, real32 animateSpeed = 5.0f, real32 target_t = 1.0f);


// NOTE: Handle event
void HandleEvents(Array<TweenEvent, MAX_EVENT> & events);
// NOTE: Get a play event to play    
void OnPlayEvent(TweenController * controller);

void OnDeleteEvent(Entity * deleteEntity);

void OnBreakGlass(Entity * breakEntity);

    
#define TWEEN_H
#endif
