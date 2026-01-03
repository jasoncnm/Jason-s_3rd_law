/* date = January 2nd 2026 6:12 pm */

#ifndef MEMORY_H
#define MEMORY_H

#include "log.h"

#define b8 char
#define BIT(x) 1 << (x)
#define KB(x) (1024LL * x)
#define MB(x) (1024LL * KB(x))
#define GB(x) (1024LL * MB(x))
#define TB(x) (1024LL * GB(x))

//  ========================================================================
// NOTE: Bump Allocator
//  ========================================================================
struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char *memory;
    
};


#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

//  ========================================================================
// NOTE: Bump Allocator Functions
//  ========================================================================
#define BumpAllocArray(ba, count, size) BumpAlloc(ba, (count)*size)
char * BumpAlloc(BumpAllocator * ba, size_t size)
{
    char * result = nullptr;
    
    size_t allignedSize = (size + 7) & ~ 7; // NOTE: This make sure the first 4 bits are 0
    if (ba->used + allignedSize <= ba->capacity)
    {
        result = ba->memory + ba->used;
        ba->used += allignedSize;
    }
    else
    {
        SM_ASSERT(false, "Bump Allocator is full");
    }
    
    return result;
}


#endif //MEMORY_H
