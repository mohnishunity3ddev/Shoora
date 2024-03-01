#if !defined(UTILS_H)
#define UTILS_H

#include <defines.h>
#include "sort/sort.h"
#include <math/math.h>

struct colorU32
{
    static const u32 Red = 0xffff0000;
    static const u32 Green = 0xff00ff00;
    static const u32 Blue = 0xff0000ff;
    static const u32 Cyan = 0xff00ffff;
    static const u32 Magenta = 0xffff00ff;
    static const u32 Yellow = 0xffffff00;
    static const u32 White = 0xffffffff;
    static const u32 Gray = 0xff313131;
    static const u32 Proto_Orange = 0xffFB8621;
    static const u32 Proto_Yellow = 0xffFFB900;
    static const u32 Proto_Green = 0xff6DA174;
    static const u32 Proto_Red = 0xffBD4334;
    static const u32 Proto_Blue = 0xff697FC4;
};

u32 GetMaxValueIndex(u32 *NumsArray, u32 NumsCount);
u32 LogBase2(u32 Num);

u32 StringLength(const char *A);
void StringConcat(const char *A, const char *B, char *Out);
void StringConcat(char *A, const char *B);
void StringNCopy(const char *Src, char *Dst, i32 Num);
void StringCopy(const char *Src, char *Dst);

u32 StringFindLastOf(const char *String, char Separator);
void StringSubString(const char *SrcString, u32 StartIndex, u32 EndIndex, char *OutString);
b32 StringsEqual(const char *A, const char *B);

shu::vec3f GetColor(u32 Col);
u32 GetColorU32(const shu::vec3f &Color);

#if _SHU_DEBUG

#include <type_traits>
#include <cstring>

template <typename T>
void
GetFormatSpecifier(char *Buffer, T Value)
{
    if constexpr (std::is_same_v<T, int>) { StringCopy("%d", Buffer); }
    else if constexpr (std::is_same_v<T, unsigned int>) { StringCopy("%u", Buffer); }
    else if constexpr (std::is_same_v<T, long>) { StringCopy("%ld", Buffer); }
    else if constexpr (std::is_same_v<T, unsigned long>) { StringCopy("%lu", Buffer); }
    else if constexpr (std::is_same_v<T, long long>) { StringCopy("%lld", Buffer); }
    else if constexpr (std::is_same_v<T, unsigned long long>) { StringCopy("%llu", Buffer); }
    else if constexpr (std::is_same_v<T, float>) { StringCopy("%f", Buffer); }
    else if constexpr (std::is_same_v<T, double>) { StringCopy("%lf", Buffer); }
    else if constexpr (std::is_same_v<T, char>) { StringCopy("%c", Buffer); }
    else if constexpr (std::is_same_v<T, const char *>) { StringCopy("%s", Buffer); }
    else if constexpr (std::is_pointer_v<T>) { StringCopy("%p", Buffer); }
    else { SHU_INVALID_CODEPATH; }
}

struct templated_log_string
{
  private:
    char Buffer[4096];
    i32 Index;

  public:
    templated_log_string() : Index(0) {}

    templated_log_string& operator<<(const char *Rhs)
    {
        StringConcat(Buffer + Index, Rhs);
        Index += StringLength(Rhs);
        return *this;
    }

    template <typename T>
    templated_log_string& operator<<(const T& Rhs)
    {
        char FormatSpec[5];
        GetFormatSpecifier(FormatSpec, Rhs);

        StringConcat(Buffer + Index, FormatSpec);
        Index += StringLength(FormatSpec);
        return *this;
    }

    char *ToString()
    {
        Buffer[Index] = '\0';
        Index = 0;
        return Buffer;
    }
};
typedef templated_log_string templateString;

// Base case: when there's only one argument left
template <typename T>
const char *
AccumulateTlsString(templateString &Tls, T arg)
{
    Tls << arg;
    return Tls.ToString();
}

template <typename T, typename... VarArgs>
void
AccumulateTlsString(templateString &Tls, T FirstArg, VarArgs... Args)
{
    Tls << FirstArg;
    AccumulateTlsString(Tls, Args...);
}

#define TemplatedLogString(Buffer, ...)                                                                           \
    do                                                                                                            \
    {                                                                                                             \
        tls Tls;                                                                                                  \
        AccumulateTlsString(Tls, __VA_ARGS__);                                                                    \
        StringCopy(Tls.ToString(), Buffer);                                                                       \
    } while (0)

#define TemplatedLog(LogFunc, s1, v1)                                                                             \
    do                                                                                                            \
    {                                                                                                             \
        char FormatSpecifier[5];                                                                                  \
        GetFormatSpecifier(FormatSpecifier, v1);                                                                  \
                                                                                                                  \
        char Buffer2[4096];                                                                                       \
        char FormatString[4098];                                                                                  \
        StringConcat(s1, "%s", FormatString);                                                                     \
        Platform_GenerateString(Buffer2, ARRAY_SIZE(Buffer2), FormatString, FormatSpecifier);                     \
                                                                                                                  \
        LogFunc(Buffer2, v1);                                                                                     \
    } while (0)

#elif
#define TemplatedLog(LogFunc, s1, v1)
#define TemplatedLogString(Buffer, ...)

#endif

#endif // UTILS_H