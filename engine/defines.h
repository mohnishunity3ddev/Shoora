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
#define SHU_INVALID_DEFAULT                                                                                       \
    default:                                                                                                      \
    {                                                                                                             \
        ASSERT(!"Invalid Default Case");                                                                          \
    }

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