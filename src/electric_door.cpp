/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"
#include "move_animation.h"

inline bool CanFreezeSlime(Entity * connection)
{
    bool result = false;

    if (connection->conductive) return false;
    
    if (connection->cableType == CABLE_TYPE_CONNECTION_POINT)
    {
        int indexes[4] = { connection->leftIndex, connection->rightIndex, connection->upIndex, connection->downIndex };
        for (int i = 0; i < 4; i++)
        {
            int id = indexes[i];
            if (id >= 0)
            {
                Entity * cable = GetEntity(Cable_Indices[id]);
                SM_ASSERT(cable, "Entity is not active");
                result |= cable->conductive;
            }
        }
    }

    return result;
}
    

inline bool SameSide(Entity * door, IVec2 tilePos, IVec2 reachDir)
{
    SM_ASSERT(door->type == ENTITY_TYPE_ELECTRIC_DOOR && door->cableType == CABLE_TYPE_DOOR,
              "Entity is not a door");
    
    bool result = false;

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
    SM_ASSERT(gameState->electricDoorSystem, "electric door does not exist");

#if 0
    for (int i = 0; i < CP_Indices.count; i++)
    {
        Entity * connect = GetEntity(Cable_Indices[CP_Indices[i]]);
        if (connect && connect->sourceIndex == door->sourceIndex)
        {
            Entity * slime = FindSlime(connect->tilePos);

            if (slime)
            {
                slime->actionState = MOVE_STATE;
            }
        }
    }
#else

    for (int i = 0; i < gameState->slimeEntityIndices.count; i++)
    {
        Entity * slime = GetEntity(gameState->slimeEntityIndices[i]);
        if (slime)
        {
            SetActionState(slime, MOVE_STATE);
        }
    }

#endif
}

inline void PowerOnCable(Entity * cable, bool & end)
{
    if (cable->cableType == CABLE_TYPE_DOOR)
    {
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

                default:
                {
                    SM_ASSERT(false, "door has no sprite");
                }
                
            }

            Entity * entity = FindEntityByLocationAndLayer(cable->tilePos + bounceDir, LAYER_BLOCK);
            if (entity)
            {
                Vector2 moveStart = GetTilePivot(entity);
                BounceEntity(cable, entity, bounceDir);
                Vector2 moveEnd = GetTilePivot(entity);
                
                float dist = Vector2Distance(moveStart, moveEnd);
                float iDist = dist / MAP_TILE_SIZE;

                AddMoveAnimateQueue(entity->moveAniQueue,
                                    GetMoveAnimation(nullptr, moveStart, moveEnd, 20.0f, iDist, true, 0));
                
            }
            else
            {
                entity = FindEntityByLocationAndLayer(cable->tilePos + bounceDir, LAYER_SLIME);
                if (entity)
                {
                    BounceEntity(cable, entity, bounceDir);
                }
            }

            // TODO
#if 0
            if (entity && (entity->tilePos == cable->tilePos + bounceDir))
            {
                cable->open = false;
                cable->sprite = GetSprite(cable->spriteID);
            }
            else
#endif
            {
                cable->tilePos = cable->tilePos + offset;
            }
        }
        end = true;
    }
    else if (cable->cableType == CABLE_TYPE_CONNECTION_POINT)
    {
        if (!cable->conductive)
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
        cable->sprite = GetSprite(GetCablePowerONID(cable->spriteID));
        end = false;
    }
    
}


void OnSourcePowerOn(Array<bool, MAX_CABLE> & visited, int currentIndex)
{
    bool end = false;
    
    visited[currentIndex] = true;

    Entity * cable = GetEntity(Cable_Indices[currentIndex]);
    SM_ASSERT(cable, "entity is not active");
    
    PowerOnCable(cable, end);
    
    if (end) return;
    
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
        if (id >= 0 && !visited[id])
        {
            OnSourcePowerOn(visited, id);
        }
    }
    return;
}


inline bool DoorBlocked(Entity * door, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool result = false;

    if (reachDir.x == 1)
    {
        result = door->spriteID == SPRITE_DOOR_RIGHT_CLOSE;
    }
    else if (reachDir.x == -1)
    {
        result = door->spriteID == SPRITE_DOOR_LEFT_CLOSE;
    }
    else if (reachDir.y == 1)
    {
        result = door->spriteID == SPRITE_DOOR_DOWN_CLOSE;
        
    }
    else if (reachDir.y == -1)
    {
        result = door->spriteID == SPRITE_DOOR_TOP_CLOSE;
    }
    
    return result;
}

