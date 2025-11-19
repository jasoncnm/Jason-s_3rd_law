/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "tween_controller.h"

inline Entity * GetSource(int sourceIndex)
{
    int cableIndex = Source_Indices[sourceIndex];
    Entity * source = GetEntity(Cable_Indices[cableIndex]);

    return source;
}

inline bool8 SameSide(Entity * door, IVec2 tilePos, IVec2 reachDir)
{
    SM_ASSERT(door->type == ENTITY_TYPE_ELECTRIC_DOOR && door->cableType == CABLE_TYPE_DOOR,
              "Entity is not a door");
    
    bool8 result = false;

    if (door->tilePos == tilePos)
    {
        if (Abs(reachDir.x) > 0)
        {
            result = door->left || door->right;
        }
        else if (Abs(reachDir.y) > 0)
        {
            result = door->up || door->down;
        }
    }
    
    return result;
}

inline void UnfreezeSlimes(Entity * door)
{
    SM_ASSERT(door, "door is inactive");
    SM_ASSERT(door->cableType == CABLE_TYPE_DOOR, "entity is not a door");

    for (uint32 i = 0; i < CP_Indices.count; i++)
    {
        Entity * connect = GetEntity(Cable_Indices[CP_Indices[i]]);
        if (connect && connect->sourceIndex == door->sourceIndex)
        {
            auto & slimeEntityIndices = gameState->entityTable[LAYER_SLIME];
            for (uint32 i = 0; i < slimeEntityIndices.count; i++)
            {
                Entity * slime = GetEntity(slimeEntityIndices[i]);
                if (slime)
                {
                    SetActionState(slime, MOVE_STATE);
                }
            }
        }
    }
}

