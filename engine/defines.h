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

#define SHU_USE_GLM 0
#if SHU_USE_GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#endif

#define SHU_CRASH_DUMP_ENABLE 0
// NOTE: To make sure NSight is able to launch the app.
#define SHU_ENGINE_NAME "Shoora Game Engine"
#define SHU_RENDERER_BACKEND_VULKAN
#define SHU_USE_STB 1
#define SHU_VULKAN_EXAMPLE 1
#define SHU_DEFAULT_FENCE_TIMEOUT 100000000000
#define ARRAY_SIZE(Arr) sizeof(Arr) / sizeof(Arr[0])
#define SHU_VK_ENABLE_MSAA 1
#define SHU_PIXELS_PER_METER 100.0f
#define FPS_CAPPING_ENABLED 0

#define SHU_INT_MIN (i32)1 << 31

#ifdef _MSC_VER
#define SHU_ALIGN_16 __declspec(align(16))
#include <cfloat>
#define SHU_FLOAT_MIN -FLT_MAX
#define SHU_FLOAT_MAX FLT_MAX
#define SHU_EPSILON FLT_EPSILON
#if _DEBUG
    #define _SHU_DEBUG 1
#else
    #define _SHU_RELEASE 1
#endif
#elif defined(__clang__)
#define SHU_FLOAT_MIN -__FLT_MAX__
#define SHU_FLOAT_MAX __FLT_MAX__
#define SHU_ALIGN_16 __attribute__((aligned(16)))
#else
#define ALIGN_16
#endif

#define SHU_PI (3.14159265359f)
#define SHU_PI_BY_2 (1.57079632679f)
#define DEG_TO_RAD (SHU_PI / 180.0f)
#define RAD_TO_DEG (180.0f / SHU_PI)

enum shoora_quality
{
    Quality_Low,
    Quality_Medium,
    Quality_High,

    Quality_Count
};

#define ABSOLUTE(Val) (((Val) < 0) ? (-Val) : (Val))
#define SIGN(Val) (((Val) < 0) ? (-1) : (1))

#if _SHU_DEBUG
#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((int volatile *)0) = 0;                                                                                 \
    }
#else
#define ASSERT(Expression)
#endif

#define OFFSET_OF(Var, Member) (u64)(&(((Var *)0)->Member))
#define SHU_INVALID_DEFAULT                                                                                       \
    default:                                                                                                      \
    {                                                                                                             \
        ASSERT(!"Invalid Default Case");                                                                          \
    } break;
#define SHU_INVALID_CODEPATH ASSERT(!"Invalid Code Path")

#define VK_CHECK(Call)                                                                                            \
    {                                                                                                             \
        ASSERT(Call == VK_SUCCESS);                                                                               \
    }

