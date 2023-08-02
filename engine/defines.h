#if !defined(DEFINES_H)
#include <stdint.h>

typedef int8_t b8;
typedef int16_t b16;
typedef int32_t b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float f32;
typedef double f64;

#define ARRAY_SIZE(Arr) sizeof(Arr) / sizeof(Arr[0])
#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((int volatile *)0) = 0;                                                                                 \
    }
#define VK_CHECK(Call)                                                                                            \
    {                                                                                                             \
        SU_ASSERT(Call == VK_SUCCESS);                                                                            \
    }
#define MEMZERO(MemPtr, Size)                                                                                     \
    {                                                                                                             \
        u8 *Ptr = (u8 *)MemPtr;                                                                                   \
        for (i32 i = 0; i < Size; ++i)                                                                            \
        {                                                                                                         \
            *Ptr++ = 0;                                                                                           \
        }                                                                                                         \
    }
#define WIN32_LOG_OUTPUT(FormatString, ...)                                                                       \
    {                                                                                                             \
        char TextBuffer[256];                                                                                     \
        _snprintf_s(TextBuffer, 256, FormatString, __VA_ARGS__);                                                  \
        OutputDebugStringA(TextBuffer);                                                                           \
    }

#define SU_EXPORT __declspec(dllimport)

const char *EngineName = "Shura Engine";
#define DEFINES_H
#endif // DEFINES_H