inline bool8 PowerOnCable(Entity * cable, bool8 & end)
{
    bool8 doorOpened = false;
    if (cable->cableType == CABLE_TYPE_DOOR)
    {
        doorOpened = true;
        if (!cable->open)
        {
            cable->open = true;

            UnfreezeSlimes(cable);

            IVec2 offset = { 0 };
            IVec2 bounceDir = { 0 };
            
            switch(cable->spriteID)
            {

                case SPRITE_DOOR_LEFT_CLOSE:
                {
                    offset = { 1, -1 };
                    bounceDir = { 1, 0 };
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    cable->spriteID = SPRITE_DOOR_LEFT_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_LEFT_OPEN);
                    break;
                }
                case SPRITE_DOOR_RIGHT_CLOSE:
                {
                    offset = { -1, 1 };
                    bounceDir = { -1, 0 };
                    cable->spriteID = SPRITE_DOOR_RIGHT_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_RIGHT_OPEN);
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    
                    break;
                }
                case SPRITE_DOOR_TOP_CLOSE:
                {
                    offset = { -1, 1 };
                    bounceDir = { 0, 1 };
                    cable->spriteID = SPRITE_DOOR_TOP_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_TOP_OPEN);
                    cable->left = cable->right = true;
                    cable->up = cable->down = false;
                    
                    break;
                }
                case SPRITE_DOOR_DOWN_CLOSE:
                {
                    offset = { 1, -1 };
                    bounceDir = { 0, -1 };
                    cable->spriteID = SPRITE_DOOR_DOWN_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_DOWN_OPEN);
                    cable->left = cable->right = true;
                    cable->up = cable->down = false;
                    
                    break;
                }

                case SPRITE_DOOR_LEFT_R_CLOSE:
                {
                    offset = { 1, 1 };
                    bounceDir = { 1, 0 };
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    cable->spriteID = SPRITE_DOOR_LEFT_R_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_LEFT_R_OPEN);
                    break;
                }
                case SPRITE_DOOR_RIGHT_R_CLOSE:
                {
                    offset = { -1, -1 };
                    bounceDir = { -1, 0 };
                    cable->spriteID = SPRITE_DOOR_RIGHT_R_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_RIGHT_R_OPEN);
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    
                    break;
                }
                case SPRITE_DOOR_TOP_R_CLOSE:
                {
                    offset = { 1, 1 };
                    bounceDir = { 0, 1 };
                    cable->spriteID = SPRITE_DOOR_TOP_R_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_TOP_R_OPEN);
                    cable->left = cable->right = true;
                    cable->up = cable->down = false;
                    
                    break;
                }
                case SPRITE_DOOR_DOWN_R_CLOSE:
                {
                    offset = { -1, -1 };
                    bounceDir = { 0, -1 };
                    cable->spriteID = SPRITE_DOOR_DOWN_R_OPEN;
                    cable->sprite = GetSprite(SPRITE_DOOR_DOWN_R_OPEN);
                    cable->left = cable->right = true;
                    cable->up = cable->down = false;
                    
                    break;
                }

                default:
                {
                    SM_ASSERT(false, "door has no sprite");
                }
                
            }

            EntityLayer layers[] = { LAYER_BLOCK };
            Entity * entity = FindEntityByLocationAndLayers(cable->tilePos + bounceDir, layers, ArrayCount(layers));
            if (entity)
            {
                Vector2 moveStart = GetTilePivot(entity);
                BounceEntity(entity, bounceDir);
                Vector2 moveEnd = GetTilePivot(entity);

                if (!Vector2Equals(moveStart, moveEnd))
                {
                    float dist = Vector2Distance(moveStart, moveEnd);
                    float iDist = dist / MAP_TILE_SIZE;

                    TweenParams params = {};
                    params.paramType = PARAM_TYPE_VECTOR2;
                    params.startVec2 = moveStart;
                    params.endVec2 = moveEnd;
                    params.realVec2  = &entity->pivot;

                                            
                    AddTween(entity->tweenController, CreateTween(params, nullptr,  BOUNCE_SPEED, iDist));
                    OnPlayEvent(&entity->tweenController);
                }
                else
                {
                    ShiftEntities(entity->tilePos, bounceDir);                    
                }
            }
            else
            {
                EntityLayer layers[] = { LAYER_SLIME };
                entity = FindEntityByLocationAndLayers(cable->tilePos + bounceDir, layers, ArrayCount(layers));
                if (entity)
                {
                    Vector2 moveStart = GetTilePivot(entity);
                    BounceEntity(entity, bounceDir);
                    Vector2 moveEnd = GetTilePivot(entity);
                    if (!Vector2Equals(moveStart, moveEnd))
                    {
                
                        float dist = Vector2Distance(moveStart, moveEnd);
                        float iDist = dist / MAP_TILE_SIZE;

                        TweenParams params = {};
                        params.paramType = PARAM_TYPE_VECTOR2;
                        params.startVec2 = moveStart;
                        params.endVec2 = moveEnd;
                        params.realVec2  = &entity->pivot;

                                            
                        AddTween(entity->tweenController, CreateTween(params, nullptr,  BOUNCE_SPEED, iDist));
                        OnPlayEvent(&entity->tweenController);
                        
                    }
                    else
                    {
                        ShiftEntities(entity->tilePos, bounceDir);                    
                    }
                }
            }

            cable->tilePos = cable->tilePos + offset;
        }
        
        GetSource(cable->sourceIndex)->sourceLit = true;
        end = true;
    }
    else if (cable->cableType == CABLE_TYPE_CONNECTION_POINT)
    {
        cable->hasPower = true;
        if (!cable->conductive)
        {
            EntityLayer layers[] = { LAYER_SLIME, LAYER_BLOCK };
            bool8 has = FindEntityByLocationAndLayers(cable->tilePos, layers, ArrayCount(layers)) != nullptr;

            if (has)
            {
                cable->conductive = true;
                end = false;
            }
            else
            {
                end = true;
            }
        }
        else
        {
            end = false;
        }
    }
    else
    {
        cable->conductive = true;
        cable->sprite = GetSprite(GetCablePowerOnID(cable->spriteID));
        end = false;
    }

    return doorOpened;
}

bool8 OnSourcePowerOn(int currentIndex)
{

    Array<int, CABLE_MAX_CALL_STACK> callStack  = {};
    callStack.Add(currentIndex);
    bool8 doorOpened = false;

    while (!callStack.IsEmpty())
    {
        int index = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(Cable_Indices[index]);
        SM_ASSERT(cable, "entity is not active");

        bool8 end = false;
        bool _open = PowerOnCable(cable, end);
        doorOpened = _open || doorOpened;
        if (!end)
        {
            // return doorOpened;
            int indexes[4] =
                {
                    cable->leftIndex,
                    cable->rightIndex,
                    cable->upIndex,
                    cable->downIndex
                };
    
            for (int i = 0; i < 4; i++)
            {
                int id = indexes[i];
                if (id >= 0)
                {
                    callStack.Add(id);
                }
            }
            // return doorOpened;
        }
    }

    return doorOpened;
}

