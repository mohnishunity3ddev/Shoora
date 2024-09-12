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

// NOTE: To make sure NSight is able to launch the app.
#define SHU_ENGINE_NAME "Shoora Game Engine"
#define SHU_RENDERER_BACKEND_VULKAN
#define SHU_USE_STB 1
#define SHU_VULKAN_EXAMPLE 1
#define SHU_DEFAULT_FENCE_TIMEOUT 100000000000
#define ARRAY_SIZE(Arr) (sizeof((Arr)) / sizeof(Arr[0]))
#define SHU_VK_ENABLE_MSAA 1
#define SHU_PIXELS_PER_METER 100.0f
#define FPS_CAPPING_ENABLED 0

#define KILOBYTES(Val) ((Val) * 1024)
#define MEGABYTES(Val) (KILOBYTES(Val) * 1024LL)
#define GIGABYTES(Val) (MEGABYTES(Val) * 1024LL)
#define TERABYTES(Val) (GIGABYTES(Val) * 1024LL)

#define SHU_INT_MAX 2147483647
#define SHU_INT_MIN (-SHU_INT_MAX - 1)
#define SHU_UINT_MAX -1UL
#define SHU_UINT_MIN 0UL

#define USE_CPP_ATOMIC 0
#if USE_CPP_ATOMIC
#include <atomic>
// NOTE: This is to stop the compiler from rearranging stuff for optimization. Since this is multithreaded
// code, we have to do this! This prevents any writes that are below it to be moved above it by the compiler during
// optimization phase.
#define CompletePastWritesBeforeFutureWrites std::atomic_thread_fence(std::memory_order_release)
#define CompletePastReadsBeforeFutureReads std::atomic_thread_fence(std::memory_order_acquire)
#else
#include <intrin.h>
// NOTE: sFence makes sure the writes complete at the CPU level. WriteBarrier works at the compiler level so that
// it does not reorder stores/writes during its optimization phase.
#define CompletePastWritesBeforeFutureWrites _WriteBarrier(); _mm_sfence()
#define CompletePastReadsBeforeFutureReads _ReadBarrier()
#endif

#ifdef _MSC_VER
#define SHU_ALIGN_16 __declspec(align(16))
#include <cfloat>
#define SHU_FLOAT_MIN -FLT_MAX
#define SHU_FLOAT_MAX FLT_MAX
#define SHU_EPSILON FLT_EPSILON
#define TEST_STR_FUNCTIONS 0
#if _DEBUG
    #define _SHU_DEBUG 1
    #define SHU_CRASH_DUMP_ENABLE 1
    #undef TEST_STR_FUNCTIONS
    #define TEST_STR_FUNCTIONS 1
#else
    #define _SHU_RELEASE 1
    #define SHU_CRASH_DUMP_ENABLE 1
#endif
#elif defined(__clang__)
#define SHU_FLOAT_MIN -__FLT_MAX__
#define SHU_FLOAT_MAX __FLT_MAX__
#define SHU_ALIGN_16 __attribute__((aligned(16)))
#else
#define ALIGN_16
#endif

inline b32
IsInfinity(const f32 &F)
{
    return (F*0.0f != F*0.0f);
}
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

#define SHU_ABSOLUTE(Val) (((Val) < 0) ? -(Val) : (Val))
#define SIGN(Val) (((Val) < 0) ? (-1) : (1))

// #if _SHU_DEBUG
#define ASSERT(Expression)                                                                                        \
    if (!(Expression))                                                                                            \
    {                                                                                                             \
        *((int volatile *)0) = 0;                                                                                 \
    }
// #else
// #define ASSERT(Expression)
// #endif

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

