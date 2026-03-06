#if !defined(ELECTRIC_DOOR_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define Cable_Indices gameState->entityTable[LAYER_CABLE]
#define Source_Indices gameState->entityTable[LAYER_SOURCE]
#define Connection_Indices gameState->entityTable[LAYER_CONNECTION]
#define Door_Indices gameState->entityTable[LAYER_DOOR]

#define CABLE_MAX_CALL_STACK 50

enum CableType
{
    CABLE_TYPE_NULL,
    
    CABLE_TYPE_SOURCE,
    CABLE_TYPE_CONNECT,
    CABLE_TYPE_CONNECTION_POINT,
    CABLE_TYPE_DOOR,

    CABLE_TYPE_COUNT,
};


#define ELECTRIC_DOOR_H
#endif
