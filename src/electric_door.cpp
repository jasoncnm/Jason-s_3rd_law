/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"

 IVec2 SetDoorOpen(Entity * door)
{
    SM_ASSERT(!door->open, "door is opened");
    IVec2 bounceDir = { 0 };
    
    door->open = true;
    
    IVec2 openDir = door->openDir;
    SM_ASSERT(openDir.SqrMagnitude() > 1, "invalid open dir");
    SM_ASSERT(door->left && door->right || door->up && door->down, "invalid connect direction");
    
    door->sprite = GetOpenDoorSprite(openDir, door->left && door->right);
    
    if (door->left)
    {
        bounceDir = { openDir.x, 0 };
    }
    else
    {
        bounceDir = { 0, openDir.y };
    }
    
    bool8 newLR = door->up;
    bool8 newUD = door->left;
    door->left = door->right = newLR;
    door->up = door->down = newUD;
    
    door->tilePos = door->tilePos + openDir;
    door->pivot = GetTilePivot(door);
    
    return bounceDir;
}

void SetDoorClose(Entity * door)
{
    door->open = false;
    door->sprite = GetOpenDoorSprite(-door->openDir, door->left && door->right);
    
    bool8 newLR = door->up;
    bool8 newUD = door->left;
    door->left = door->right = newLR;
    door->up = door->down = newUD;
    
    door->tilePos = door->tilePos - door->openDir;
    door->pivot = GetTilePivot(door);
    }

void SetFreeze()
{
    for (uint32 i = 0; i < Connection_Indices.count; i++)
    {
        Entity * connection = GetEntity(Connection_Indices[i]);
        SM_ASSERT(connection, "Entity is not active");
        
        if (connection->hasPower && !GetEntity(connection->sourceIndex)->sourceLit)
        {
            auto & slimeIndexTable = gameState->entityTable[LAYER_SLIME];
            for (uint32 slimeIndex = 0; slimeIndex < slimeIndexTable.count; slimeIndex++)
            {
                Entity * slime = GetEntity(slimeIndexTable[slimeIndex]);
                if (slime && slime->actionState != FREEZE_STATE &&
                    (PivotToTilePos(slime->pivot, slime->tileSize) == connection->tilePos))
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
                        continue;
                    }
                    
                    SetEntityPosition(slime, nullptr, freezePos);
                        endPivot = GetTilePivot(freezePos, GetSlimeSize(slime));
                    
                    if ((startPivot != endPivot))
                    {
                    slime->tweenController.Reset();
                    
                    TweenParams params = {};
                    params.paramType = PARAM_TYPE_VECTOR2;
                    params.startVec2 = startPivot;
                    params.endVec2   = endPivot;
                    params.realVec2  = &slime->pivot;
                    
                    AddTween(slime->tweenController, CreateTween(params));
                    OnPlayEvent(&slime->tweenController);
                    }
                }
            }
        }
    }
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
            IVec2 oldPos = cable->tilePos;
            UnfreezeSlimes(cable);
            
            IVec2 bounceDir = SetDoorOpen(cable);
            
            EntityLayer layers[] = { LAYER_BLOCK, LAYER_SLIME };
            Entity * entity = FindEntityByLocationAndLayers(oldPos + bounceDir, layers, ArrayCount(layers));
            if (entity)
            {
                Vector2 moveStart = GetTilePivot(entity);
                ActionCheck(entity, bounceDir, CHECK_PROJECT);
                Vector2 moveEnd = GetTilePivot(entity);
                
                if ((moveStart ==  moveEnd))
                {
                    ShiftEntities(entity->tilePos, bounceDir);                    
                }
            }
            }
        
        
        SetShake(0.05f, 0.001f);
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
        // SetEntitySprite(cable, GetCablePowerOnID(cable->tileID));
        cable->color = WHITE;
        end = false;
    }

    return doorOpened;
}

