#if !defined(GAME_UI_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define RAYLIB_GUI_STYLE_PATH  "src/vendor/raygui/styles/cyber/style_cyber.rgs"

struct Arrow
{
    Sprite sprite;
    SpriteID id;

    int tileSize;
    
    Vector2 topLeftPos;
    bool show = true;
};

struct GameButton
{
    Sprite background;
    SpriteID id;
    
    int textLen;
    bool hover = false;
    
    char * buttonText;

    Rectangle bounds;
};

#define GAME_UI_H
#endif
