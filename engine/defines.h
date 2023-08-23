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

#define SHU_ENGINE_NAME "Shoora Game Engine"
#define SHU_RENDERER_BACKEND_VULKAN
#define SHU_USE_STB 1
#define SHU_VULKAN_EXAMPLE 1
#define SHU_DEFAULT_FENCE_TIMEOUT 100000000000
#define ARRAY_SIZE(Arr) sizeof(Arr) / sizeof(Arr[0])

enum shoora_quality
{
    Quality_Low,
    Quality_Medium,
    Quality_High,

    Quality_Count
};

#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((int volatile *)0) = 0;                                                                                 \
    }

#define OFFSET_OF(Var, Member) (u64)(&(((Var *)0)->Member))

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
#define SET_FLAG_BITS_IF_EQUAL(FlagsToSet, FirstFlagsToCheck, SecondFlagsToCheck, NumberOfBits)                   \
    {                                                                                                             \
        for (u32 BitIndex = 0; BitIndex < (NumberOfBits); ++BitIndex)                                             \
        {                                                                                                         \
            u32 BitMask = (1 << BitIndex);                                                                        \
            if (((FirstFlagsToCheck)&BitMask) && ((SecondFlagsToCheck)&BitMask))                                  \
            {                                                                                                     \
                (FlagsToSet) |= BitMask;                                                                          \
            }                                                                                                     \
        }                                                                                                         \
    }

#define NANOSECONDS(Seconds) Seconds * 1000000000
#define MAX(A, B) (A) > (B) ? A : B
#define MIN(A, B) (A) < (B) ? A : B

inline u32
ClampToRange(u32 Value, u32 Min, u32 Max)
{
    if(Value < Min) {Value = Min;}
    else if(Value > Max) {Value = Max;}
    return Value;
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