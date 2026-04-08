#include "render_interface.h"
#include "game_util.h"

//  ========================================================================
//              NOTE: Render Functions
//  ========================================================================

void UpdateShaderInfo(ShaderInfo & shaderInfo)
{
    
    if (GetFileModTime(shaderInfo.fsPath) != shaderInfo.fsWriteTime ||
        GetFileModTime(shaderInfo.vsPath) != shaderInfo.vsWriteTime)
    {
        UnloadShaderInfo(&shaderInfo);
        if (!LoadShaderInfo(&shaderInfo, shaderInfo.vsPath, shaderInfo.fsPath))
        {
            SM_ERROR(false, "Unable to load shader, vs(%s) fs(%s)", 
                     shaderInfo.vsPath, shaderInfo.fsPath);
        }
    }
    
    float size[2] =
    { 
        (float)GetScreenWidth(), (float)GetScreenHeight()
    };
    
    int32 timeLoc = GetShaderLocation(shaderInfo.shader, "time");
    int32 frameBufferSizeLoc = GetShaderLocation(shaderInfo.shader, "u_frameSize");
    
    SetShaderValue(shaderInfo.shader, frameBufferSizeLoc, size, SHADER_UNIFORM_VEC2);
    SetShaderValue(shaderInfo.shader, timeLoc, &gameState->time, SHADER_UNIFORM_FLOAT);
    
    
}

Rectangle GetCameraRect(Camera2D camera)
{
    //Vector2 delta = { 50.0f, 50.0f };
    Vector2 offset = Vector2Scale(camera.offset, 1.0f / camera.zoom);//  - delta;        
    Vector2 topleft = Vector2Subtract(camera.target, offset);
    
    Rectangle result = { topleft.x, topleft.y,
        offset.x * 2, offset.y * 2 };
    
    return result;
}

void DebugDrawPlayerActionState(ActionState state, int32 x, int32 y, int32 fontSize, Color color)
{
    char * stateTable[] = { 
        "NULL_STATE",
        "MOVE_STATE",
        "SPLIT_STATE",
        "ANIMATE_STATE",
        "FREEZE_STATE"
    };
    
    DrawText(TextFormat("Player Action State: %s", stateTable[state]), x, y, fontSize, color);
    
}

void DrawTile(IVec2 tilePos, Color color)
{
    Vector2 tileSize = Vector2 { MAP_TILE_SIZE, MAP_TILE_SIZE };
    Vector2 pivot = GetTilePivot(tilePos, tileSize);
    Rectangle rect = { pivot.x, pivot.y, MAP_TILE_SIZE, MAP_TILE_SIZE };
    DrawRectangleRec(rect, color);
}

void DrawTileMap(Camera2D camera, IVec2 startPos, IVec2 dim, Color tileColor, Color gridColor)
{
    int32 tileSize = MAP_TILE_SIZE;
    
    IVec2 endPos = startPos + dim;    
    
    // NOTE: Draw Tile Maps
    for (int32 y = startPos.y; y < endPos.y; y++)
    {
        for (int32 x = startPos.x; x < endPos.x; x++)
        {
            Rectangle source = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };
            
            if (CheckCollisionRecs(source, GetCameraRect(camera)))
            {
                
                DrawRectangleLinesEx(source, .5f, gridColor);
                // DrawRectangleLines(x * tileSize, y * tileSize, tileSize, tileSize, gridColor);
            }                     
        }
    }
}

void DrawSprite(Camera2D camera, Texture2D texture, Sprite & sprite, Vector2 topLeft, Vector2 tileSize, Color color = WHITE)
{
    
    SM_ASSERT(IsTextureValid(texture), "Texture is not valid");
    
    float offset = 0.01f;
    
    Rectangle source =
    {
        // (float)sprite.altasOffset.x + 1, (float)sprite.altasOffset.y + 1,
        // (float)sprite.spriteSize.x - 2, (float)sprite.spriteSize.y -2
        10.0f * ((float)sprite.altasOffset.x + offset), 10.0f * ((float)sprite.altasOffset.y + offset),
        10.0f * ((float)sprite.spriteSize.x - 2 * offset), 10.0f * ((float)sprite.spriteSize.y - 2 * offset)
    };
    
    Rectangle dest =
    {
        //topLeft.x + tileSize, topLeft.y + tileSize,
        topLeft.x, topLeft.y,
        tileSize.x, tileSize.y
    };
    
    DrawTexturePro(texture,  source,  dest, { 0, 0 }, 0, color);
}


