#if !defined(ENGINE_INPUT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


// Keyboard keys (US keyboard layout)
// NOTE: Use GetKeyPressed() to allow redefining
// required keys for alternative layouts
enum KeyboardKey
{
    E_KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    E_KEY_APOSTROPHE      = 39,       // Key: '
    E_KEY_COMMA           = 44,       // Key: ,
    E_KEY_MINUS           = 45,       // Key: -
    E_KEY_PERIOD          = 46,       // Key: .
    E_KEY_SLASH           = 47,       // Key: /
    E_KEY_ZERO            = 48,       // Key: 0
    E_KEY_ONE             = 49,       // Key: 1
    E_KEY_TWO             = 50,       // Key: 2
    E_KEY_THREE           = 51,       // Key: 3
    E_KEY_FOUR            = 52,       // Key: 4
    E_KEY_FIVE            = 53,       // Key: 5
    E_KEY_SIX             = 54,       // Key: 6
    E_KEY_SEVEN           = 55,       // Key: 7
    E_KEY_EIGHT           = 56,       // Key: 8
    E_KEY_NINE            = 57,       // Key: 9
    E_KEY_SEMICOLON       = 59,       // Key: ;
    E_KEY_EQUAL           = 61,       // Key: =
    E_KEY_A               = 65,       // Key: A | a
    E_KEY_B               = 66,       // Key: B | b
    E_KEY_C               = 67,       // Key: C | c
    E_KEY_D               = 68,       // Key: D | d
    E_KEY_E               = 69,       // Key: E | e
    E_KEY_F               = 70,       // Key: F | f
    E_KEY_G               = 71,       // Key: G | g
    E_KEY_H               = 72,       // Key: H | h
    E_KEY_I               = 73,       // Key: I | i
    E_KEY_J               = 74,       // Key: J | j
    E_KEY_K               = 75,       // Key: K | k
    E_KEY_L               = 76,       // Key: L | l
    E_KEY_M               = 77,       // Key: M | m
    E_KEY_N               = 78,       // Key: N | n
    E_KEY_O               = 79,       // Key: O | o
    E_KEY_P               = 80,       // Key: P | p
    E_KEY_Q               = 81,       // Key: Q | q
    E_KEY_R               = 82,       // Key: R | r
    E_KEY_S               = 83,       // Key: S | s
    E_KEY_T               = 84,       // Key: T | t
    E_KEY_U               = 85,       // Key: U | u
    E_KEY_V               = 86,       // Key: V | v
    E_KEY_W               = 87,       // Key: W | w
    E_KEY_X               = 88,       // Key: X | x
    E_KEY_Y               = 89,       // Key: Y | y
    E_KEY_Z               = 90,       // Key: Z | z
    E_KEY_LEFT_BRACKET    = 91,       // Key: [
    E_KEY_BACKSLASH       = 92,       // Key: '\'
    E_KEY_RIGHT_BRACKET   = 93,       // Key: ]
    E_KEY_GRAVE           = 96,       // Key: `
    // Function keys
    E_KEY_SPACE           = 32,       // Key: Space
    E_KEY_ESCAPE          = 256,      // Key: Esc
    E_KEY_ENTER           = 257,      // Key: Enter
    E_KEY_TAB             = 258,      // Key: Tab
    E_KEY_BACKSPACE       = 259,      // Key: Backspace
    E_KEY_INSERT          = 260,      // Key: Ins
    E_KEY_DELETE          = 261,      // Key: Del
    E_KEY_RIGHT           = 262,      // Key: Cursor right
    E_KEY_LEFT            = 263,      // Key: Cursor left
    E_KEY_DOWN            = 264,      // Key: Cursor down
    E_KEY_UP              = 265,      // Key: Cursor up
    E_KEY_PAGE_UP         = 266,      // Key: Page up
    E_KEY_PAGE_DOWN       = 267,      // Key: Page down
    E_KEY_HOME            = 268,      // Key: Home
    E_KEY_END             = 269,      // Key: End
    E_KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    E_KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    E_KEY_NUM_LOCK        = 282,      // Key: Num lock
    E_KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    E_KEY_PAUSE           = 284,      // Key: Pause
    E_KEY_F1              = 290,      // Key: F1
    E_KEY_F2              = 291,      // Key: F2
    E_KEY_F3              = 292,      // Key: F3
    E_KEY_F4              = 293,      // Key: F4
    E_KEY_F5              = 294,      // Key: F5
    E_KEY_F6              = 295,      // Key: F6
    E_KEY_F7              = 296,      // Key: F7
    E_KEY_F8              = 297,      // Key: F8
    E_KEY_F9              = 298,      // Key: F9
    E_KEY_F10             = 299,      // Key: F10
    E_KEY_F11             = 300,      // Key: F11
    E_KEY_F12             = 301,      // Key: F12
    E_KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    E_KEY_LEFT_CONTROL    = 341,      // Key: Control left
    E_KEY_LEFT_ALT        = 342,      // Key: Alt left
    E_KEY_LEFT_SUPER      = 343,      // Key: Super left
    E_KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    E_KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    E_KEY_RIGHT_ALT       = 346,      // Key: Alt right
    E_KEY_RIGHT_SUPER     = 347,      // Key: Super right
    E_KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    E_KEY_KP_0            = 320,      // Key: Keypad 0
    E_KEY_KP_1            = 321,      // Key: Keypad 1
    E_KEY_KP_2            = 322,      // Key: Keypad 2
    E_KEY_KP_3            = 323,      // Key: Keypad 3
    E_KEY_KP_4            = 324,      // Key: Keypad 4
    E_KEY_KP_5            = 325,      // Key: Keypad 5
    E_KEY_KP_6            = 326,      // Key: Keypad 6
    E_KEY_KP_7            = 327,      // Key: Keypad 7
    E_KEY_KP_8            = 328,      // Key: Keypad 8
    E_KEY_KP_9            = 329,      // Key: Keypad 9
    E_KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    E_KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    E_KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    E_KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    E_KEY_KP_ADD          = 334,      // Key: Keypad +
    E_KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    E_KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    E_KEY_BACK            = 4,        // Key: Android back button
    E_KEY_MENU            = 5,        // Key: Android menu button
    E_KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    E_KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
};