inline bool CheckDoor(IVec2 tilePos)
{
    
    bool result = false;
    for (int i = 0; i < Door_Indices.count; i++)
    {
        Entity * door = GetEntity(Cable_Indices[Door_Indices[i]]);
        SM_ASSERT(door, "entity is not active");
        if (door->tilePos == tilePos)
        {
            result = true;
            break;
        }
    }
    return result;
}

void Search(Array<bool, MAX_CABLE> & visited, int currentIndex, int sourceIndex)
{
    visited[currentIndex] = true;

    Entity * current = GetEntity(Cable_Indices[currentIndex]);

    current->sourceIndex = sourceIndex;

    if (current->cableType == CABLE_TYPE_DOOR) return;

    for (int i = 0; i < Cable_Indices.count; i++)
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
                entity->leftIndex = currentIndex;
            }
            // NOTE A in right B in left
            // NOTE They are connected only if A has left and B has right connection
            else if (offset.x == -1 && (current->left && entity->right))
            {
                current->leftIndex = i;
                entity->rightIndex = currentIndex;
            }
            // NOTE A in Up B in Down
            // NOTE They are connected only if A has Down and B has Up connection
            else if (offset.y == 1 && (current->down && entity->up))
            {
                current->downIndex = i;
                entity->upIndex = currentIndex;
            }
            // NOTE A in Down B in Up
            // NOTE They are connected only if A has up and B has down connection         
            else if (offset.y == -1 &&  (current->up && entity->down))
            {
                current->upIndex = i;
                entity->downIndex = currentIndex;
            }
            else
            {
                continue;
            }
            Search(visited, i, sourceIndex);
        }
    }
    
}

inline void SetUpElectricDoor()
{
    for (int index = 0; index < Source_Indices.count; index++)
    {
        int currentIndex = Source_Indices[index];

        Array<bool, MAX_CABLE> visited;

        for (int i = 0; i < Cable_Indices.count; i++) visited.Add(false);

        Search(visited, currentIndex, currentIndex);
    }
}


inline void UpdateElectricDoor()
{

    for (int i = 0; i < CP_Indices.count; i++)
    {
        Entity * connection = GetEntity(Cable_Indices[CP_Indices[i]]);
        SM_ASSERT(connection, "Entity is not active");
        
        if (!connection->conductive)
        {
            Entity * entity = FindEntityByLocationAndLayer(connection->tilePos, LAYER_SLIME);
            bool has = false;
            if (entity)
            {
                has = true;    
            }
            else
            {
                entity = FindEntityByLocationAndLayer(connection->tilePos, LAYER_BLOCK);
                if (entity)
                {
                    has = true;                    
                }
            }
            if (has)
            {
                if (CanFreezeSlime(connection))
                {
                    if (IsSlime(entity)) entity->actionState = FREEZE_STATE;
                    connection->conductive = true;
                    Array<bool, MAX_CABLE> visited;
                    for (int i = 0; i < Cable_Indices.count; i++) visited.Add(false);
                    OnSourcePowerOn(visited, connection->sourceIndex);
                }
            }
        }
    }
    
    for (int i = 0; i < Source_Indices.count; i++)
    {
        int sourceIndex = Source_Indices[i];
        Entity * source = GetEntity(Cable_Indices[sourceIndex]);
        SM_ASSERT(source, "Entity is not active");
        if (source->conductive) continue;
        auto & table = gameState->entityTable[LAYER_BLOCK];
        for (int blockIndex = 0; blockIndex < table.count; blockIndex++)
        {
            Entity * block = GetEntity(table[blockIndex]);
            if (block && block->type == ENTITY_TYPE_BLOCK && source->tilePos == block->tilePos)
            {
                
                Array<bool, MAX_CABLE> visited;
                for (int i = 0; i < Cable_Indices.count; i++) visited.Add(false);
                OnSourcePowerOn(visited, sourceIndex);
            }
        }
        
    }
    
}
