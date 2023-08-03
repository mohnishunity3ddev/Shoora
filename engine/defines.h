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

#define SHU_ENGINE_NAME "Shura Game Engine"
#define SHU_RENDERER_BACKEND_VULKAN

#define ARRAY_SIZE(Arr) sizeof(Arr) / sizeof(Arr[0])

#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((int volatile *)0) = 0;                                                                                 \
    }

#define VK_CHECK(Call)                                                                                            \
    {                                                                                                             \
        ASSERT(Call == VK_SUCCESS);                                                                               \
    }

#define MEMZERO(MemPtr, Size)                                                                                     \
    {                                                                                                             \
        u8 *Ptr = (u8 *)MemPtr;                                                                                   \
        for (i32 i = 0; i < Size; ++i)                                                                            \
        {                                                                                                         \
            *Ptr++ = 0;                                                                                           \
        }                                                                                                         \
    }
inline u64
AlignAs(u64 Number, u32 AlignAs)
{
    u64 Result;
    u64 AlignmentMask = (u64)(AlignAs - 1);
    Result = (Number + AlignmentMask) & (~AlignmentMask);
    return Result;
}
#define ALIGN4(Number) AlignAs(Number, 4)
#define ALIGN8(Number) AlignAs(Number, 8)
#define ALIGN16(Number) AlignAs(Number, 16)
#define ALIGN32(Number) AlignAs(Number, 32)
#define ALIGN64(Number) AlignAs(Number, 64)

#define WIN32_LOG_OUTPUT(FormatString, ...)                                                                       \
    {                                                                                                             \
        char TextBuffer[256];                                                                                     \
        _snprintf_s(TextBuffer, 256, FormatString, __VA_ARGS__);                                                  \
        OutputDebugStringA(TextBuffer);                                                                           \
    }

#ifdef _MSC_VER
#define SHU_EXPORT __declspec(dllexport)
#elif defined(__clang__)
#define SHU_EXPORT __attribute__((visibility("default")))
#endif




#define DEFINES_H
#endif // DEFINES_H