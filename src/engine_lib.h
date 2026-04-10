#if !defined(ENGINE_LIB_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sys/stat.h>
#include <vector>
#include <thread>
#include <string>
#include <unordered_map>

//  ========================================================================
// NOTE: Defines
//  ========================================================================

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int8 bool8;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

#include "log.h"
#include "memory.h"
#include "file.h"


//  ========================================================================
// NOTE: Dynamic Array
//  ========================================================================

// IMPORTANT NOTE: This data structure does not allocate memory upfront
// update count and  allocate memory Manually!!!

template<typename T>
struct DynamicArray
{
    uint32 count;
    T * elements;
    };

//  ========================================================================
// NOTE: Array
//  ========================================================================
template<typename T, int N>
struct Array
{
    static constexpr int maxElements = N;
    uint32 count = 0;
    T elements[N];

    T & operator[](uint32 idx)
    {
        SM_ASSERT(idx >= 0, "Idx negative!");
        SM_ASSERT(idx < count, "Idx out of bounds!");
        return elements[idx];
    }
    
    T & first()
    {
        SM_ASSERT(!IsEmpty(), "Array is empty!");
        return elements[0];
    }
    
    T & last()
    {
        SM_ASSERT(!IsEmpty(), "Array is empty!");
        return elements[count - 1];
    }
    
    void RemoveLast()
    {
        if (!IsEmpty())
        {
            count--;
        }
    }

    uint32 Add(T element)
    {
        SM_ASSERT(count < maxElements, "Array Full!");
        elements[count] = element;
        return count++;
    }

    std::vector<T> GetVectorSTD()
    {
        std::vector<T> result;
        result.reserve(count);
        for (uint32 i = 0; i < count; i++)
        {
            result.push_back(elements[i]);
        }
        
        return result;
    }

    void RemoveIdxAndSwap(uint32 idx)
    {
        SM_ASSERT(idx >= 0, "Idx negative!");
        SM_ASSERT(idx < count, "Idx out of bounds!");
        elements[idx] = elements[--count];
    }
    
    void RemoveFront()
    {
        for (uint32 i = 1; i < count; i++)
        {
            elements[i - 1] = elements[i];
        }
        count--;
    }

    void ReverseElements()
    {
        if (count > 1)
        {
            for (uint32 i = 0; i < count / 2; i++)
            {
                T temp = elements[i];
                elements[i] = elements[count - 1 - i];
                elements[count - 1 - i] = temp;
            }
        }
    }

    void Clear()
    {
        count = 0;
    }

    bool8 IsFull()
    {
        return count == N;
    }

    bool8 IsEmpty()
    {
        return count == 0;
    }

};

int StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        Count++;
    }
    return Count;
}

void CatStrings(char *SourceA, size_t SourceACount,
                char *SourceB, size_t SourceBCount,
                char *Dest,    size_t DestCount)
{
    SM_ASSERT(DestCount > (SourceACount + SourceBCount), "Insufficient Dest string length"); 
    for (int Index = 0; Index < SourceACount; Index++)
    {
        *Dest++ = *SourceA++;
    }
    for (int Index = 0; Index < SourceBCount; Index++)
    {
        *Dest++ = *SourceB++;
    }
    *Dest++ = 0; 
}

std::string FindFileNameFromPath(std::string path)
{
    // Source - https://stackoverflow.com/a
    // Posted by Pixelchemist
    // Retrieved 2026-01-23, License - CC BY-SA 3.0
    
    return path.substr(path.find_last_of("/\\") + 1);
        
}

//  ========================================================================
//              NOTE: Math Stuff
//  ========================================================================
struct Vec2
{
    float x;
    float y;

    Vec2 operator*(float scalar)
    {
        return { x * scalar, y * scalar };
    }

    Vec2 operator-(Vec2 other)
    {
        return { x - other.x, y - other.y };
    }
};

float Distance(Vec2 a, Vec2 b)
{
    Vec2 offset = a - b;
    float result = sqrtf(offset.x * offset.x + offset.y * offset.y);
    return result;
}

struct IVec2;
Vec2 IVec2ToVec2(IVec2 val);
float Abs(float x)
{
    return x > 0 ? x : -x;
}

struct IVec2
{
    int x;
    int y;

    IVec2 operator-(IVec2 other)
    {
        return { x - other.x, y - other.y };
    }

    IVec2 operator+(IVec2 other)
    {
        return { x + other.x, y + other.y };
    }

    IVec2 operator-()
    {
        return { -x, -y };
    }
    
    IVec2 operator*(int scale)
    {
         return { x * scale, y * scale };
    }

    bool8 operator==(IVec2 other)
    {
        return (x == other.x) && (y == other.y); 
    }

    bool8 operator!=(IVec2 other)
    {
        return !(*this == other); 
    }
    
    

    void operator += (IVec2 other)
    {
        x += other.x;
        y += other.y;
    }
    
    int SqrMagnitude()
    {
        return x * x + y * y;
    }
    
