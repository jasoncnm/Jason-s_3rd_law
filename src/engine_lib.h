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


//  ========================================================================
// NOTE: Defines
//  ========================================================================
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

#define b8 char
#define BIT(x) 1 << (x)
#define KB(x) (1024LL * x)
#define MB(x) (1024LL * KB(x))
#define GB(x) (1024LL * MB(x))

#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

//  ========================================================================
// NOTE: Logging
//  ========================================================================
enum TextColor
{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT,
};

template <typename ...Args>
void _log(char * prefix, char * msg, TextColor textColor, Args... args)
{
    static char * TextColorTable[TEXT_COLOR_COUNT] =
        {
            "\x1b[30m", // TEXT_COLOR_BLACK,
            "\x1b[31m", // TEXT_COLOR_RED,
            "\x1b[32m", // TEXT_COLOR_GREEN,
            "\x1b[33m", // TEXT_COLOR_YELLOW,
            "\x1b[34m", // TEXT_COLOR_BLUE,
            "\x1b[35m", // TEXT_COLOR_MAGENTA,
            "\x1b[36m", // TEXT_COLOR_CYAN,
             "\x1b[37m", // TEXT_COLOR_WHITE,
            "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK,
            "\x1b[91m", // TEXT_COLOR_BRIGHT_RED,
            "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN,
            "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW,
            "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE,
            "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA,
            "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN,
            "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE,
        };

    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

    char textBuffer[8192] = {};
    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer);
}

#if GAME_INTERNAL

#define SM_TRACE(msg, ...) _log("Trace:   ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(msg, ...)  _log("Warning: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("Error:   ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);
#define SM_ASSERT(x, msg, ...)                    \
{                                                 \
    if (!(x))                                     \
    {                                             \
        SM_ERROR(msg, ##__VA_ARGS__);             \
        DEBUG_BREAK();                            \
    }                                             \
}

#else

#define SM_TRACE(msg, ...)
#define SM_WARN(msg, ...)
#define SM_ERROR(msg, ...)
#define SM_ASSERT(x, msg, ...)

#endif

//  ========================================================================
// NOTE: Array
//  ========================================================================
template<typename T, int N>
struct Array
{
    static constexpr int maxElements = N;
    int count = 0;
    T elements[N];

    T & operator[](int idx)
    {
        SM_ASSERT(idx >= 0, "Idx negative!");
        SM_ASSERT(idx < count, "Idx out of bounds!");
        return elements[idx];
    }

    T & last()
    {
        SM_ASSERT(!IsEmpty(), "Array is empty!");
        return elements[count - 1];
    }

    int Add(T element)
    {
        SM_ASSERT(count < maxElements, "Array Full!");
        elements[count] = element;
        return count++;
    }

    std::vector<T> GetVectorSTD()
    {
        std::vector<T> result;
        result.reserve(count);
        for (int i = 0; i < count; i++)
        {
            result.push_back(elements[i]);
        }
        
        return result;
    }

    void RemoveIdxAndSwap(int idx)
    {
        SM_ASSERT(idx >= 0, "Idx negative!");
        SM_ASSERT(idx < count, "Idx out of bounds!");
        elements[idx] = elements[--count];
    }

    void ReverseElements()
    {
        if (count > 1)
        {
            for (int i = 0; i < count / 2; i++)
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

    bool IsFull()
    {
        return count == N;
    }

    bool IsEmpty()
    {
        return count == 0;
    }

};


//  ========================================================================
// NOTE: Bump Allocator
//  ========================================================================
struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char *memory;

};

BumpAllocator MakeBumpAllocator(size_t size)
{
    BumpAllocator ba = {};
    ba.memory = (char *)malloc(size);

    if (ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size); // NOTE: Set the memory to zero
    }
    else
    {
        SM_ASSERT(false, "Failed to allocate memory!");
    }
    
    return ba;
}

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

//  ========================================================================
// NOTE: File I/O
//  ========================================================================
long long GetTimestamp(char * file)
{
    struct stat fileStat = {};
    stat(file, &fileStat);
    return fileStat.st_mtime;
}

bool FileExists(char * filePath)
{
    SM_ASSERT(filePath, "No file path provided!");

    auto file = fopen(filePath, "rb");
    if (!file)
    {
        return false;
    }
    fclose(file);

    return true;    
}

long GetFileSize(char * filePath)
{
    SM_ASSERT(filePath, "No file path provided!");

    long fileSize = 0;
    
    auto file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
        
    fclose(file);

    return fileSize;
}

// NOTE: Reads a file into a supplied buffer. We manage our own memory and therefore want more contorl over where it is allocated
char * read_file(char * filePath, int * fileSize, char * buffer)
{
    SM_ASSERT(filePath, "No file pth provided!");
    SM_ASSERT(fileSize, "No file size provided!");
    SM_ASSERT(buffer,   "No buffer provided!");

    *fileSize = 0;
    
    auto file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);

    fclose(file);

    return buffer;
    
}

char * read_file(char *filePath, int * fileSize, BumpAllocator * ba)
{
    char * file = nullptr;
    long fileSize2 = GetFileSize(filePath);
    if (fileSize2)
    {
        char * buffer = BumpAlloc(ba, fileSize2 + 1);
        file = read_file(filePath, fileSize, buffer);
    }

    return file;

}

void write_file(char * filePath, char * buffer, int size)
{
    SM_ASSERT(filePath, "No file path provided!");
    SM_ASSERT(buffer,   "No buffer provided!");
    
    auto file = fopen(filePath, "wb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return;
    }

    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char * fileName, char * outputName, char * buffer)
{
    int fileSize = 0;
    char * data = read_file(fileName, &fileSize, buffer);

    auto outputFile = fopen(outputName, "wb");
    if (!outputFile)
    {
        SM_ERROR("Failed to open file: %s", outputName);
        return false;
    }

    size_t result = fwrite(data, sizeof(char), fileSize, outputFile);
    if (!result)
    {
        SM_ERROR("Failed to open file: %s", outputName);
        return false;
    }

    fclose(outputFile);

    return true;
}


bool copy_file(char * fileName, char * outputName, BumpAllocator * ba)
{

    char * file = nullptr;
    long fileSize = GetFileSize(fileName);
    if (fileSize)
    {
        char * buffer = BumpAlloc(ba, fileSize + 1);
        return copy_file(fileName, outputName, buffer);
    }

    return false;
    
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

    bool operator==(IVec2 other)
    {
        return (x == other.x) && (y == other.y); 
    }

    bool operator!=(IVec2 other)
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
    
    bool IsBetween(IVec2 a, IVec2 b)
    {
        float lenAB = Distance(IVec2ToVec2(a), IVec2ToVec2(b));
        float lenA = Distance(IVec2ToVec2(a), IVec2ToVec2(*this));
        float lenB = Distance(IVec2ToVec2(b), IVec2ToVec2(*this));

        bool result = lenAB - (lenA + lenB) >= 0;

        return result;
        
    }
};

float Distance(IVec2 a, IVec2 b)
{
    return Distance(IVec2ToVec2(a), IVec2ToVec2(b));
}

int Sign(int x)
{
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

bool SameSign(int x, int y)
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

    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2 / d1) {
        return n1 * (x -= 1.5f / d1) * x + 0.75f;
    } else if (x < 2.5 / d1) {
        return n1 * (x -= 2.25f / d1) * x + 0.9375f;
    } else {
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
#define ENGINE_LIB_H
#endif
