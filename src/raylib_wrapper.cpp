/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */


#include "render_interface.h"
#include "engine_math.h"
#include "engine_input.h"
#include "engine_collision.h"
#include "engine_text.h"
#include "engine_lib.h"

#include "raylib.h"
#include "raymath.h"

Vec2::operator Vector2() const
{
    Vector2 v;
    v.x = x;
    v.y = y;
    
    return v;
}

EngineCamera2D::operator Camera2D() const
{
    Camera2D cam;
    cam.offset = offset;
    cam.target = target;
    cam.rotation = rotation;
    cam.zoom = zoom;
    return cam;
}

EngineTexture2D::operator Texture() const
{
    Texture t;
    t.id = id;
    t.width = width;
    t.height = height;
    t.mipmaps = mipmaps;
    t.format = format;
    return t;
}

Rect::operator Rectangle() const
{
    return { x, y, width, height };
}

Vec4::operator Color() const
{
    return { (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };
}

EngineTexture2D Texture2DToEngineTexture2D(Texture2D texture)
{
    EngineTexture2D t;
    t.id = texture.id;
    t.width = texture.width;
    t.height = texture.height;
    t.mipmaps = texture.mipmaps;
    t.format = texture.format;

    return t;
}

Rect RectangleToRect(Rectangle rec)
{
    return { rec.x, rec.y, rec.width, rec.height };
}

Vec2 Vector2ToVec2(Vector2 vec)
{
    return { vec.x, vec.y };
}

//  ========================================================================
//              NOTE: Render Interface Functions
//  ========================================================================
Vec2 EngineGetScreenToWorld2D(Vec2 position, EngineCamera2D camera)    // Get the world space position for a 2d camera screen space position
{
    Vector2 result = GetScreenToWorld2D(position, camera); 
    return { result.x, result.y };
}

EngineTexture2D EngineLoadTexture(const char * fileName)
{
    Texture2D result = LoadTexture(fileName);
    return Texture2DToEngineTexture2D(result);
}

bool EngineIsTextureValid(EngineTexture2D texture)
{
    return IsTextureValid(texture);
}

void EngineUnloadTexture(EngineTexture2D texture)
{
    UnloadTexture(texture);
}

void EngineDrawRectangle(int posX, int posY, int width, int height, Vec4 color)
{
    DrawRectangle(posX, posY, width, height, color);
}

void EngineDrawRectangleLinesEx(Rect rec, float lineThick, Vec4 color)
{
    DrawRectangleLinesEx(rec, lineThick, color);
}

void EngineDrawTexturePro(EngineTexture2D texture, Rect source, Rect dest, Vec2 origin, float rotation, Vec4 tint)
{
    DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

void EngineBeginDrawing()
{
    BeginDrawing();
}

void EngineClearBackground(Vec4 color)
{
    ClearBackground(color);
}

void EngineBeginMode2D(EngineCamera2D camera)
{
    BeginMode2D(camera);    
}

void EngineEndMode2D()
{
    EndMode2D();
}

void EngineDrawText(const char *text, int posX, int posY, int fontSize, Vec4 color)       // Draw text (using default font)
{
    DrawText(text, posX, posY, fontSize, color);
}

void EngineDrawFPS(int posX, int posY)
{
    DrawFPS(posX, posY);
}

void EngineEndDrawing()
{
    EndDrawing();
}



//  ========================================================================
//              NOTE: Engine LIB Functions
//  ========================================================================
float DeltaTime()
{
    return GetFrameTime();
}

bool EngineIsWindowResized()
{
    return IsWindowResized();
}

int EngineGetScreenWidth()
{
    return GetScreenWidth();
}

int EngineGetScreenHeight()
{
    return GetScreenHeight();
}

//  ========================================================================
//              NOTE: Engine INPUT Functions
//  ========================================================================
bool EngineIsKeyPressed(int key)
{
    return IsKeyPressed(key);
}

bool EngineIsMouseButtonPressed(int button)
{
    return IsMouseButtonPressed(button);
}

bool EngineIsKeyDown(int key)
{
    return IsKeyDown(key);
}

bool EngineIsMouseButtonDown(int button)
{
    return IsMouseButtonDown(button);
}

float EngineGetMouseWheelMove(void)
{
    return GetMouseWheelMove();
}

Vec2 EngineGetMouseDelta()
{
    Vector2 result = GetMouseDelta();

    return Vector2ToVec2(result);
    
}

Vec2 EngineGetMousePosition(void)
{
    return Vector2ToVec2(GetMousePosition());
}

//  ========================================================================
//              NOTE: Engine COLLISION Functions
//  ========================================================================
bool EngineCheckCollisionPointRec(Vec2 point, Rect rec) // Check if point is inside rectangle
{
    return CheckCollisionPointRec(point, rec);
}


//  ========================================================================
//              NOTE: Engine MATH Functions
//  ========================================================================
Vec4 EngineFade(Vec4 color, float alpha)                                 // Get color with alpha applied, alpha goes from 0.0f to 1.0f
{
    Color result = Fade(color, alpha);
    return { (float)result.r, (float)result.g, (float)result.b, (float)result.a };
}

Vec2 Vec2One()
{
    Vector2 result = Vector2One();
    return { result.x, result.y };
}

Vec2 Vec2Zero(void)                                                  // Vec with components value 0.0f
{
    Vector2 v = Vector2Zero();
    return { v.x, v.y };
}

Vec2 Vec2Scale(Vec2 v, float scale)                               // Scale vector (multiply by value)
{
    Vector2 vec = Vector2Scale(v, scale);
    return { vec.x, vec.y };
}

Vec2 Vec2Subtract(Vec2 v1, Vec2 v2)                            // Subtract two vectors (v1 - v2)
{
    Vector2 vec = Vector2Subtract(v1, v2);
    return { vec.x, vec.y };
}

Vec2 Vec2Add(Vec2 v1, Vec2 v2)                                 // Add two vectors (v1 + v2)
{
    Vector2 vec = Vector2Add(v1, v2);
    return { vec.x, vec.y };
}

float Vec2Distance(Vec2 v1, Vec2 v2)                              // Calculate distance between two vectors
{
    return Vector2Distance(v1, v2);
}

int Vec2Equals(Vec2 p, Vec2 q)                                    // Check whether two given vectors are almost equal
{
    return Vector2Equals(p, q);
}

#if 0
Vec2 Vec2AddValue(Vec2 v, float add);                              // Add vector and float value
Vec2 Vec2SubtractValue(Vec2 v, float sub);                         // Subtract vector by float value
float Vec2Length(Vec2 v);                                             // Calculate vector length
float Vec2LengthSqr(Vec2 v);                                          // Calculate vector square length
float Vec2DotProduct(Vec2 v1, Vec2 v2);                            // Calculate two vectors dot product
float Vec2DistanceSqr(Vec2 v1, Vec2 v2);                           // Calculate square distance between two vectors
float Vec2Angle(Vec2 v1, Vec2 v2);                                 // Calculate angle from two vectors
Vec2 Vec2Scale(Vec2 v, float scale);                               // Scale vector (multiply by value)
Vec2 Vec2Multiply(Vec2 v1, Vec2 v2);                            // Multiply vector by vector
Vec2 Vec2Negate(Vec2 v);                                           // Negate vector
Vec2 Vec2Divide(Vec2 v1, Vec2 v2);                              // Divide vector by vector
Vec2 Vec2Normalize(Vec2 v);                                        // Normalize provided vector
Vec2 Vec2Transform(Vec2 v, Mat4 mat);                            // Transforms a Vec2 by a given Matrix
Vec2 Vec2Lerp(Vec2 v1, Vec2 v2, float amount);                  // Calculate linear interpolation between two vectors
Vec2 Vec2Reflect(Vec2 v, Vec2 normal);                          // Calculate reflected vector to normal
Vec2 Vec2Rotate(Vec2 v, float angle);                              // Rotate vector by angle
Vec2 Vec2MoveTowards(Vec2 v, Vec2 target, float maxDistance);   // Move Vec towards target
Vec2 Vec2Invert(Vec2 v);                                           // Invert the given vector
Vec2 Vec2Clamp(Vec2 v, Vec2 min, Vec2 max);                  // Clamp the components of the vector between min and max values specified by the given vectors
Vec2 Vec2ClampValue(Vec2 v, float min, float max);                 // Clamp the magnitude of the vector between two min and max values
#endif

//  ========================================================================
//              NOTE: Engine TEXT Functions
//  ========================================================================
template <typename ...Args>
const char * EngineTextFormat(const char *text, Args... args)                                        // Text formatting with variables (sprintf() style)
{
    return TextFormat(text, args...);
}