// Add backwards compatibility support for deprecated names
#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON_RIGHT
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON_MIDDLE

// Mouse buttons
typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} MouseButton;

// Mouse cursor
typedef enum {
    MOUSE_CURSOR_DEFAULT       = 0,     // Default pointer shape
    MOUSE_CURSOR_ARROW         = 1,     // Arrow shape
    MOUSE_CURSOR_IBEAM         = 2,     // Text writing cursor shape
    MOUSE_CURSOR_CROSSHAIR     = 3,     // Cross shape
    MOUSE_CURSOR_POINTING_HAND = 4,     // Pointing hand cursor
    MOUSE_CURSOR_RESIZE_EW     = 5,     // Horizontal resize/move arrow shape
    MOUSE_CURSOR_RESIZE_NS     = 6,     // Vertical resize/move arrow shape
    MOUSE_CURSOR_RESIZE_NWSE   = 7,     // Top-left to bottom-right diagonal resize/move arrow shape
    MOUSE_CURSOR_RESIZE_NESW   = 8,     // The top-right to bottom-left diagonal resize/move arrow shape
    MOUSE_CURSOR_RESIZE_ALL    = 9,     // The omnidirectional resize/move cursor shape
    MOUSE_CURSOR_NOT_ALLOWED   = 10     // The operation-not-allowed shape
} MouseCursor;

// Gamepad buttons
typedef enum {
    GAMEPAD_BUTTON_UNKNOWN = 0,         // Unknown button, just for error checking
    GAMEPAD_BUTTON_LEFT_FACE_UP,        // Gamepad left DPAD up button
    GAMEPAD_BUTTON_LEFT_FACE_RIGHT,     // Gamepad left DPAD right button
    GAMEPAD_BUTTON_LEFT_FACE_DOWN,      // Gamepad left DPAD down button
    GAMEPAD_BUTTON_LEFT_FACE_LEFT,      // Gamepad left DPAD left button
    GAMEPAD_BUTTON_RIGHT_FACE_UP,       // Gamepad right button up (i.e. PS3: Triangle, Xbox: Y)
    GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,    // Gamepad right button right (i.e. PS3: Circle, Xbox: B)
    GAMEPAD_BUTTON_RIGHT_FACE_DOWN,     // Gamepad right button down (i.e. PS3: Cross, Xbox: A)
    GAMEPAD_BUTTON_RIGHT_FACE_LEFT,     // Gamepad right button left (i.e. PS3: Square, Xbox: X)
    GAMEPAD_BUTTON_LEFT_TRIGGER_1,      // Gamepad top/back trigger left (first), it could be a trailing button
    GAMEPAD_BUTTON_LEFT_TRIGGER_2,      // Gamepad top/back trigger left (second), it could be a trailing button
    GAMEPAD_BUTTON_RIGHT_TRIGGER_1,     // Gamepad top/back trigger right (first), it could be a trailing button
    GAMEPAD_BUTTON_RIGHT_TRIGGER_2,     // Gamepad top/back trigger right (second), it could be a trailing button
    GAMEPAD_BUTTON_MIDDLE_LEFT,         // Gamepad center buttons, left one (i.e. PS3: Select)
    GAMEPAD_BUTTON_MIDDLE,              // Gamepad center buttons, middle one (i.e. PS3: PS, Xbox: XBOX)
    GAMEPAD_BUTTON_MIDDLE_RIGHT,        // Gamepad center buttons, right one (i.e. PS3: Start)
    GAMEPAD_BUTTON_LEFT_THUMB,          // Gamepad joystick pressed button left
    GAMEPAD_BUTTON_RIGHT_THUMB          // Gamepad joystick pressed button right
} GamepadButton;


bool EngineIsKeyPressed(int key);

bool EngineIsMouseButtonPressed(int button); 

bool EngineIsKeyDown(int key);

bool EngineIsMouseButtonDown(int button);

float EngineGetMouseWheelMove(void);  

Vec2 EngineGetMouseDelta();

Vec2 EngineGetMousePosition(void);   

#define ENGINE_INPUT_H
#endif