#define VK_CHECK_RESULT(Call)                                                                                                                                       \
    {                                                                                                                                                               \
        VkResult Result = Call;                                                                                                                                     \
        switch (Result)                                                                                                                                             \
        {                                                                                                                                                           \
            case VK_SUCCESS: { LogInfoUnformatted("VK_SUCCESS:\n"); } break;                                                                                        \
            case VK_NOT_READY: { LogErrorUnformatted("VK_NOT_READY:\n"); } break;                                                                                   \
            case VK_TIMEOUT: { LogErrorUnformatted("VK_TIMEOUT:\n"); } break;                                                                                       \
            case VK_EVENT_SET: { LogErrorUnformatted("VK_EVENT_SET:\n"); } break;                                                                                   \
            case VK_EVENT_RESET: { LogErrorUnformatted("VK_EVENT_RESET:\n"); } break;                                                                               \
            case VK_INCOMPLETE: { LogErrorUnformatted("VK_INCOMPLETE:\n"); } break;                                                                                 \
            case VK_ERROR_OUT_OF_HOST_MEMORY: { LogErrorUnformatted("VK_ERROR_OUT_OF_HOST_MEMORY\n"); } break;                                                      \
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: { LogErrorUnformatted("VK_ERROR_OUT_OF_DEVICE_MEMORY\n"); } break;                                                  \
            case VK_ERROR_INITIALIZATION_FAILED: { LogErrorUnformatted("VK_ERROR_INITIALIZATION_FAILED\n"); } break;                                                \
            case VK_ERROR_DEVICE_LOST: { LogErrorUnformatted("VK_ERROR_DEVICE_LOST\n"); } break;                                                                    \
            case VK_ERROR_MEMORY_MAP_FAILED: { LogErrorUnformatted("VK_ERROR_MEMORY_MAP_FAILED\n"); } break;                                                        \
            case VK_ERROR_LAYER_NOT_PRESENT: { LogErrorUnformatted("VK_ERROR_LAYER_NOT_PRESENT\n"); } break;                                                        \
            case VK_ERROR_EXTENSION_NOT_PRESENT: { LogErrorUnformatted("VK_ERROR_EXTENSION_NOT_PRESENT\n"); } break;                                                \
            case VK_ERROR_FEATURE_NOT_PRESENT: { LogErrorUnformatted("VK_ERROR_FEATURE_NOT_PRESENT\n"); } break;                                                    \
            case VK_ERROR_INCOMPATIBLE_DRIVER: { LogErrorUnformatted("VK_ERROR_INCOMPATIBLE_DRIVER\n"); } break;                                                    \
            case VK_ERROR_TOO_MANY_OBJECTS: { LogErrorUnformatted("VK_ERROR_TOO_MANY_OBJECTS\n"); } break;                                                          \
            case VK_ERROR_FORMAT_NOT_SUPPORTED: { LogErrorUnformatted("VK_ERROR_FORMAT_NOT_SUPPORTED\n"); } break;                                                  \
            case VK_ERROR_FRAGMENTED_POOL: { LogErrorUnformatted("VK_ERROR_FRAGMENTED_POOL\n"); } break;                                                            \
            case VK_ERROR_UNKNOWN: { LogErrorUnformatted("VK_ERROR_UNKNOWN:\n"); } break;                                                                           \
            case VK_ERROR_OUT_OF_POOL_MEMORY: { LogErrorUnformatted("VK_ERROR_OUT_OF_POOL_MEMORY\n"); } break;                                                      \
            case VK_ERROR_INVALID_EXTERNAL_HANDLE: { LogErrorUnformatted("VK_ERROR_INVALID_EXTERNAL_HANDLE\n"); } break;                                            \
            case VK_ERROR_FRAGMENTATION: { LogErrorUnformatted("VK_ERROR_FRAGMENTATION\n"); } break;                                                                \
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: { LogErrorUnformatted("VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS\n"); } break;                              \
            case VK_PIPELINE_COMPILE_REQUIRED: { LogErrorUnformatted("VK_PIPELINE_COMPILE_REQUIRED\n"); } break;                                                    \
            case VK_ERROR_SURFACE_LOST_KHR: { LogErrorUnformatted("VK_ERROR_SURFACE_LOST_KHR\n"); } break;                                                          \
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: { LogErrorUnformatted("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR\n"); } break;                                          \
            case VK_SUBOPTIMAL_KHR: { LogErrorUnformatted("VK_SUBOPTIMAL_KHR:\n"); } break;                                                                         \
            case VK_ERROR_OUT_OF_DATE_KHR: { LogErrorUnformatted("VK_ERROR_OUT_OF_DATE_KHR\n"); } break;                                                            \
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: { LogErrorUnformatted("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR\n"); } break;                                          \
            case VK_ERROR_VALIDATION_FAILED_EXT: { LogErrorUnformatted("VK_ERROR_VALIDATION_FAILED_EXT\n"); } break;                                                \
            case VK_ERROR_INVALID_SHADER_NV: { LogErrorUnformatted("VK_ERROR_INVALID_SHADER_NV\n"); } break;                                                        \
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: { LogErrorUnformatted("VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT\n"); } break;  \
            case VK_ERROR_NOT_PERMITTED_KHR: { LogErrorUnformatted("VK_ERROR_NOT_PERMITTED_KHR\n"); } break;                                                        \
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: { LogErrorUnformatted("VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT\n"); } break;                    \
            case VK_THREAD_IDLE_KHR: { LogErrorUnformatted("VK_THREAD_IDLE_KHR:\n"); } break;                                                                       \
            case VK_THREAD_DONE_KHR: { LogErrorUnformatted("VK_THREAD_DONE_KHR:\n"); } break;                                                                       \
            case VK_OPERATION_DEFERRED_KHR: { LogErrorUnformatted("VK_OPERATION_DEFERRED_KHR\n"); } break;                                                          \
            case VK_OPERATION_NOT_DEFERRED_KHR: { LogErrorUnformatted("VK_OPERATION_NOT_DEFERRED_KHR\n"); } break;                                                  \
            default: { LogError("Unknown Result: %u\n", Result); } break;                                                                                           \
        }                                                                                                                                                           \
    }

