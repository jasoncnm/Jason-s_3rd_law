

#include "memory.h"

#if GAME_INTERNAL
#include <memoryapi.h>
BumpAllocator MakeBumpAllocator(void * baseAddress, size_t size)
{
    BumpAllocator ba = {};
    
    ba.memory = (char *)VirtualAlloc((LPVOID)baseAddress, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); 
                        
    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size); // NOTE: Set the memory to zero
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate memory!, BaseAddress: %p", ba.memory);
    }
    
    return ba;
}
#endif 

BumpAllocator MakeBumpAllocator(size_t size)
{
    BumpAllocator ba = {};
    
    ba.memory = (char *)MemAlloc((uint32)size);
    
    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size); // NOTE: Set the memory to zero
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate memory!, BaseAddress: %p", ba.memory);
    }
    return ba;
    }
