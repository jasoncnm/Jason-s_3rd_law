#if !defined(EVENT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_EVENT 50

struct Event
{
    EventType type;

    // Callbacks
    
    bool handled = false;
};

struct EventManager
{
    Array<Event, MAX_EVENT> eventQueue;

    void Dispatch()
    {
        for (int i = 0; i < eventQueue.count; i++)
        {
            
        }
    }
};


#define EVENT_H
#endif
