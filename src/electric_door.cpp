/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "tween_controller.h"

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

    for (uint32 i = 0; i < Connection_Indices.count; i++)
    {
        Entity * connect = GetEntity(Connection_Indices[i]);
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

            EntityLayer layers[] = { LAYER_BLOCK, LAYER_SLIME };
            Entity * entity = FindEntityByLocationAndLayers(cable->tilePos + bounceDir, layers, ArrayCount(layers));
            if (entity)
            {
                Vector2 moveStart = GetTilePivot(entity);
                ActionCheck(entity, bounceDir, CHECK_PROJECT);
                Vector2 moveEnd = GetTilePivot(entity);
                
                if (Vector2Equals(moveStart, moveEnd))
                {
                    ShiftEntities(entity->tilePos, bounceDir);                    
                }
            }
            
            cable->tilePos = cable->tilePos + offset;
            cable->pivot = GetTilePivot(cable);
        }
        
        GetEntity(cable->sourceIndex)->sourceLit = true;
        end = true;
    }
    else if (cable->cableType == CABLE_TYPE_CONNECTION_POINT)
    {
        cable->hasPower = true;
            EntityLayer layers[] = { LAYER_SLIME, LAYER_BLOCK };
        Entity * has = FindEntityByLocationAndLayers(cable->tilePos, layers, ArrayCount(layers));
        if (has)
        {
            has->actionState = FREEZE_STATE;
            cable->conductive = true;
            end = false;
        }
        else if (!cable->conductive)
        {
end = true;
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

bool8 OnSourcePowerOn(int sourceIndex)
{

    Array<int, CABLE_MAX_CALL_STACK> callStack  = {};
    callStack.Add(sourceIndex);
    bool8 doorOpened = false;

    while (!callStack.IsEmpty())
    {
        int entityIndex = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(entityIndex);
        SM_ASSERT(cable && cable->type == ENTITY_TYPE_ELECTRIC_DOOR, "currentIndex is not an electic door entity");
        
        cable->changed = true;
        
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

void ShutDownPower(int sourceIndex)
{
    Array<int, CABLE_MAX_CALL_STACK> callStack = {};
    callStack.Add(sourceIndex);

    while(!callStack.IsEmpty())
    {
        int currentIndex = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(currentIndex);
        SM_ASSERT(cable && cable->type == ENTITY_TYPE_ELECTRIC_DOOR, "currentIndex is not an electic door entity");
        
        cable->changed = true;

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

inline bool8 IsDoor(Entity * door)
{
    return door->active && door->type == ENTITY_TYPE_ELECTRIC_DOOR && door->cableType == CABLE_TYPE_DOOR;
}

inline bool8 CheckDoor(IVec2 tilePos)
{
    bool8 result = false;
    for (uint32 i = 0; i < Door_Indices.count; i++)
    {
        Entity * door = GetEntity(Door_Indices[i]);
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
    auto Visited = [](Entity * ent) {
        bool8 result = ent && (ent->leftIndex > 0 || ent->rightIndex > 0 || ent->upIndex > 0 || ent->downIndex > 0);
        return result;
    };
    
    EntityLayer findLayers[] = { 
        LAYER_DOOR,
        LAYER_CABLE,
        LAYER_CONNECTION,
    };
    
    uint32 layerCount = ArrayCount(findLayers);
    IVec2 dirs[] = { IVec2{ 0, -1 }, IVec2{ 0, 1 }, IVec2 { -1, 0 }, IVec2{ 1, 0 }, };
    
    for (uint32 ind = 0; ind < Source_Indices.count; ind++)
    {
        Array<int, CABLE_MAX_CALL_STACK> callStack = {};

        int sourceIndex = Source_Indices[ind];
            callStack.Add( sourceIndex );
        
            
        while(!callStack.IsEmpty())
        {
            int currentIndex = callStack.last();
            callStack.RemoveLast();
            
            Entity * current = GetEntity(currentIndex);
            current->sourceIndex = sourceIndex;
            
            if (current->cableType == CABLE_TYPE_DOOR) continue;
            
            // NOTE: up
            if (current->up) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 0, -1 }, findLayers, layerCount);
                if (cable && !Visited(cable) && cable->down)
                {
                    current->upIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                }
            }
            // NOTE: down
            if (current->down) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 0, 1 }, findLayers, layerCount);
                if (cable && !Visited(cable) && cable->up)
                {
                    current->downIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                }
                
            }
            // NOTE: left
            if (current->left) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ -1, 0 }, findLayers, layerCount);
                if (cable && !Visited(cable) && cable->right)
                {
                    current->leftIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                }
                
            }
            // NOTE: right
            if (current->right) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 1, 0 }, findLayers, layerCount);
                if (cable && !Visited(cable) && cable->left)
                {
                    current->rightIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                }
                }
            }
    }
    
    for (uint32 i = 0; i < Source_Indices.count; i++)
    {
        Entity * source = GetEntity(Source_Indices[i]);
        SM_ASSERT(source, "Entity is not active");
        bool8 has = false;
        int sourceCableIndex = Source_Indices[i];
        
        EntityLayer layers[] = { LAYER_BLOCK };
        Entity * block = FindEntityByLocationAndLayers(source->tilePos, layers, ArrayCount(layers));
        if (block && block->tweenController.NoTweens())
        {
            block->actionState = FREEZE_STATE;
            OnSourcePowerOn(sourceCableIndex);
        }
        else
        {
            ShutDownPower(sourceCableIndex);
        }
        }
    
}


inline void UpdateElectricDoor()
{
    
    for (uint32 i = 0; i < Source_Indices.count; i++)
    {
        int sourceCableIndex = Source_Indices[i];
        Entity * source = GetEntity(sourceCableIndex);
        SM_ASSERT(source, "Entity is not active");
        if (source->sourceLit) continue;

        bool8 has = false;

        EntityLayer layers[] = { LAYER_BLOCK };
        Entity * block = FindEntityByLocationAndLayers(source->tilePos, layers, ArrayCount(layers));
        if (block && block->tweenController.NoTweens())
        {
            block->actionState = FREEZE_STATE;
            if (!source->conductive)
                OnSourcePowerOn(sourceCableIndex);
        }
        else
        {
            ShutDownPower(sourceCableIndex);
        }
        
    }

    for (uint32 i = 0; i < Connection_Indices.count; i++)
    {
        Entity * connection = GetEntity(Connection_Indices[i]);
        SM_ASSERT(connection, "Entity is not active");
        
        if (!connection->conductive)
        {
            auto & slimeIndexTable = gameState->entityTable[LAYER_SLIME];
            for (uint32 slimeIndex = 0; slimeIndex < slimeIndexTable.count; slimeIndex++)
            {
                Entity * slime = GetEntity(slimeIndexTable[slimeIndex]);
                if (slime)
                {
                    if (PivotToTilePos(slime->pivot, slime->tileSize) == connection->tilePos)
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
                                
                                slime->tweenController.Reset();
                                
                                if (result.has)
                                {
                                    MoveEntity(slime, result.entity, nullptr, freezePos, BLOCK_MOVE_FUNC);
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
                                Entity * source = GetEntity(connection->sourceIndex);
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
                            Entity * source = GetEntity(connection->sourceIndex);
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
            if (!GetEntity(connection->sourceIndex)->sourceLit)
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
