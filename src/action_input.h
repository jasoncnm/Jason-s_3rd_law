#if !defined(ACTION_INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */
// Set axis deadzones
constexpr float leftStickDeadzoneX = 0.1f;
constexpr float leftStickDeadzoneY = 0.1f;
constexpr float rightStickDeadzoneX = 0.1f;
constexpr float rightStickDeadzoneY = 0.1f;
constexpr float leftTriggerDeadzone = -0.9f;
constexpr float rightTriggerDeadzone = -0.9f;


inline void InitKeyMapping()
{
    gameState->keyMappings[MOUSE_LEFT].keys.Add(MOUSE_BUTTON_LEFT);
    gameState->keyMappings[MOUSE_RIGHT].keys.Add(MOUSE_BUTTON_RIGHT);
    
    gameState->keyMappings[LEFT_KEY].keys.Add(KEY_A);
    gameState->keyMappings[LEFT_KEY].keys.Add(KEY_LEFT);
    gameState->keyMappings[LEFT_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
        
    gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_D);
    gameState->keyMappings[RIGHT_KEY].keys.Add(KEY_RIGHT);
    gameState->keyMappings[RIGHT_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;

    gameState->keyMappings[UP_KEY].keys.Add(KEY_W);
    gameState->keyMappings[UP_KEY].keys.Add(KEY_UP);
    gameState->keyMappings[UP_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_UP;
    
    gameState->keyMappings[DOWN_KEY].keys.Add(KEY_S);
    gameState->keyMappings[DOWN_KEY].keys.Add(KEY_DOWN);
    gameState->keyMappings[DOWN_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_DOWN;

    gameState->keyMappings[SPLIT_KEY].keys.Add(KEY_SPACE);
    gameState->keyMappings[SPLIT_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    
    gameState->keyMappings[POSSES_KEY].keys.Add(KEY_F);
    gameState->keyMappings[POSSES_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    

    gameState->keyMappings[UNDO_KEY].keys.Add(KEY_Z);
    gameState->keyMappings[UNDO_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_TRIGGER_1;
        
    gameState->keyMappings[RESET_KEY].keys.Add(KEY_R);
    gameState->keyMappings[RESET_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    
}

inline bool ProccessJoysticks(GameInputType type, int gamepad)
{
    float leftStickX =  GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
    float leftStickY =  GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
    float rightStickX = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X);
    float rightStickY = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y);

    // Calculate deadzones
    if (leftStickX > -leftStickDeadzoneX && leftStickX < leftStickDeadzoneX) leftStickX = 0.0f;
    if (leftStickY > -leftStickDeadzoneY && leftStickY < leftStickDeadzoneY) leftStickY = 0.0f;
    if (rightStickX > -rightStickDeadzoneX && rightStickX < rightStickDeadzoneX) rightStickX = 0.0f;
    if (rightStickY > -rightStickDeadzoneY && rightStickY < rightStickDeadzoneY) rightStickY = 0.0f;

    if (type >= LEFT_KEY && type <= DOWN_KEY)
    {
        if (Abs(leftStickX) > Abs(leftStickY))
        {
            return (leftStickX > 0 && type == RIGHT_KEY) || (leftStickX < 0 && type == LEFT_KEY);
        }
        else
        {
            return (leftStickY < 0 && type == UP_KEY) || (leftStickY > 0 && type == DOWN_KEY);
        }
    }

    return false;
}

inline bool JustPressed(GameInputType type)
{

    if (type == ANY_KEY)
    {
        return !IsKeyDown(KEY_LEFT_ALT)  &&
               !IsKeyDown(KEY_RIGHT_ALT) &&
               !IsKeyDown(KEY_LEFT_SUPER) &&
               !IsKeyDown(KEY_RIGHT_SUPER) &&
               (GetKeyPressed() > 0 || GetGamepadButtonPressed() > 0);
    }
    
    KeyMapping & mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx]) || IsMouseButtonPressed(mapping.keys[idx])) return true;
    }

    for (int gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (!IsGamepadAvailable(gamepad)) continue;
        if (IsGamepadButtonPressed(gamepad, mapping.gamepadButton) || ProccessJoysticks(type, gamepad)) return true;
    }
    
    return false;
}

inline bool JustPressedMoveKey()
{
    bool result = JustPressed(UP_KEY) || JustPressed(DOWN_KEY) || JustPressed(LEFT_KEY) || JustPressed(RIGHT_KEY);
    return result;
}

inline bool IsAnyGamepadButtonDown(int button)
{
    for (int gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (IsGamepadButtonDown(gamepad, button)) return true;        
    }

    return false;
}

inline bool IsDown(GameInputType type)
{
    
    if (type == ANY_KEY)
    {
        int keycode = GetKeyPressed();
        int gamepadButton = GetGamepadButtonPressed();
        return !IsKeyDown(KEY_LEFT_ALT)  &&
               !IsKeyDown(KEY_RIGHT_ALT) &&
               !IsKeyDown(KEY_LEFT_SUPER) &&
               !IsKeyDown(KEY_RIGHT_SUPER) &&
               (IsKeyDown(keycode) || IsAnyGamepadButtonDown(gamepadButton));
    }
    
    KeyMapping mapping = gameState->keyMappings[type];
    for (int idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx]) || IsMouseButtonDown(mapping.keys[idx])) return true;
    }
    
    for (int gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (!IsGamepadAvailable(gamepad)) continue;
        if (IsGamepadButtonDown(gamepad, mapping.gamepadButton) || ProccessJoysticks(type, gamepad)) return true;
    }

    return false;
}


#define ACTION_INPUT_H
#endif