#define VK_CHECK_RESULT(Call)                                                                                     \
    {                                                                                                             \
        VkResult Result = Call;                                                                                   \
        switch (Result)                                                                                           \
        {                                                                                                         \
        case VK_SUCCESS:                                                                                          \
        {                                                                                                         \
            LogInfoUnformatted("VK_SUCCESS:\n");                                                                  \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_NOT_READY:                                                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_NOT_READY:\n");                                                               \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_TIMEOUT:                                                                                          \
        {                                                                                                         \
            LogErrorUnformatted("VK_TIMEOUT:\n");                                                                 \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_EVENT_SET:                                                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_EVENT_SET:\n");                                                               \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_EVENT_RESET:                                                                                      \
        {                                                                                                         \
            LogErrorUnformatted("VK_EVENT_RESET:\n");                                                             \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_INCOMPLETE:                                                                                       \
        {                                                                                                         \
            LogErrorUnformatted("VK_INCOMPLETE:\n");                                                              \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_OUT_OF_HOST_MEMORY:                                                                         \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_OUT_OF_HOST_MEMORY\n");                                                 \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:                                                                       \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_OUT_OF_DEVICE_MEMORY\n");                                               \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INITIALIZATION_FAILED:                                                                      \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INITIALIZATION_FAILED\n");                                              \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_DEVICE_LOST:                                                                                \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_DEVICE_LOST\n");                                                        \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_MEMORY_MAP_FAILED:                                                                          \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_MEMORY_MAP_FAILED\n");                                                  \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_LAYER_NOT_PRESENT:                                                                          \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_LAYER_NOT_PRESENT\n");                                                  \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_EXTENSION_NOT_PRESENT:                                                                      \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_EXTENSION_NOT_PRESENT\n");                                              \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_FEATURE_NOT_PRESENT:                                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_FEATURE_NOT_PRESENT\n");                                                \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INCOMPATIBLE_DRIVER:                                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INCOMPATIBLE_DRIVER\n");                                                \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_TOO_MANY_OBJECTS:                                                                           \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_TOO_MANY_OBJECTS\n");                                                   \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_FORMAT_NOT_SUPPORTED:                                                                       \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_FORMAT_NOT_SUPPORTED\n");                                               \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_FRAGMENTED_POOL:                                                                            \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_FRAGMENTED_POOL\n");                                                    \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_UNKNOWN:                                                                                    \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_UNKNOWN:\n");                                                           \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_OUT_OF_POOL_MEMORY:                                                                         \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_OUT_OF_POOL_MEMORY\n");                                                 \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:                                                                    \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INVALID_EXTERNAL_HANDLE\n");                                            \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_FRAGMENTATION:                                                                              \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_FRAGMENTATION\n");                                                      \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:                                                             \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS\n");                                     \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_PIPELINE_COMPILE_REQUIRED:                                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_PIPELINE_COMPILE_REQUIRED\n");                                                \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_SURFACE_LOST_KHR:                                                                           \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_SURFACE_LOST_KHR\n");                                                   \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                                                                   \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR\n");                                           \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_SUBOPTIMAL_KHR:                                                                                   \
        {                                                                                                         \
            LogErrorUnformatted("VK_SUBOPTIMAL_KHR:\n");                                                          \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_OUT_OF_DATE_KHR:                                                                            \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_OUT_OF_DATE_KHR\n");                                                    \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                                                                   \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR\n");                                           \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_VALIDATION_FAILED_EXT:                                                                      \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_VALIDATION_FAILED_EXT\n");                                              \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INVALID_SHADER_NV:                                                                          \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INVALID_SHADER_NV\n");                                                  \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:                                               \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT\n");                       \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_NOT_PERMITTED_KHR:                                                                          \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_NOT_PERMITTED_KHR\n");                                                  \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:                                                        \
        {                                                                                                         \
            LogErrorUnformatted("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT\n");                                \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_THREAD_IDLE_KHR:                                                                                  \
        {                                                                                                         \
            LogErrorUnformatted("VK_THREAD_IDLE_KHR:\n");                                                         \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_THREAD_DONE_KHR:                                                                                  \
        {                                                                                                         \
            LogErrorUnformatted("VK_THREAD_DONE_KHR:\n");                                                         \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_OPERATION_DEFERRED_KHR:                                                                           \
        {                                                                                                         \
            LogErrorUnformatted("VK_OPERATION_DEFERRED_KHR\n");                                                   \
        }                                                                                                         \
        break;                                                                                                    \
        case VK_OPERATION_NOT_DEFERRED_KHR:                                                                       \
        {                                                                                                         \
            LogErrorUnformatted("VK_OPERATION_NOT_DEFERRED_KHR\n");                                               \
        }                                                                                                         \
        break;                                                                                                    \
        default:                                                                                                  \
        {                                                                                                         \
            LogError("Unknown Result: %u\n", Result);                                                             \
        }                                                                                                         \
        break;                                                                                                    \
        }                                                                                                         \
    }

#define MEMZERO(MemPtr, Size)                                                                                     \
    {                                                                                                             \
        u8 *Ptr = (u8 *)(MemPtr);                                                                                 \
        for (i32 i = 0; (i < (Size)); ++i)                                                                        \
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

#define NANOSECONDS(Seconds) ((Seconds) * 1000000000)
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

template <typename T>
inline T
ClampToRange(T Value, T Min, T Max)
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

template<typename T>
struct interval {
    T low;
    T high;

    b32 operator == (const interval<T> &other) {
        b32 result = low == other.low && high == other.high;
        return result;
    }
    b32 operator != (const interval<T> &other) {
        b32 result = low != other.low || high != other.high;
        return result;
    }
};

template <typename T>
inline b32
DefaultLessComparator(const T &a, const T &b)
{
    b32 Result = a < b;
    return Result;
}
template <typename T>
inline b32
DefaultLessEqualComparator(const T &a, const T &b)
{
    b32 Result = a <= b;
    return Result;
}
template <typename T>
inline b32
DefaultGreaterComparator(const T &a, const T &b)
{
    b32 Result = a > b;
    return Result;
}
template <typename T>
inline b32
DefaultGreaterEqualComparator(const T &a, const T &b)
{
    b32 Result = a >= b;
    return Result;
}

#define SWAP(a, b)                                                                                                \
    {                                                                                                             \
        auto temp = (a);                                                                                          \
        (a) = (b);                                                                                                \
        (b) = temp;                                                                                               \
    }

inline b32
NearlyEqual(f32 n, f32 expected, f32 epsilon = 2.0f*SHU_EPSILON)
{
    auto t = ABSOLUTE(n - expected);
    b32 Result = (t <= epsilon);
    return Result;
}

// inline b32
// NearlyEqualUlps(f32 n, f32 expected, i64 maxUlps = 2)
// {
//     b32 Result = false;
//     if(n == expected) {
//         Result = true;
//     } else {
//         u32 uN = *((u32 *)&n);
//         u32 uExp = *((u32 *)&expected);
//         u32 diff = 0;
//         if(uExp > uN) {
//             diff = uExp - uN;
//         } else {
//             diff = uN - uExp;
//         }
//         Result = (diff <= maxUlps);
//     }

//     return Result;
// }

#define SHU_NEARLY_EQ(n, e, type, diff, res)                                                                      \
    {                                                                                                             \
        type uN = *((type *)&n);                                                                                  \
        type uExp = *((type *)&e);                                                                                \
        if (uExp > uN)                                                                                            \
        {                                                                                                         \
            diff = uExp - uN;                                                                                     \
        }                                                                                                         \
        else                                                                                                      \
        {                                                                                                         \
            diff = uN - uExp;                                                                                     \
        }                                                                                                         \
        res = (diff <= maxUlps);                                                                                  \
    }

template<typename T>
inline b32
NearlyEqualUlps(T n, T expected, i64 maxUlps = 2)
{
    b32 Result = false;
    u64 Diff = 0;
    if(n == expected) {
        Result = true;
    } else {
        if(sizeof(T) == 8) // Double
        {
            u64 diff = 0;
            SHU_NEARLY_EQ(n, expected, u64, diff, Result);
            Diff = diff;
        }
        else if (sizeof(T) == 4)
        {
            u32 diff = 0;
            {
                u32 uN = *((u32 *)&n);
                u32 uExp = *((u32 *)&expected);
                // Check for -0.0f
                if(uN == 0x8fffffff && expected >= 0.0f) uN = 0;
                if(uExp == 0x8fffffff && n >= 0.0f) uExp = 0;
                if (uExp > uN)
                {
                    diff = uExp - uN;
                }
                else
                {
                    diff = uN - uExp;
                }
                Result = (diff <= maxUlps);
                if(!Result) {
                    int x = 0;
                }
            };
            Diff = diff;
        }
        else
        {
            SHU_INVALID_CODEPATH;
        }
    }

    if(!Result)
    {
        int x = 0;
    }

    return Result;
}

#define ALIGN4(Number) AlignAs((Number), 4)
#define ALIGN8(Number) AlignAs((Number), 8)
#define ALIGN16(Number) AlignAs((Number), 16)
#define ALIGN32(Number) AlignAs((Number), 32)
#define ALIGN64(Number) AlignAs((Number), 64)

template<typename T>
struct arr
{
    T *data;
    i32 size;
    i32 maxSize;

    arr() = delete;
    arr(T *d, i32 max) : data(d), size(0), maxSize(max) {}

    inline void
    add(const T &item)
    {
        ASSERT((size + 1) < maxSize);
        data[size++] = item;
    }
};

#define WIN32_LOG_OUTPUT(FormatString, ...)                                                                       \
    {                                                                                                             \
        char TextBuffer[256];                                                                                     \
        _snprintf_s(TextBuffer, 256, (FormatString), __VA_ARGS__);                                                \
        OutputDebugStringA(TextBuffer);                                                                           \
    }

#ifdef _MSC_VER
#define SHU_EXPORT __declspec(dllexport)
#elif defined(__clang__)
#define SHU_EXPORT __attribute__((visibility("default")))
#endif

#define DEFINES_H
#endif // DEFINES_H