bool8 OnSourcePowerOn(int32 sourceIndex)
{

    Array<int32, CABLE_MAX_CALL_STACK> callStack  = {};
    callStack.Add(sourceIndex);
    bool8 doorOpened = false;

    while (!callStack.IsEmpty())
    {
        int32 entityIndex = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(entityIndex);
        SM_ASSERT(cable && cable->type == ENTITY_TYPE_ELECTRIC_DOOR, "currentIndex is not an electic door entity");
        
        bool8 end = false;
        bool _open = PowerOnCable(cable, end);
        doorOpened = _open || doorOpened;
        if (!end)
        {
            // return doorOpened;
            int32 indexes[4] =
                {
                    cable->leftIndex,
                    cable->rightIndex,
                    cable->upIndex,
                    cable->downIndex
                };
    
            for (int32 i = 0; i < 4; i++)
            {
                int32 id = indexes[i];
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

void ShutDownPower(int32 sourceIndex)
{
    Array<int32, CABLE_MAX_CALL_STACK> callStack = {};
    callStack.Add(sourceIndex);
    
    while(!callStack.IsEmpty())
    {
        int32 currentIndex = callStack.last();
        callStack.RemoveLast();
        
        Entity * cable = GetEntity(currentIndex);
        SM_ASSERT(cable && cable->type == ENTITY_TYPE_ELECTRIC_DOOR, "currentIndex is not an electic door entity");
        
        cable->conductive = false;
        cable->sourceLit = false;
        
        if (cable->cableType == CABLE_TYPE_CONNECTION_POINT)
        {
            cable->hasPower = false;
        }
        else if (cable->cableType == CABLE_TYPE_DOOR)
        {
            if (cable->open) SetDoorClose(cable);
        }
        else
        {
            cable->color = GRAY;// SetEntitySprite(cable, GetCablePowerOffID(cable->tileID));
            }
    
        int32 indexes[4] =
            {
                cable->leftIndex,
                cable->rightIndex,
                cable->upIndex,
                cable->downIndex
            };
    
        for (int32 i = 0; i < 4; i++)
        {
            int32 id = indexes[i];
            if (id >= 0)
            {
                callStack.Add(id);
            }
        }
    }
}

inline IVec2 GetDoorDirection(Entity * door)
{
    IVec2 dir = { 0 };
    
    if (door->left && door->right)
    {
        dir = door->open ? IVec2 { door->openDir.x, 0 } : IVec2 { -door->openDir.x, 0 };
    }
    else if (door->up && door->down)
    {
        dir = door->open ? IVec2 { 0, door->openDir.y } : IVec2 { 0, - door->openDir.y };
    }
    
    return dir;
}

inline bool8 DoorBlocked(Entity * door, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool8 result = (reachDir == GetDoorDirection(door));
    
    return result;
}

inline bool8 IsDoor(Entity * door)
{
    return door && door->active && door->type == ENTITY_TYPE_ELECTRIC_DOOR && door->cableType == CABLE_TYPE_DOOR;
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
    auto Visited = [](Entity * ent) 
    {
        bool8 result = ent && (ent->leftIndex >= 0 || ent->rightIndex >= 0 || ent->upIndex >= 0 || ent->downIndex >= 0);
        return result;
    };
    
     auto IsCable = [](Entity * ent)
    {
        return ent && ent->type == ENTITY_TYPE_ELECTRIC_DOOR;
    };
    
    EntityLayer findLayers[] = { 
        LAYER_SOURCE,
        LAYER_DOOR,
        LAYER_CABLE,
        LAYER_CONNECTION,
    };
    
    uint32 layerCount = ArrayCount(findLayers);
    IVec2 dirs[] = { IVec2{ 0, -1 }, IVec2{ 0, 1 }, IVec2 { -1, 0 }, IVec2{ 1, 0 }, };
    
    for (uint32 ind = 0; ind < Source_Indices.count; ind++)
    {
        Array<int32, CABLE_MAX_CALL_STACK> callStack = {};

        int32 sourceIndex = Source_Indices[ind];
            callStack.Add( sourceIndex );
        
            
        while(!callStack.IsEmpty())
        {
            int32 currentIndex = callStack.last();
            callStack.RemoveLast();
            
            Entity * current = GetEntity(currentIndex);
            current->sourceIndex = sourceIndex;
            
            if (current->cableType == CABLE_TYPE_DOOR) continue;
            
            bool connected = false;
            
            // NOTE: up
            if (current->up) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 0, -1 }, findLayers, layerCount);
                if (cable && IsCable(cable) && !Visited(cable) && cable->down)
                {
                    current->upIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                    connected = true;
                }
            }
            // NOTE: down
            if (current->down) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 0, 1 }, findLayers, layerCount);
                if (cable && IsCable(cable) && !Visited(cable) && cable->up)
                {
                    current->downIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                    connected = true;
                }
                
            }
            // NOTE: left
            if (current->left) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ -1, 0 }, findLayers, layerCount);
                if (cable && IsCable(cable) && !Visited(cable) && cable->right)
                {
                    current->leftIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                    connected = true;
                }
                
            }
            // NOTE: right
            if (current->right) 
            {
                Entity * cable = FindEntityByLocationAndLayers(current->tilePos + IVec2{ 1, 0 }, findLayers, layerCount);
                if (cable && IsCable(cable) && !Visited(cable) && cable->left)
                {
                    current->rightIndex = cable->entityIndex;
                    callStack.Add(cable->entityIndex);
                    connected = true;
                }
            }
            
            SM_ASSERT(connected, "cable are not connected");
            
            }
    }
    
    for (uint32 i = 0; i < Source_Indices.count; i++)
    {
        Entity * source = GetEntity(Source_Indices[i]);
        SM_ASSERT(source, "Entity is not active");
        bool8 has = false;
        int32 sourceCableIndex = Source_Indices[i];
        
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


inline bool8 UpdateElectricDoor()
{
    bool8 changed = false;
    for (uint32 i = 0; i < Source_Indices.count; i++)
    {
        int32 sourceCableIndex = Source_Indices[i];
        Entity * source = GetEntity(sourceCableIndex);
        SM_ASSERT(source, "Entity is not active");
        bool8 has = false;

        EntityLayer layers[] = { LAYER_BLOCK };
        Entity * block = FindEntityByLocationAndLayers(source->tilePos, layers, ArrayCount(layers));
        if (block && block->tweenController.NoTweens())
        {
            block->actionState = FREEZE_STATE;
            if (!source->conductive && OnSourcePowerOn(sourceCableIndex))
            {
                changed = true;
                source->sourceLit = true;
            }
                
        }
    }
    
for (uint32 i = 0; i < Connection_Indices.count; i++)
    {
        Entity * connection = GetEntity(Connection_Indices[i]);
        SM_ASSERT(connection, "Entity is not active");
        
        EntityLayer layers[] = { LAYER_BLOCK, LAYER_SLIME };
        Entity * entity = FindEntityByLocationAndLayers(connection->tilePos, layers, ArrayCount(layers));
        
        if (!connection->conductive)
            {
                if (entity && entity->tweenController.NoTweens())
                {
                    if (connection->hasPower)
                    {
                        connection->conductive = true;
                        if (OnSourcePowerOn(connection->sourceIndex))
                        {
                            Entity * source = GetEntity(connection->sourceIndex);
                        connection->sourceLit = true;
                        changed = true;
                        }

                    }
            }
        }
    }
    return changed;
}