#define SHU_MEMZERO(MemPtr, Size)                                                                                 \
    do                                                                                                            \
    {                                                                                                             \
        u8 *Ptr = (u8 *)(MemPtr);                                                                                 \
        for (i32 i = 0; (i < (Size)); ++i)                                                                        \
        {                                                                                                         \
            *Ptr++ = 0;                                                                                           \
        }                                                                                                         \
    } while (0)

#define SHU_MEMSET(MemPtr, Value, Size)                                                                           \
    do                                                                                                            \
    {                                                                                                             \
        u8 *Ptr = (u8 *)(MemPtr);                                                                                 \
        for (i32 i = 0; (i < (Size)); ++i)                                                                        \
        {                                                                                                         \
            *Ptr++ = (Value);                                                                                     \
        }                                                                                                         \
    } while (0)

#define SHU_MEMCOPY(SrcPtr, DestPtr, NumBytes)                                                                    \
    do                                                                                                            \
    {                                                                                                             \
        u8 *Destination = (u8 *)(DestPtr);                                                                        \
        u8 *Source = (u8 *)(SrcPtr);                                                                              \
        for (size_t i = 0; i < NumBytes; ++i)                                                                     \
        {                                                                                                         \
            *Destination++ = *Source++;                                                                           \
        }                                                                                                         \
    } while (0)

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

void Shu_DebugBreak();

inline i32
StringLen(const char *s)
{
    const char *ptr = s;
    i32 len = 0;
    while (*ptr != '\0')
    {
        ++len;
        ptr++;
    }

    return (len + 1);
}

inline b32 StringIsEmpty(const char *str)
{
    b32 result = false;

    if (StringLen(str) == 1 && str[0] == '\0')
        result = true;

    return result;
}

inline b32
StringCompare(const char *a, const char *b)
{
    if (!a || !b)
        return false;

    int m = 0;
    const char *c = a;
    while(*a != '\0')
    {
        m |= *a ^ *b;

        if (*b != '\0')
            b++;

        // NOTE: To keep the function constant time.
        if (*b == '\0')
            c++;

        a++;
    }

    return m == 0;
}

inline void
SubString(const char *str, i32 start, i32 end, char mem[128])
{
    i32 len = end - start;
    ASSERT(len <= 128);
    SHU_MEMCOPY(str + start, mem, len);
    mem[len + 1] = 0;
}

inline b32
IsDigit(char c)
{
    b32 result = false;

    if (c >= '0' && c <= '9')
        result = true;

    return result;
}

inline b32
IsAlpha(char c)
{
    b32 result = ((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c == '_'));
    return result;
}

inline b32
IsAlphaNumeric(char c)
{
    b32 result = IsAlpha(c) || IsDigit(c);
    return result;
}

inline b32
StringIsBoolean(const char *str)
{
    ASSERT(str != nullptr);
    if (StringIsEmpty(str))
        return false;

    b32 result = false;
    if (StringCompare(str, "false") || StringCompare(str, "true"))
        result = true;

    return result;
}

inline b32
StringIsInt(const char *str)
{
    ASSERT(str != nullptr);
    if (StringIsEmpty(str))
        return false;

    const char *p = str;
    if (*p == '-' || *p == '+')
        ++p;

    while (*p != '\0')
    {
        if (!IsDigit(*p++))
            return false;
    }

    return true;
}