void ShutDownPower(int currentCableIndex)
{
    Array<int, CABLE_MAX_CALL_STACK> callStack = {};
    callStack.Add(currentCableIndex);

    while(!callStack.IsEmpty())
    {
        int currentIndex = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(Cable_Indices[currentIndex]);

        cable->conductive = false;
        if (cable->cableType == CABLE_TYPE_CONNECTION_POINT)
        {
            cable->hasPower = false;
            EntityLayer layers[] = { LAYER_SLIME };
            Entity * slime = FindEntityByLocationAndLayers(cable->tilePos, layers, ArrayCount(layers));
            if (slime)
            {
                SetActionState(slime, MOVE_STATE);
            }
        }
        else
        {
            cable->sprite = GetSprite(cable->spriteID);        
        }
    
        int indexes[4] =
            {
                cable->leftIndex,
                cable->rightIndex,
                cable->upIndex,
                cable->downIndex
            };
    
        for (int i = 0; i < 4; i++)
        {
            int id = indexes[i];
            if (id >= 0)
            {
                callStack.Add(id);
            }
        }
    }
}


inline bool8 DoorBlocked(Entity * door, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool8 result = false;

    if (reachDir.x == 1)
    {
        result = (door->spriteID == SPRITE_DOOR_RIGHT_CLOSE || door->spriteID == SPRITE_DOOR_RIGHT_R_CLOSE);
    }
    else if (reachDir.x == -1)
    {
        result = (door->spriteID == SPRITE_DOOR_LEFT_CLOSE || door->spriteID == SPRITE_DOOR_LEFT_R_CLOSE);
    }
    else if (reachDir.y == 1)
    {
        result = (door->spriteID == SPRITE_DOOR_DOWN_CLOSE || door->spriteID == SPRITE_DOOR_DOWN_R_CLOSE);
        
    }
    else if (reachDir.y == -1)
    {
        result = (door->spriteID == SPRITE_DOOR_TOP_CLOSE || door->spriteID == SPRITE_DOOR_TOP_R_CLOSE);
    }
    
    return result;
}

inline bool8 CheckDoor(IVec2 tilePos)
{
    
    bool8 result = false;
    for (uint32 i = 0; i < Door_Indices.count; i++)
    {
        Entity * door = GetEntity(Cable_Indices[Door_Indices[i]]);
        if (door && door->tilePos == tilePos)
        {
            result = true;
            break;
        }
    }
    return result;
}

void SetUpElectricDoor()
{
    
    for (uint32 sourceIndex = 0; sourceIndex < Source_Indices.count; sourceIndex++)
    {
        Array<int, CABLE_MAX_CALL_STACK> callStack = {};

        {
            int currentIndex = Source_Indices[sourceIndex];
            callStack.Add( currentIndex );
        }

        uint32 visitCount = Cable_Indices.count;
        bool8 * visited = (bool8 *)BumpAllocArray(gameMemory->transientStorage, visitCount, sizeof(bool8));
        for (uint32 i = 0; i < visitCount; i++) visited[i] = false;
        
        while(!callStack.IsEmpty())
        {
            int currentIndex = callStack.last();
            callStack.RemoveLast();
            
            visited[currentIndex] = true;

            Entity * current = GetEntity(Cable_Indices[currentIndex]);
            current->sourceIndex = sourceIndex;

            if (current->cableType != CABLE_TYPE_DOOR)
            {
                for (uint32 i = 0; i < Cable_Indices.count; i++)
                {
                    if (!visited[i])
                    {
                        
                        Entity * entity = GetEntity(Cable_Indices[i]);
                        SM_ASSERT(entity, "entity is not active");
                    
                        IVec2 offset = entity->tilePos - current->tilePos;

                        if (offset.SqrMagnitude() > 1) continue;

                        // NOTE A in left B in right
                        // NOTE They are connected only if A has right and B has left connection
                        if (offset.x == 1 && (current->right && entity->left))
                        {
                            current->rightIndex = i;
                            // entity->leftIndex = currentIndex;
                        }
                        // NOTE A in right B in left
                        // NOTE They are connected only if A has left and B has right connection
                        else if (offset.x == -1 && (current->left && entity->right))
                        {
                            current->leftIndex = i;
                            // entity->rightIndex = currentIndex;
                        }
                        // NOTE A in Up B in Down
                        // NOTE They are connected only if A has Down and B has Up connection
                        else if (offset.y == 1 && (current->down && entity->up))
                        {
                            current->downIndex = i;
                            // entity->upIndex = currentIndex;
                        }
                        // NOTE A in Down B in Up
                        // NOTE They are connected only if A has up and B has down connection         
                        else if (offset.y == -1 &&  (current->up && entity->down))
                        {
                            current->upIndex = i;
                            // entity->downIndex = currentIndex;
                        }
                        else
                        {
                            continue;
                        }

                        callStack.Add(i);
                    }
                }
            }
        }
    }
}


