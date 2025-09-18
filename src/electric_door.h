#if !defined(ELECTRIC_DOOR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */



enum CableType
{
    CABLE_TYPE_SOURCE,
    CABLE_TYPE_CONNECT,
    CABLE_TYPE_CONNECTION_POINT,
    CABLE_TYPE_DOOR,
};


struct Cable
{
    bool open = false;
    bool conductive = false;
    bool left = false, right = false, up = false, down = false;

    Sprite sprite;
    SpriteID id;

    CableType type;

    int sourceIndex = -1;
    int leftIndex = -1;
    int rightIndex = -1;
    int upIndex = -1;
    int downIndex = -1;

    IVec2 tile;

};


bool IsConnected(Cable & a, Cable & b, IVec2 & offset)
{
    offset = b.tile - a.tile;

    if (a.tile == b.tile || offset.SqrMagnitude() > 1) return false;

    if (offset.x == 1)
    {
        // NOTE A in left B in right
        // NOTE They are connected only if A has right and B has left connection
        return a.right && b.left;
    }
    else if (offset.x == -1)
    {
        // NOTE A in right B in left
        // NOTE They are connected only if A has left and B has right connection
        return a.left && b.right;
    }
    else if (offset.y == 1)
    {
        // NOTE A in Up B in Down
        // NOTE They are connected only if A has Down and B has Up connection
        return a.down && b.up;        
    }
    else if (offset.y == -1)
    {
        // NOTE A in Down B in Up
        // NOTE They are connected only if A has up and B has down connection         
        return a.up && b.down;
    }

    SM_ASSERT(false, "What is going on??");
    return false;
    
}

struct ElectricDoorSystem
{
    Array<int, MAX_CABLE> sourceIndexes;
    Array<int, MAX_CABLE> doorIndexes;

    Array<Cable, MAX_CABLE> cables;

    void SetUp();
    
    void Search(Array<bool, MAX_CABLE> & visited, int currentIndex, int sourceIndex);

    bool DoorBlocked(IVec2 tilepos, IVec2 reachDir);
    bool CheckDoor(IVec2 tilePos);
};

bool ElectricDoorSystem::DoorBlocked(IVec2 tilePos, IVec2 reachDir)
{
    SM_ASSERT(reachDir.SqrMagnitude() <= 1, "Directional Vector should be a unit vector");
    
    bool result = false;
    for (int i = 0; i < doorIndexes.count; i++)
    {
        Cable & door = cables[doorIndexes[i]];
        if (door.tile == tilePos)
        {
            if (Abs(reachDir.x) > 0)
            {
                result = door.left || door.right;
            }
            else if (Abs(reachDir.y) > 0)
            {
                result = door.up || door.down;
            }
                     
            break;
        }
    }
    return result;
}

bool ElectricDoorSystem::CheckDoor(IVec2 tilePos)
{
    
    bool result = false;
    for (int i = 0; i < doorIndexes.count; i++)
    {
        Cable & door = cables[doorIndexes[i]];
        if (door.tile == tilePos)
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

    Cable & current = cables[currentIndex];

    current.sourceIndex = sourceIndex;

    if (current.type == CABLE_TYPE_DOOR) return;

    for (int i = 0; i < cables.count; i++)
    {
        IVec2 offset;
        if (!visited[i] && IsConnected(current, cables[i], offset))
        {
            if (offset.x == 1)
            {
                current.rightIndex = i;
                cables[i].leftIndex = currentIndex;
            }
            else if (offset.x == -1)
            {
                current.leftIndex = i;
                cables[i].rightIndex = currentIndex;
            }
            else if (offset.y == 1)
            {
                current.downIndex = i;
                cables[i].upIndex = currentIndex;
            }
            else if (offset.y == -1)
            {
                current.upIndex = i;
                cables[i].downIndex = currentIndex;
            }
            else
            {
                SM_ASSERT(false, "The offset is impossible at this point wtf?");
            }
            Search(visited, i, sourceIndex);
        }
    }
    
}

void ElectricDoorSystem::SetUp()
{

    if (cables.IsEmpty()) return;

    for (int index = 0; index < sourceIndexes.count; index++)
    {
        int currentIndex = sourceIndexes[index];

        Array<bool, MAX_CABLE> visited;
        for (int i = 0; i < cables.count; i++) visited.Add(false);

        Search(visited, currentIndex, currentIndex);        
        
    }
}

#define ELECTRIC_DOOR_H
#endif