inline b32
StringIsDouble(const char *str)
{
    ASSERT(str != nullptr);
    if (StringIsEmpty(str))
        return false;

    b32 in_exponent = false;
    b32 in_fraction = false;

    b32 result = true;
    const char *p = str;
    if (*p == '-' || *p == '+')
        ++p;

    while (*p != '\0')
    {
        char c = *p++;

        if (!IsDigit(c)) {
            if ((c == 'e' || c == 'E')) {
                // two e's isnt allowed.
                if (in_exponent) {
                    result = false;
                    break;
                }

                in_exponent = true;
                continue;
            }

            if (c == '.') {
                // Two decimal dots are not allowed.
                if (in_fraction) {
                    result = false;
                    break;
                }

                in_fraction = true;
                continue;
            }

            result = false;
            break;
        }
    }

    return result;
}

inline char
CharAt(const char *str, i32 at)
{
    char result = str[at];
    return result;
}

enum shoora_object_type
{
    OBJ_TYPE_NULL,
    OBJ_TYPE_INT_32,
    OBJ_TYPE_INT_64,
    OBJ_TYPE_FLOAT,
    OBJ_TYPE_DOUBLE,
    OBJ_TYPE_BOOLEAN
};

inline shoora_object_type
StringToObjType(const char *str)
{
    ASSERT(str != nullptr);

    shoora_object_type result = shoora_object_type::OBJ_TYPE_NULL;
    if (!StringIsEmpty(str))
    {
        if      (StringIsBoolean(str))  { result = shoora_object_type::OBJ_TYPE_BOOLEAN; }
        else if (StringIsInt(str))      { result = shoora_object_type::OBJ_TYPE_INT_32; }
        else if (StringIsDouble(str))   { result = shoora_object_type::OBJ_TYPE_DOUBLE; }
    }

    return result;
}

inline b32
StringToBoolean(const char *str)
{
    ASSERT(str != nullptr);

    b32 result = true;

    if (StringCompare(str, "false"))
        result = false;

    return result;
}

inline i32
StringToInteger(const char *str)
{
    i32 sign = 1;
    i64 ans = 0;
    i32 i = 0;

    while (str[i] == ' ')
    {
        i++;
    }

    if ((str[i] == '-' || str[i] == '+'))
    {
        sign = (str[i] == '-') ? -1 : 1;
        i++;
    }

    i32 result;
    while (str[i] != '\0')
    {
        if (IsDigit(str[i]))
        {
            ans = ans * 10 + (str[i] - '0');
            if (ans * sign <= SHU_INT_MIN)
                return SHU_INT_MIN;
            if (ans * sign >= SHU_INT_MAX)
                return SHU_INT_MAX;
            i++;
        }
        else
        {
            result = 0;
        }
    }

    result = ans * sign;

#if TEST_STR_FUNCTIONS
    int atoi_test = atoi(str);
    ASSERT(atoi_test == result);
#endif

    return result;
}

inline f64
StringToDouble(const char *str)
{
#if 0
    f64 result = 0.0;
    f64 fraction = 1.0;
    b32 negative = false;
    b32 in_fraction = false;
    b32 in_exponent = false;
    i32 exponent_sign = 1;
    i32 exponent_value = 0;

    i32 i = 0;

    // Skip leading whitespaces
    while (str[i] == ' ') { i++; }

    // Handle optional sign
    if (str[i] == '-') {
        negative = true;
        i++;
    }
    else if (str[i] == '+') {
        i++;
    }

    // Process the digits and decimal part
    while (str[i] != '\0')
    {
        if (IsDigit(str[i])) {
            if (in_exponent) {
                exponent_value = exponent_value * 10 + (str[i] - '0');
            }
            else if (in_fraction) {
                fraction /= 10.0;
                result += (str[i] - '0') * fraction;
            }
            else {
                result = result * 10.0 + (str[i] - '0');
            }
        }
        else if (str[i] == '.' && !in_fraction && !in_exponent) {
            in_fraction = true;
        }
        else if ((str[i] == 'e' || str[i] == 'E') && !in_exponent) {
            in_exponent = true;
            i++;
            if (str[i] == '-')
            {
                exponent_sign = -1;
                i++;
            }
            else if (str[i] == '+')
            {
                i++;
            }
            continue;
        }
        else {
            // Invalid character found
            break;
        }

        i++;
    }

    // Apply exponent if necessary
    if (in_exponent) {
        f64 exponent_multiplier = 1.0;

        while (exponent_value > 0) {
            exponent_multiplier *= 10.0;
            exponent_value--;
        }

        if (exponent_sign == -1) {
            result /= exponent_multiplier;
        }
        else {
            result *= exponent_multiplier;
        }
    }

    if (negative) { result = -result; }

#if TEST_STR_FUNCTIONS
    char *end;
    double test = strtod(str, &end);
    ASSERT(test == result);
#endif
#else
    // TODO: Have to check for rounding errors. 1.213 gets parsed to 1.21299999999
    // TODO: strtod returns 1.21300001 which is better
    char *end;
    f64 result = strtod(str, &end);
#endif

    return result;
}

