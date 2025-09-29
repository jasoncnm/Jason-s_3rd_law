/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "electric_door.h"

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

    auto & connectionPointIndices = gameState->electricDoorSystem->connectionPointIndices;
    auto & entityIndices = gameState->electricDoorSystem->entityIndices;

    for (int i = 0; i < connectionPointIndices.count; i++)
    {
        Entity * connect = GetEntity(entityIndices[connectionPointIndices[i]]);
        if (connect && connect->sourceIndex == door->sourceIndex)
        {
            Entity * slime = FindSlime(connect->tilePos);

            if (slime)
            {
                slime->actionState = MOVE_STATE;
            }
        }
    }
        
}

inline void PowerOnCable(Entity * cable, bool & end)
{
    if (cable->cableType == CABLE_TYPE_DOOR)
    {
        if (!cable->open)
        {
            cable->open = true;

            UnfreezeSlimes(cable);

            IVec2 offset;
            IVec2 bounceDir;
            
            switch(cable->spriteID)
            {
                case SPRITE_DOOR_LEFT_CLOSE:
                {
                    offset = { 1, -1 };
                    bounceDir = { 1, 0 };
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    cable->sprite = GetSprite(SPRITE_DOOR_LEFT_OPEN);
                    break;
                }
                case SPRITE_DOOR_RIGHT_CLOSE:
                {
                    offset = { -1, 1 };
                    bounceDir = { -1, 0 };
                    cable->sprite = GetSprite(SPRITE_DOOR_RIGHT_OPEN);
                    cable->left = cable->right = false;
                    cable->up = cable->down = true;
                    
                    break;
                }
                case SPRITE_DOOR_TOP_CLOSE:
                {
                    offset = { -1, 1 };
                    bounceDir = { 0, 1 };
                    cable->sprite = GetSprite(SPRITE_DOOR_TOP_OPEN);
                    cable->left = cable->right = true;
                    cable->up = cable->down = false;
                    
                    break;
                }
                case SPRITE_DOOR_DOWN_CLOSE:
                {
                    offset = { 1, -1 };
                    bounceDir = { 0, -1 };
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

            Entity * entity = FindEntityByLocationAndLayer(cable->tilePos + bounceDir, LAYER_BLOCKS);
            if (entity)
            {
                BounceEntity(entity, bounceDir);
            }
            else
            {
                entity = FindEntityByLocationAndLayer(cable->tilePos + bounceDir, LAYER_PLAYER);
                if (entity)
                {
                    BounceEntity(entity, bounceDir);
                }
            }

            if (entity && (entity->tilePos == cable->tilePos + bounceDir))
            {
                cable->open = false;
                cable->sprite = GetSprite(cable->spriteID);
            }
            else
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


void ElectricDoorSystem::OnSourcePowerOn(Array<bool, MAX_CABLE> & visited, int currentIndex)
{
    bool end = false;
    
    visited[currentIndex] = true;

    Entity * cable = GetEntity(entityIndices[currentIndex]);
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


inline bool ElectricDoorSystem::DoorBlocked(IVec2 tilePos, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool result = false;
    for (int i = 0; i < doorIndices.count; i++)
    {
        Entity * door = GetEntity(entityIndices[doorIndices[i]]);
        SM_ASSERT(door, "entity is not active");

        if (SameSide(door, tilePos, reachDir)) return true;
            
    }
    return result;
}

inline bool ElectricDoorSystem::CheckDoor(IVec2 tilePos)
{
    
    bool result = false;
    for (int i = 0; i < doorIndices.count; i++)
    {
        Entity * door = GetEntity(entityIndices[doorIndices[i]]);
        SM_ASSERT(door, "entity is not active");
        if (door->tilePos == tilePos)
        {
            result = true;
            break;
        }
    }
    return result;
}

void ElectricDoorSystem::Search(Array<bool, MAX_CABLE> & visited, int currentIndex, int sourceIndex)
{
    visited[currentIndex] = true;

    Entity * current = GetEntity(entityIndices[currentIndex]);

    current->sourceIndex = sourceIndex;

    if (current->cableType == CABLE_TYPE_DOOR) return;

    for (int i = 0; i < entityIndices.count; i++)
    {
        if (!visited[i])
        {
            Entity * entity = GetEntity(entityIndices[i]);
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

inline void ElectricDoorSystem::SetUp()
{
    for (int index = 0; index < sourceIndices.count; index++)
    {
        int currentIndex = sourceIndices[index];

        Array<bool, MAX_CABLE> visited;

        for (int i = 0; i < entityIndices.count; i++) visited.Add(false);

        Search(visited, currentIndex, currentIndex);
    }
}


inline void ElectricDoorSystem::Update()
{

    for (int i = 0; i < connectionPointIndices.count; i++)
    {
        Entity * connection = GetEntity(entityIndices[connectionPointIndices[i]]);
        SM_ASSERT(connection, "Entity is not active");
        
        if (!connection->conductive)
        {
            Entity * entity = FindEntityByLocationAndLayer(connection->tilePos, LAYER_PLAYER);
            bool has = false;
            if (entity)
            {
                has = true;    
            }
            else
            {
                entity = FindEntityByLocationAndLayer(connection->tilePos, LAYER_BLOCKS);
                if (entity)
                {
                    has = true;                    
                }
            }
            if (has)
            {
                int indexes[4] = { connection->leftIndex, connection->rightIndex, connection->upIndex, connection->downIndex };
                for (int i = 0; i < 4; i++)
                {
                    int id = indexes[i];
                    if (id >= 0)
                    {
                        
                        Entity * cable = GetEntity(entityIndices[id]);
                        SM_ASSERT(cable, "Entity is not active");
                        if (cable->conductive)
                        {
                            if (entity->type == ENTITY_TYPE_PLAYER || entity->type == ENTITY_TYPE_CLONE) entity->actionState = FREEZE_STATE;
                            connection->conductive = true;

                            Array<bool, MAX_CABLE> visited;
                            for (int i = 0; i < entityIndices.count; i++) visited.Add(false);
                            OnSourcePowerOn(visited, connection->sourceIndex);

                            return;
                        } 
                    } 
                }

            }
        }
    }
    
    for (int i = 0; i < sourceIndices.count; i++)
    {
        int sourceIndex = sourceIndices[i];
        Entity * source = GetEntity(entityIndices[sourceIndex]);
        SM_ASSERT(source, "Entity is not active");

        if (source->conductive) continue;

        auto & table = gameState->entityTable[LAYER_BLOCKS];
        for (int blockIndex = 0; blockIndex < table.count; blockIndex++)
        {
            Entity * block = GetEntity(table[blockIndex]);
            
            if (block && block->type == ENTITY_TYPE_BLOCK && source->tilePos == block->tilePos)
            {
                
                Array<bool, MAX_CABLE> visited;
                for (int i = 0; i < entityIndices.count; i++) visited.Add(false);
                OnSourcePowerOn(visited, sourceIndex);
            }
        }
        
    }
}
