/* date = January 2nd 2026 6:12 pm */

#ifndef MEMORY_H
#define MEMORY_H


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


#endif //MEMORY_H
