#if !defined(ACTION_INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

inline void InitKeyMapping()
{
    gameState->keyMappings[MOUSE_LEFT].keys.Add(MOUSE_BUTTON_LEFT);
    gameState->keyMappings[MOUSE_RIGHT].keys.Add(MOUSE_BUTTON_RIGHT);
    gameState->keyMappings[LEFT_KEY].keys.Add(KEY_A);
    gameState->keyMappings[LEFT_KEY].keys.Add(KEY_LEFT);
    gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_D);
    gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_RIGHT);
    gameState->keyMappings[UP_KEY].keys.Add(KEY_W);
    gameState->keyMappings[UP_KEY].keys.Add(KEY_UP);
    gameState->keyMappings[DOWN_KEY].keys.Add(KEY_S);
    gameState->keyMappings[DOWN_KEY].keys.Add(KEY_DOWN);
    gameState->keyMappings[SPACE_KEY].keys.Add(KEY_SPACE);
    gameState->keyMappings[POSSES_KEY].keys.Add(KEY_F);
    gameState->keyMappings[UNDO_KEY].keys.Add(KEY_Z);
    gameState->keyMappings[RESET_KEY].keys.Add(KEY_R);
}

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