void DrawError()
{
    char * text = "SOMETHING IS WRONG PLEASE UNDO(Z) OR RESET(R)";
    int32 size = 20;
    int32 x = (GetScreenWidth() - MeasureText(text, size)) / 2;
    int32 y = ((GetScreenHeight() - size) / 2);
    DrawText(text, x, y, size, RED);
}

void UpdateStarField(Vector3 * stars, Vector2 * starsScreenPos, Vector2 moveDir, float flySpeed, float dt,
                     int32 screenWidth, int32 screenHeight, int32 offsetX, int32 offsetY,
                     uint32 start, uint32 end)
{
    float moveSpeed = 500;
    for (uint32 i = start; i < end; i++)
    {
        stars[i].x += moveDir.x * dt * moveSpeed;
        stars[i].y += moveDir.y * dt * moveSpeed;
        // Update star's timer
        stars[i].z -= dt * flySpeed;
        // Calculate the screen position
        starsScreenPos[i] =
        {
            screenWidth  * 0.5f + stars[i].x/stars[i].z,
            screenHeight * 0.5f + stars[i].y/stars[i].z,
        };
        
        // If the star is too old, or offscreen, it dies and we make a new random one
        if ((stars[i].z < 0.0f) || (starsScreenPos[i].x < 0) || (starsScreenPos[i].y < 0.0f) ||
            (starsScreenPos[i].x > screenWidth) || (starsScreenPos[i].y > screenHeight))
        {
            stars[i].x = (float)GetRandomValue(-screenWidth / 2 + offsetX,
                                               screenWidth / 2 - offsetX);
            stars[i].y = (float)GetRandomValue(-screenHeight / 2 + offsetY,
                                               screenHeight / 2 - offsetY);
            stars[i].z = 1.0f;
        }
        
    }
}

void UpdateAndDrawStarFieldBG(StarFields * starFields, int32 offsetX = 0, int32 offsetY = 0, 
                              Vector2 moveDir = {0, 0})
{
    if (!starFields->initialized)
    {
        starFields->initialized = true;
        // Speed at which we fly forward
        starFields->flySpeed = 0.1f;
        // Setup the stars with a random position
        for (int32 i = 0; i < STAR_COUNT; i++)
        {
            starFields->stars[i].x = (float)GetRandomValue(-GetScreenWidth() / 2 + offsetX,
                                                           GetScreenWidth() / 2 - offsetX);
            starFields->stars[i].y = (float)GetRandomValue(-GetScreenHeight() / 2 + offsetY,
                                                           GetScreenHeight() / 2  - offsetY);
            starFields->stars[i].z = (float)GetRandomValue(0, INT_MAX) / INT_MAX;
        }
    }
    
    starFields->flySpeed = 0.2f;
    
    Vector3 * stars = starFields->stars;
    float flySpeed = starFields->flySpeed;
    
    int32 screenWidth = GetScreenWidth();
    int32 screenHeight = GetScreenHeight();
    static float time = 0;    
    float dt = GetFrameTime();
    time += dt;
    
    uint32 chunck = STAR_COUNT / 4;
    
    Vector2 sp[STAR_COUNT] = {};
    Vector2 * starsScreenPos = sp;
    
    for (uint32 tid = 0; tid < 4; tid++)
    {
        int32 start = tid * chunck;
        int32 end = (tid + 1) * chunck;
        if (end > STAR_COUNT) end = STAR_COUNT;
        
        UpdateStarField(stars, starsScreenPos, moveDir, flySpeed,  dt, screenWidth, screenHeight, offsetX, offsetY,
                        start, end);
        
    }
    
    for (uint32 i = 0; i < STAR_COUNT; i++)
    {
        float radius = Lerp(15, 5, stars[i].z);
        Color color = ColorLerp(DARKPURPLE, SKYBLUE, stars[i].z);
        DrawTexturePro(starFields->starTexture.texture,
                       Rectangle 
                       {
                           0,
                           0,
                           (float)starFields->starTexture.texture.width,
                           -(float)starFields->starTexture.texture.height
                       },
                       Rectangle
                       {
                           starsScreenPos[i].x,
                           starsScreenPos[i].y,
                           radius, radius
                       },
                       Vector2 { 0, 0 }, 0, color);
        
        // DrawCircleV(starsScreenPos[i], radius * 1.5f, color);
    }
    
}

