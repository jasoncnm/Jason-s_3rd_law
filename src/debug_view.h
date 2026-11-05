/* date = November 3rd 2026 6:05 pm */

#ifndef DEBUG_VIEW_H
#define DEBUG_VIEW_H


void DebugViewControl(bool8 & debugView, bool8 & showFog)
{
    // NOTE: Developer Debug Cheats
    {
        
        if (IsKeyPressed(KEY_EQUAL))
        {
            SetShake(0.05f, 0.001f);
            gameState->starCount++;
        }
        
        
        if (IsKeyPressed(KEY_NINE))
        {
            UnloadTexture(gameState->bgTexture);
            UnloadTexture(gameState->textureAltas);    // Unload render texture
            gameState->textureAltas = LoadTexture(TEXTURE_PATH);
            gameState->bgTexture = LoadTexture(BACKGROUND_PATH);
        }
        
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            debugView = !debugView;
        }
        
        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            showFog = !showFog;
        }
    }
}

void DrawDebugViewGame(Entity * followEnt)
{
    Color colors[] = {
        LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN, WHITE, BLACK, MAGENTA, RAYWHITE,
    };
    
    int32 colorCount = ArrayCount(colors);
    
    static Color * mColors = (Color *)BumpAllocArray(gameMemory->persistentStorage,
                                                     gameState->tileMapCount,
                                                     sizeof(Color));
    
    static bool * set = (bool *)BumpAllocArray(gameMemory->persistentStorage,
                                               gameState->tileMapCount,
                                               sizeof(bool));
    
    for (uint32 mapIndex = 0; mapIndex < gameState->tileMapCount; mapIndex++)
    {
        Map * tileMap = gameState->tileMaps + mapIndex;
        
        if (!set[mapIndex])
        {
            set[mapIndex] = true;
            int32 colorIndex = GetRandomValue(0, colorCount-1);
            mColors[mapIndex] = colors[colorIndex];
        }
        
        // NOTE: DrawCenterTile
        IVec2 mapCenter = tileMap->tilePos + 
            IVec2 { tileMap->width / 2 + 1, tileMap->height / 2 + 1 };
        
        DrawTile(mapCenter, WHITE);
        
        // NOTE: Could be a setting
        DrawTileMap(gameState->camera.base, tileMap->tilePos, 
                    IVec2{ tileMap->width, tileMap->height },
                    mColors[mapIndex], mColors[mapIndex]);
        Map & map = gameState->tileMaps[mapIndex];
        Rectangle mapRect = 
        { 
            (real32)map.tilePos.x * MAP_TILE_SIZE, (real32)map.tilePos.y * MAP_TILE_SIZE, 
            (real32)map.width * MAP_TILE_SIZE,  (real32)map.height * MAP_TILE_SIZE
        };
        
        DrawRectangleLinesEx(mapRect, 5, BLUE);
    }
    
    
    // NOTE: draw mouse tilePos
    Vector2 mousePos = GetMousePosition();
    Vector2 mouseWorld = GetScreenToWorld2D(mousePos, gameState->camera.base);
    
    IVec2 tilePos = PivotToTilePos(mouseWorld, Vector2{ 0 });
    DrawTile(tilePos, RED);
    DrawLineBezier(GetPlayer()->pivot, GetTilePivot(tilePos, DEFAULT_TILE_SIZE), 2, GREEN);
    
    Vector2 textWorld = GetScreenToWorld2D({ 1200.0f, 200.0f }, gameState->camera.base);
    DrawText(TextFormat("Mouse TilePos (%d, %d)", tilePos.x, tilePos.y), (int)textWorld.x, (int)textWorld.y, 10, GREEN);
    
    // Draw rectangle outline with extended parameters
    Rectangle cameraRect = GetCameraRect(gameState->camera.base);
    DrawRectangleLinesEx(cameraRect, 1, RED);
    
    if (followEnt)
    {
        Vector2 center = followEnt->pivot + Vector2 { followEnt->tileSize.x * 0.5f, followEnt->tileSize.y * 0.5f };
        DrawCircleV(center, 5,  RED);
        DrawCircleV(gameState->camera.base.target, 5, YELLOW);
    }
    
}

