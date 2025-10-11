#if !defined(RENDER_INTERFACE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "assets.h"
#include "engine_lib.h"

#define TEXTURE_PATH "Assets/Texture/SpriteAtlas.png"
#define MAX_TRANSFORM 1000

//  ========================================================================
//              NOTE: Render Structs
//  ========================================================================

#define E_LIGHTGRAY  (Vec4){ 200, 200, 200, 255 }   // Light Gray
#define E_GRAY       (Vec4){ 130, 130, 130, 255 }   // Gray
#define E_DARKGRAY   (Vec4){ 80, 80, 80, 255 }      // Dark Gray
#define E_YELLOW     (Vec4){ 253, 249, 0, 255 }     // Yellow
#define E_GOLD       (Vec4){ 255, 203, 0, 255 }     // Gold
#define E_ORANGE     (Vec4){ 255, 161, 0, 255 }     // Orange
#define E_PINK       (Vec4){ 255, 109, 194, 255 }   // Pink
#define E_RED        (Vec4){ 230, 41, 55, 255 }     // Red
#define E_MAROON     (Vec4){ 190, 33, 55, 255 }     // Maroon
#define E_GREEN      (Vec4){ 0, 228, 48, 255 }      // Green
#define E_LIME       (Vec4){ 0, 158, 47, 255 }      // Lime
#define E_DARKGREEN  (Vec4){ 0, 117, 44, 255 }      // Dark Green
#define E_SKYBLUE    (Vec4){ 102, 191, 255, 255 }   // Sky Blue
#define E_BLUE       (Vec4){ 0, 121, 241, 255 }     // Blue
#define E_DARKBLUE   (Vec4){ 0, 82, 172, 255 }      // Dark Blue
#define E_PURPLE     (Vec4){ 200, 122, 255, 255 }   // Purple
#define E_VIOLET     (Vec4){ 135, 60, 190, 255 }    // Violet
#define E_DARKPURPLE (Vec4){ 112, 31, 126, 255 }    // Dark Purple
#define E_BEIGE      (Vec4){ 211, 176, 131, 255 }   // Beige
#define E_BROWN      (Vec4){ 127, 106, 79, 255 }    // Brown
#define E_DARKBROWN  (Vec4){ 76, 63, 47, 255 }      // Dark Brown

#define E_WHITE      (Vec4){ 255, 255, 255, 255 }   // White
#define E_BLACK      (Vec4){ 0, 0, 0, 255 }         // Black
#define E_BLANK      (Vec4){ 0, 0, 0, 0 }           // Blank (Transparent)
#define E_MAGENTA    (Vec4){ 255, 0, 255, 255 }     // Magenta
#define E_RAYWHITE   (Vec4){ 245, 245, 245, 255 }   // My own White (raylib logo)

struct Camera2D;
struct EngineCamera2D
{
    Vec2 offset;         // Camera offset (displacement from target)
    Vec2 target;         // Camera target (rotation and zoom origin)
    float rotation;         // Camera rotation in degrees
    float zoom;             // Camera zoom (scaling), should be 1.0f by default

    operator Camera2D() const;    
    
};

struct Texture;
struct EngineTexture2D {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture base width
    int height;             // Texture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)

    operator Texture() const;
    
};


//  ========================================================================
//              NOTE: Render Globals
//  ========================================================================
static EngineTexture2D texture;

//  ========================================================================
//              NOTE: Render Functions (External)
//  ========================================================================
EngineTexture2D EngineLoadTexture(const char * fileName);

bool EngineIsTextureValid(EngineTexture2D textrue);

void EngineUnloadTexture(EngineTexture2D texture); 

void EngineDrawRectangle(int posX, int posY, int width, int height, Vec4 color);

void EngineDrawRectangleLinesEx(Rect rec, float lineThick, Vec4 color);

void EngineDrawTexturePro(EngineTexture2D texture, Rect source, Rect dest, Vec2 origin, float rotation, Vec4 tint);

Vec2 EngineGetScreenToWorld2D(Vec2 position, EngineCamera2D camera);    // Get the world space position for a 2d camera screen space position

void EngineBeginDrawing();

void EngineClearBackground(Vec4 color);
    
void EngineBeginMode2D(EngineCamera2D camera);

void EngineEndMode2D();

void EngineDrawText(const char *text, int posX, int posY, int fontSize, Vec4 color);       // Draw text (using default font)

void EngineDrawFPS(int posX, int posY);

void EngineEndDrawing();
   

//  ========================================================================
//              NOTE: Render Functions (Internal)
//  ========================================================================

void InitTexture()
{
    texture = EngineLoadTexture(TEXTURE_PATH);
    SM_ASSERT(EngineIsTextureValid(texture), "Unable to load file (%s) to texture", TEXTURE_PATH);
}

void DeInitTexture()
{
    EngineUnloadTexture(texture);
}

void DrawTileMap(IVec2 startPos, IVec2 dim, Vec4 tileColor, Vec4 gridColor)
{
    int tileSize = MAP_TILE_SIZE;

    IVec2 endPos = startPos + dim;    
    
    // NOTE: Draw Tile Maps
    for (int y = startPos.y; y < endPos.y; y++)
    {
        for (int x = startPos.x; x < endPos.x; x++)
        {
            // NOTE: Draw tiles from id (and tile borders)
            EngineDrawRectangle(
                x * tileSize,
                y * tileSize,
                tileSize,
                tileSize,
                tileColor);

            Rect source = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };
            EngineDrawRectangleLinesEx(source, .5f, gridColor);
            // DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, gridColor);
                    
        }
    }
}

void DrawSprite(EngineTexture2D & texture, Sprite & sprite, Vec2 topLeft, float tileSize = 32, Vec4 color = E_WHITE)
{
            
    Rect source =
        {
            (float)sprite.altasOffset.x + 1, (float)sprite.altasOffset.y + 1,
            (float)sprite.spriteSize.x - 2, (float)sprite.spriteSize.y -2
        };

    Rect dest =
        {
            topLeft.x + tileSize, topLeft.y + tileSize,
            tileSize, tileSize
        };
        
    
 // Draw a part of a texture defined by a rectangle with 'pro' parameters
    EngineDrawTexturePro(texture,  source,  dest, { dest.width, dest.height }, 0, color);
}

#define RENDER_INTERFACE_H
#endif