inline void UpdateElectricDoor()
{
    
    for (uint32 i = 0; i < Source_Indices.count; i++)
    {
        Entity * source = GetSource(i);
        SM_ASSERT(source, "Entity is not active");
        if (source->sourceLit) continue;

        bool8 has = false;
        int sourceCableIndex = Source_Indices[i];

        EntityLayer layers[] = { LAYER_BLOCK };
        Entity * block = FindEntityByLocationAndLayers(source->tilePos, layers, ArrayCount(layers));
        if (block && block->tweenController.NoTweens())
        {
            if (!source->conductive)
                OnSourcePowerOn(sourceCableIndex);
        }
        else
        {
            ShutDownPower(sourceCableIndex);
        }
        
    }

    for (uint32 i = 0; i < CP_Indices.count; i++)
    {
        Entity * connection = GetEntity(Cable_Indices[CP_Indices[i]]);
        SM_ASSERT(connection, "Entity is not active");
        
        if (!connection->conductive)
        {
            auto & slimeIndexTable = gameState->entityTable[LAYER_SLIME];
            for (uint32 slimeIndex = 0; slimeIndex < slimeIndexTable.count; slimeIndex++)
            {
                Entity * slime = GetEntity(slimeIndexTable[slimeIndex]);
                if (slime)
                {
                    if (CheckCollisionRecs(GetEntityRect(slime), GetEntityRect(connection)))
                    {
                        if (connection->hasPower)
                        {
                            connection->conductive = true;

                            int sourceCableIndex = Source_Indices[connection->sourceIndex];
                            bool8 doorOpened = OnSourcePowerOn(sourceCableIndex);

                            if (!doorOpened)
                            {                                
                                //      1.force slime stay at the connection point location
                                //      2. find if the slime can attach to any objects at that location
                                //         - If yes, attach slime to that object and set actionState FREEZE_STATE
                                //         - If no,  move slime to the center of the grid and set actionState to FREEZE_STATE
                                SetActionState(slime, FREEZE_STATE);
                                IVec2 freezePos = connection->tilePos;
                                Vector2 startPivot = slime->pivot;
                                Vector2 endPivot = {};

                                FindAttachableResult result = FindAttachable(freezePos + slime->attachDir, slime->attachDir);

                                if (result.has)
                                {
                                    // NOTE: This is the yes logic
                                    SetEntityPosition(slime, result.entity, freezePos);
                                    endPivot = GetTilePivot(slime);
                                }
                                else
                                {
                                    // NOTE: This is the no logic
                                    SetEntityPosition(slime, nullptr, freezePos);
                                    endPivot = GetTilePivot(freezePos, slime->tileSize);
                                }

                                slime->tweenController.Reset();

                                TweenParams params = {};
                                params.paramType = PARAM_TYPE_VECTOR2;
                                params.startVec2 = startPivot;
                                params.endVec2   = endPivot;
                                params.realVec2  = &slime->pivot;

                                AddTween(slime->tweenController, CreateTween(params));
                                OnPlayEvent(&slime->tweenController);
                            }
                            else
                            {
                                Entity * source = GetSource(connection->sourceIndex);
                                source->sourceLit = true;
                            }
                        }
                        break;
                    }
                }
            }
            
            if (!connection->conductive)
            {
                EntityLayer layers[] = { LAYER_BLOCK };
                Entity * entity = FindEntityByLocationAndLayers(connection->tilePos, layers, ArrayCount(layers));
                if (entity && entity->tweenController.NoTweens())
                {
                    if (connection->hasPower)
                    {
                        connection->conductive = true;
                        int sourceCableIndex = Source_Indices[connection->sourceIndex];
                        if (OnSourcePowerOn(sourceCableIndex))
                        {
                            Entity * source = GetSource(connection->sourceIndex);
                            source->sourceLit = true;
                        }

                    }
                }
            }

        }
        else
        {
            // Check if the circuit of the connection point are lit (i.e. power source connected to the door)
            // If not, set conductive to false if no block or slime on top
            if (!GetSource(connection->sourceIndex)->sourceLit)
            {
                EntityLayer layers[] = { LAYER_BLOCK, LAYER_SLIME };
                if (!FindEntityByLocationAndLayers(connection->tilePos, layers, ArrayCount(layers)))
                {
                    connection->conductive = false;
                        
                    int indexes[4] =
                        {
                            connection->leftIndex,
                            connection->rightIndex,
                            connection->upIndex,
                            connection->downIndex
                        };
                    for (int i = 0; i < 4; i++)
                    {
                        int id = indexes[i];
                        if (id >= 0)
                        {
                            ShutDownPower(id);                    
                        }
                    }
                }
            }
        }

    }
        
}