    bool8 IsBetween(IVec2 a, IVec2 b)
    {
        float lenAB = Distance(IVec2ToVec2(a), IVec2ToVec2(b));
        float lenA = Distance(IVec2ToVec2(a), IVec2ToVec2(*this));
        float lenB = Distance(IVec2ToVec2(b), IVec2ToVec2(*this));

        bool8 result = lenAB - (lenA + lenB) >= 0;

        return result;
        }
};

int Dot(IVec2 a, IVec2 b)
{
    return a.x * a.x + a.y * a.y;
}

float Distance(IVec2 a, IVec2 b)
{
    return Distance(IVec2ToVec2(a), IVec2ToVec2(b));
}

int Min(int a, int b)
{
    return a < b ? a : b;
}

float Min(float a, float b)
{
    return a < b ? a : b;
}

int Sign(float x)
{
    if (FloatEquals(x, 0)) return 0;
    
    return x < 0 ? -1 : 1;
}

int Sign(int x)
{
    if (x == 0) return 0;
    return x >= 0 ? 1 : -1;
}

int Abs(int x)
{
    return x > 0 ? x : -x;
}

IVec2 Abs(IVec2 val)
{
    return { Abs(val.x), Abs(val.y) };
}

Vec2 IVec2ToVec2(IVec2 val)
{
    return Vec2 { (float)val.x, (float)val.y };
}

struct Vec4
{
    union
    {
        float value[4];

        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
    };

    float & operator[](int idx)
    {
        return value[idx];
    }
};

struct Mat4
{
    union
    {
        Vec4 value[4];
        struct
        {
            float ax;
            float bx;
            float cx;
            float dx;

            float ay;
            float by;
            float cy;
            float dy;

            float az;
            float bz;
            float cz;
            float dz;

            float aw;
            float bw;
            float cw;
            float dw;
        };
    };

    Vec4 & operator[](int col)
    {
        return value[col];
    }
};

Mat4 OrthographicProjection(float left, float right, float top, float bottom)
{
    Mat4 result = {};

    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = 0.0f;
    
    result[0][0] =  2.0f / (right - left);
    result[1][1] =  2.0f / (top - bottom);
    result[2][2] =  1.0f;
    result.dw = 1.0f;

    return result;
}


// NOTE Easing functions
float Linear(float x)
{
    return x;
}

float EaseInSine(float x)
{
    return 1 - cosf((x * PI) / 2);
}

float EaseOutSine(float x)
{
    return sinf((x * PI) / 2);
}

float EaseInOutSine(float x)
{    
    return -(cosf(PI * x) - 1) / 2;
}

float EaseOutCubic(float x)
{
    return 1 - powf(1 - x, 3);
}

float EaseInOutCubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
}

float EaseInQuint(float x)
{
    return x * x * x * x * x;
}


bool8 SameSign(int x, int y)
{
    if (x == 0 || y == 0) return x == y;

    int a = x > 0 ? 1 : -1;
    int b = y > 0 ? 1 : -1;

    return a == b;
}

float EaseInOutBack(float x)
{
    float c1 = 1.70158f;
    float c2 = c1 * 1.525f;

    return x < 0.5
        ? (powf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
        : (powf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}

float EaseOutElastic(float x)
{
    float c4 = (2.0f * PI) / 3.0f;

    float result;
    
    if (FloatEquals(x, 0))
    {
        result = 0;
    }
    else if (FloatEquals(x, 1))
    {
        result = 1;
    }
    else
    {
        result = powf(2, -10 * x) * sinf((x * 10 - 0.75f) * c4) + 1;
    }

    return result;
}

float EaseInOutElastic(float x)
{
    float c5 = (2 * PI) / 4.5f;

    float result;
    
    if (FloatEquals(x, 0))
    {
        result = 0;
    }
    else if (FloatEquals(x, 1))
    {
        result = 1;
    }
    else if (x < 0.5f)
    {
        result = -(powf(2, 20 * x - 10) * sinf((20 * x - 11.125f) * c5)) / 2.0f;
    }
    else
    {
        result = (powf(2, -20 * x + 10) * sinf((20 * x - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    return result;
}

float EaseOutBounce(float x)
{
    float n1 = 7.5625f;
    float d1 = 2.75f;
    if (x < 1 / d1)
    {
        return n1 * x * x;
    } 
    else if (x < 2 / d1)
    {
        return n1 * (x -= 1.5f / d1) * x + 0.75f;
    } 
    else if (x < 2.5 / d1)
    {
        return n1 * (x -= 2.25f / d1) * x + 0.9375f;
    } 
    else
    {
        return n1 * (x -= 2.625f / d1) * x + 0.984375f;
    }
}

float EaseInBounce(float x)
{
    return 1 - EaseOutBounce(1 - x);
}

float EaseInOutBounce(float x)
{
    return x < 0.5f
        ? (1 - EaseOutBounce(1 - 2 * x)) / 2
        : (1 + EaseOutBounce(2 * x - 1)) / 2;
}

void
Pack32(uint32 val,uint8 *dest)
{
    dest[0] = (uint8)((val & 0xff000000) >> 24);
    dest[1] = (uint8)((val & 0x00ff0000) >> 16);
    dest[2] = (uint8)((val & 0x0000ff00) >>  8);
    dest[3] = (uint8)((val & 0x000000ff))      ;
}

#define ENGINE_LIB_H
#endif