template <typename T>
inline T
ClampToRange(T Value, T Min, T Max)
{
    if(Value < Min) {Value = Min;}
    else if(Value > Max) {Value = Max;}
    return Value;
}

// NOTE: Only works for power of 2 alignments.
inline u64
AlignAsPow2(u64 Number, u64 Alignment)
{
    u64 Result = Number;
    u64 AlignmentMask = (u64)(Alignment - 1);
    if(Number & AlignmentMask)
    {
        u64 Offset = Alignment - (Number & AlignmentMask);
        Result += Offset;
    }
    return Result;
}

inline u64
AlignAsGeneric(u64 Number, u64 Alignment)
{
    u64 Result = Number;
    if(Alignment != 0)
    {
        if(Number % Alignment != 0)
        {
            Result = Alignment * ((Number / Alignment) + 1);
        }
    }

    return Result;
}

// NOTE: Only works for power of 2 alignments.
inline u64
NextAlignAsPow2(u64 Number, u64 Alignment)
{
    u64 Result = AlignAsPow2(Number + 1, Alignment);
    ASSERT(Result > Number);
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
    auto t = SHU_ABSOLUTE(n - expected);
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
                    LogWarnUnformatted("Worth inspecting this!\n");
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
        LogWarnUnformatted("Worth inspecting this!\n");
    }

    return Result;
}

#define ALIGN4(Number) AlignAsPow2((Number), 4)
#define ALIGN8(Number) AlignAsPow2((Number), 8)
#define ALIGN16(Number) AlignAsPow2((Number), 16)
#define ALIGN32(Number) AlignAsPow2((Number), 32)
#define ALIGN64(Number) AlignAsPow2((Number), 64)

template<typename T>
struct stack_array
{
    T *data;
    i32 size;
    i32 maxSize;

    stack_array() = delete;
    stack_array(T *d, i32 max) : data(d), size(0), maxSize(max) {}

    inline void
    add(const T &item)
    {
        ASSERT((size + 1) <= maxSize);
        data[size++] = item;
    }

    inline void
    erase(i32 index)
    {
        ASSERT(index < size);
        if(index != size - 1)
        {
            for(i32 i = index; (i < size); ++i)
            {
                data[i] = data[i + 1];
            }
        }

        size--;
    }

    inline void
    clear()
    {
        SHU_MEMZERO(data, maxSize * sizeof(T));
    }
};

struct shoora_object
{
    shoora_object_type type;
    union
    {
        i32 i;
        i64 ii;
        f32 f;
        f64 d;
        b32 b;
    };

    shoora_object()
    {
        type = shoora_object_type::OBJ_TYPE_NULL;
        d = 0.0;
    }

    shoora_object(i32 val)
    {
        this->type = shoora_object_type::OBJ_TYPE_INT_32;
        this->i = val;
    }

    shoora_object(i64 val)
    {
        this->type = shoora_object_type::OBJ_TYPE_INT_64;
        this->ii = val;
    }

    shoora_object(f32 val)
    {
        this->type = shoora_object_type::OBJ_TYPE_FLOAT;
        this->f = val;
    }

    shoora_object(f64 val)
    {
        this->type = shoora_object_type::OBJ_TYPE_DOUBLE;
        this->d = val;
    }

    shoora_object(bool val)
    {
        this->type = shoora_object_type::OBJ_TYPE_BOOLEAN;
        this->b = val;
    }

    shoora_object(const char *str)
    {
        ASSERT(str != nullptr);

        shoora_object_type t = StringToObjType(str);
        switch (t)
        {
            case shoora_object_type::OBJ_TYPE_INT_32:
            {
                i32 Value = StringToInteger(str);
                this->i = Value;
            } break;

            case shoora_object_type::OBJ_TYPE_DOUBLE:
            {
                f64 Value = StringToDouble(str);
                this->d = Value;
            } break;

            case shoora_object_type::OBJ_TYPE_BOOLEAN:
            {
                b32 Value = StringToBoolean(str);
                this->b = Value;
            } break;

            case shoora_object_type::OBJ_TYPE_NULL:
            break;

            SHU_INVALID_DEFAULT
        }
        this->type = t;
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