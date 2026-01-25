#if !defined(ELECTRIC_DOOR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define Cable_Indices gameState->electricDoorSystem.entityIndices
#define Source_Indices gameState->electricDoorSystem.sourceIndices
#define CP_Indices gameState->electricDoorSystem.connectionPointIndices
#define Door_Indices gameState->electricDoorSystem.doorIndices

#define CABLE_MAX_CALL_STACK 50

struct Entity;

enum CableType
{
    CABLE_TYPE_NULL,
    
    CABLE_TYPE_SOURCE,
    CABLE_TYPE_CONNECT,
    CABLE_TYPE_CONNECTION_POINT,
    CABLE_TYPE_DOOR,

    CABLE_TYPE_COUNT,
};


struct ElectricDoorSystem
{
    Array<int, 50> sourceIndices;
    Array<int, 50> doorIndices;
    Array<int, 50> connectionPointIndices;
    Array<int, 800> entityIndices;

    void CleanUp()
    {
        sourceIndices.Clear();
        doorIndices.Clear();
        connectionPointIndices.Clear();
        entityIndices.Clear();
    }
    
};

#define ELECTRIC_DOOR_H
#endif
