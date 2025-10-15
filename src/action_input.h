#if !defined(ACTION_INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

inline bool JustPressed(GameInputType type)
{
    KeyMapping & mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx]) || IsMouseButtonPressed(mapping.keys[idx])) return true;
    }
    
    return false;
}

inline bool JustPressedMoveKey()
{
    bool result = JustPressed(UP_KEY) || JustPressed(DOWN_KEY) || JustPressed(LEFT_KEY) || JustPressed(RIGHT_KEY);
    return result;
}

inline bool IsDown(GameInputType type)
{
    KeyMapping mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx]) || IsMouseButtonDown(mapping.keys[idx])) return true;
    }
    return false;
}


#define ACTION_INPUT_H
#endif
