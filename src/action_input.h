#if !defined(ACTION_INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_GAMEPAD 5

enum GameInputType 
{
    NO_INPUT,
    LEFT_KEY,
    RIGHT_KEY,
    UP_KEY,
    DOWN_KEY,
    
    POSSES_KEY,
    SPLIT_KEY,
    
    UNDO_KEY,
    RESET_KEY,
    
    RECOVER_KEY,
    GAME_INPUT_COUNT,
};

struct KeyMapping
{
    Array<int32, 3> keys;
    int32 gamepadButton;
    int32 gamepadAxis;
};

struct Input
{
    bool initialized = false;
    KeyMapping keyMappings[GAME_INPUT_COUNT];
    
};


// Set axis deadzones
constexpr float leftStickDeadzoneX = 0.1f;
constexpr float leftStickDeadzoneY = 0.1f;
constexpr float rightStickDeadzoneX = 0.1f;
constexpr float rightStickDeadzoneY = 0.1f;
constexpr float leftTriggerDeadzone = -0.9f;
constexpr float rightTriggerDeadzone = -0.9f;

inline void CleanUpKeyMapping(KeyMapping * keyMappings)
{
    for (int32 i = 0; i < GAME_INPUT_COUNT; i++)
    {
        keyMappings[i].keys.Clear();
    }
}

inline void InitKeyMapping(KeyMapping * keyMappings)
{
    
    keyMappings[LEFT_KEY].keys.Add(KEY_A);
    keyMappings[LEFT_KEY].keys.Add(KEY_LEFT);
    keyMappings[LEFT_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
        
    keyMappings[RIGHT_KEY].keys.Add(KEY_D);
    keyMappings[RIGHT_KEY].keys.Add(KEY_RIGHT);
    keyMappings[RIGHT_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;

    keyMappings[UP_KEY].keys.Add(KEY_W);
    keyMappings[UP_KEY].keys.Add(KEY_UP);
    keyMappings[UP_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_UP;
    
    keyMappings[DOWN_KEY].keys.Add(KEY_S);
    keyMappings[DOWN_KEY].keys.Add(KEY_DOWN);
    keyMappings[DOWN_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_FACE_DOWN;

    keyMappings[SPLIT_KEY].keys.Add(KEY_SPACE);
    keyMappings[SPLIT_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    
    keyMappings[POSSES_KEY].keys.Add(KEY_F);
    keyMappings[POSSES_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    

    keyMappings[UNDO_KEY].keys.Add(KEY_Z);
    keyMappings[UNDO_KEY].gamepadButton = GAMEPAD_BUTTON_LEFT_TRIGGER_1;
        
    keyMappings[RESET_KEY].keys.Add(KEY_R);
    keyMappings[RESET_KEY].gamepadButton = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    
    keyMappings[RECOVER_KEY].keys.Add(KEY_TAB);
    
    
}

inline bool8 ProccessJoysticks(GameInputType type, int32 gamepad)
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

inline bool8 JustPressed(KeyMapping * keyMappings, GameInputType type)
{

    KeyMapping & mapping = keyMappings[type];
    for (uint32 idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyPressed(mapping.keys[idx])) 
        {
            return true;
        }
    }

    for (int32 gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (!IsGamepadAvailable(gamepad)) continue;
        if (IsGamepadButtonPressed(gamepad, mapping.gamepadButton) || ProccessJoysticks(type, gamepad)) 
        {
            return true;
        }
    }
    
    return false;
}

inline bool8 IsAnyGamepadButtonDown(int32 button)
{
    for (int32 gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (IsGamepadButtonDown(gamepad, button)) return true;        
    }

    return false;
}

inline bool8 IsDown(KeyMapping * keyMappings, GameInputType type)
{
    
    KeyMapping mapping = keyMappings[type];
    for (uint32 idx = 0; idx < mapping.keys.count; idx++)
    {
        if (IsKeyDown(mapping.keys[idx])) 
        {
            return true;
        }
    }
    
    for (int32 gamepad = 0; gamepad < MAX_GAMEPAD; gamepad++)
    {
        if (!IsGamepadAvailable(gamepad)) continue;
        if (IsGamepadButtonDown(gamepad, mapping.gamepadButton) || ProccessJoysticks(type, gamepad)) 
        {
            return true;
        }
    }

    return false;
}

inline bool8 IsActionKeyDown(KeyMapping * keyMappings)
{
    bool8 down = false;
    for (uint32 i = 1; i < GAME_INPUT_COUNT; i++)
    {
        if (IsDown(keyMappings, (GameInputType)i))
        {
            down = true;
            break;
        }
    }
    return down;
}

inline GameInputType GetPressedMoveKey(KeyMapping * keyMappings)
{
    GameInputType result = NO_INPUT;
    if (JustPressed(keyMappings, LEFT_KEY)) result = LEFT_KEY;
    if (JustPressed(keyMappings, RIGHT_KEY)) result = RIGHT_KEY;
    if (JustPressed(keyMappings, UP_KEY)) result = UP_KEY;
    if (JustPressed(keyMappings, DOWN_KEY)) result = DOWN_KEY;
        
    return result;
}

inline GameInputType GetDownedMoveKey(KeyMapping * keyMappings)
{
    GameInputType result = NO_INPUT;
    if (IsDown(keyMappings, LEFT_KEY)) result = LEFT_KEY;
    if (IsDown(keyMappings, RIGHT_KEY)) result = RIGHT_KEY;
    if (IsDown(keyMappings, UP_KEY)) result = UP_KEY;
    if (IsDown(keyMappings, DOWN_KEY)) result = DOWN_KEY;
    
    return result;
}

#define ACTION_INPUT_H
#endif