void DrawDebugViewUI(Entity * followEnt, real32 * contrast, real32 * saturation, real32 * brightness)
{
    // NOTE: UI Draw Game Informations
    Entity * player = GetEntity(gameState->playerEntityIndex);
    if (player)
    {
        IVec2 centerPos = player->tilePos;
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), gameState->camera.base);
    }
    
    
    DrawText(TextFormat("Camera target: (%.2f, %.2f)\nCamera offset: (%.2f, %.2f)\nCamera Zoom: %.2f\nCamera State: %s",
                        gameState->camera.base.target.x, gameState->camera.base.target.y,
                        gameState->camera.base.offset.x, gameState->camera.base.offset.y, gameState->camera.base.zoom,
                        GetCameraState(gameState->camera)), 10, 50, 20, RAYWHITE);
    
    if (followEnt)
    {
        DrawText(TextFormat("Follow Entity Type: %s(%d)", GetEntityType(followEnt), followEnt->entityIndex),
                 10, 200, 25, YELLOW);
    }
    
    if (GetPlayer()->actionState == FREEZE_STATE)
    {
        DrawText(TextFormat("Player Freezed"),
                 10, 220, 25, YELLOW);
    }
    else if (GetPlayer()->actionState == MOVE_STATE)
    {
        DrawText(TextFormat("Player Moved"),
                 10, 220, 25, YELLOW);
    }
    
    real32 vert = 445;
    real32 gap = 100;
    GuiSliderBar(Rectangle{ 100, vert, 400, 30 }, "Contrast", TextFormat("%.1f", contrast), contrast, -100.0f, 100.0f);
    GuiSliderBar(Rectangle{ 100, vert + gap, 400, 30 }, "Saturation", TextFormat("%.1f", saturation), saturation, -100.0f, 100.0f);
    GuiSliderBar(Rectangle{ 100, vert + gap * 2, 400, 30 }, "Brightness", TextFormat("%.1f", brightness), brightness, 0, 10.0f);
    
    DrawText(TextFormat("Stars Collected: %d",
                        gameState->starCount),
             10, 250, 25, ORANGE);
    
    
    
    if (player)
    {
        DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: (%.2f, %.2f),  Entity Count: %i",
                            player->tilePos.x, player->tilePos.y,
                            player->mass, player->tileSize.x , player->tileSize.y,  gameState->entities.count), 10, 140, 20, GREEN);
    }
#if 0
    
    DrawText(TextFormat("TotalLine: %d, Connection: %d, Door: %d, TotalSource: %d",
                        Cable_Indices.count,
                        CP_Indices.count,
                        Door_Indices.count,
                        Source_Indices.count
                        ), 10, 280, 20, GREEN);
    
    DrawText(TextFormat("TotalEntities: %d, TotalWall: %d, TotalDoor: %d, TotalCable: %d, TotalGlass: %d, TotalSlime %d, TotalBlock: %d, TotalPit: %d, TotalPortal %d", gameState->entities.count,
                        gameState->entityTable[LAYER_WALL].count,
                        gameState->entityTable[LAYER_DOOR].count,
                        gameState->entityTable[LAYER_CABLE].count,
                        gameState->entityTable[LAYER_GLASS].count,
                        gameState->entityTable[LAYER_SLIME].count,
                        gameState->entityTable[LAYER_BLOCK].count,
                        gameState->entityTable[LAYER_PIT].count,
                        gameState->entityTable[LAYER_PORTAL].count
                        ), 10, 250, 20, GREEN);
    
    DrawText(TextFormat("Player pivot (%.2f, %.2f), mouse world (%.2f, %.2f)",
                        player->pivot.x, player->pivot.y, mousePos.x, mousePos.y ), 10, 200, 20, GREEN);
    
    DrawText(TextFormat("Player Points at tile (%i, %i), Player Mass: %i, Player tile size: %.2f,  Entity Count: %i",
                        centerPos.x, centerPos.y,
                        player->mass, player->tileSize,  gameState->entities.count), 10, 140, 20, GREEN);
    
    DrawText(TextFormat("UndoStack Count: %d, last index: %d",
                        gameState->undoStack.count, gameState->undoStack.last - 1), 10, 170, 20, GREEN);
    
    
    int32 posX = GetScreenWidth() - MeasureText("Entity Action State: FFFFFFFFFFFFFFFFFFFF", 20);
    DebugDrawPlayerActionState(player->actionState, posX, 50, 20, IntToRGBA(0x923eed));
    
    if (gameState->simulating)
    {
        DrawText("Game Simulating", gameState->screenWidth / 4, gameState->screenHeight / 4, 20, RED);
    }
    
    DrawText(TextFormat("Move buffer timer: %.2fs", gameState->moveBufferTimer), 10, 350, 20, GREEN);
#endif
    
}

#endif //DEBUG_VIEW_H