void PostProcessing(RenderTexture2D & renderTarget, ShaderInfo & shader, 
                    int32 shake, real32 shakeStrength, float time)
{
    
    int32 tWidth = renderTarget.texture.width;
    int32 tHeight = renderTarget.texture.height;
    
    float size[2] =
    { 
        (float)GetScreenWidth(), (float)GetScreenHeight()
    };
    
    int32 shakeStrengthLoc = GetShaderLocation(shader.shader, "strength");
    int32 shakeLoc = GetShaderLocation(shader.shader, "shake");
    int32 offsetLoc = GetShaderLocation(shader.shader, "offset");
    static float val = 0; 
    val -= GetFrameTime() * 0.0015f;
    
    SetShaderValue(shader.shader, offsetLoc, &val, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader.shader, shakeStrengthLoc, &shakeStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader.shader, shakeLoc, &shake, SHADER_UNIFORM_INT);
    
    BeginShaderMode(shader.shader);
    
    Rectangle source =
    {
        0,0, (real32)tWidth, -(real32)tHeight
            //(screenWidth - mn) / 2, (screenHeight - mn) / 2, (float)mn, (float)-mn
    };
    
    Vector2 position = 
    {
        0, 0,
        //0.5f * (screenWidth - mn), 0.5f * (screenHeight - mn)
    };
    
    DrawTextureRec(renderTarget.texture, source, position, WHITE);
    
    EndShaderMode();
    
}

// NOTE:
// Code from https://github.com/raysan5/raylib/blob/master/examples/textures/textures_tiled_drawing.c
// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;
    
    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                       Rectangle{dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, Rectangle{dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }
        
        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                           Rectangle{dest.x, dest.y + dy, dest.width,  dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, Rectangle{dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }
        
        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                           Rectangle{dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }
            
            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                               Rectangle{dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }
        
        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                               Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }
            
            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, Rectangle{source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                               Rectangle{dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}


void DrawScrollingBackGround(Texture2D bgTexture, Color tint = WHITE)
{
    
    static Vector2 offset = { 0, 0 };
    offset += Vector2 { 1, -1 } * GetFrameTime() * 10;
    
    int32 tWidth = bgTexture.width;
    int32 tHeight = bgTexture.height;
    
    Rectangle bgSrc = { 
        0,
        0,
        (real32)tWidth, 
        (real32)tHeight
    };
    
    int32 divW = GetScreenWidth() / tWidth;
    int32 divH = GetScreenHeight() / tHeight;
    int32 remW = GetScreenWidth() % tWidth;
    int32 remH = GetScreenHeight() % tHeight;
    
    int32 size = 2;
    int32 w = size * tWidth * (divW + 1);
    int32 h = size * tWidth * (divH + 1);
    
    if (offset.x >= w) offset.x = 0;
    if (offset.y <= -h) offset.y = 0;
    
    Rectangle bgDst = {
        0, 
        0,
        (real32)(w), 
        (real32)(h),
    };
    
    
    DrawTextureTiled(bgTexture, bgSrc, bgDst,
                     offset, 
                     0, (real32)size, tint);
    
    DrawTextureTiled(bgTexture, bgSrc, bgDst, 
                     offset + Vector2{ -(real32)w, 0 },
                     0, (real32)size, tint);
    
    DrawTextureTiled(bgTexture, bgSrc, bgDst, 
                     offset + Vector2{ 0, (real32)h  },
                     0, (real32)size, tint);
    
    DrawTextureTiled(bgTexture, bgSrc, bgDst, 
                     offset + Vector2{ -(real32)w, (real32)h  },
                     0, (real32)size, tint);
    
}
    