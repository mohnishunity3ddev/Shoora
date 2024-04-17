#include "utils.h"
static const u32 DebugColors[] = {
    0xffff0000, // Red
    0xff00ff00, // Green
    0xff0000ff, // Blue
    0xff00ffff, // Cyan
    0xffff00ff, // Magenta
    0xffffff00, // Yellow
    0xffffffff, // White
    0xff313131, // Gray
    0xffFB8621, // Proto_Orange
    0xffFFB900, // Proto_Yellow
    0xff6DA174, // Proto_Green
    0xffBD4334, // Proto_Red
    0xff697FC4 // Proto_Blue
};

shu::vec3f
GetColor(u32 Col)
{
    f32 m = 1.0f / 255.0f;

    u32 r = (Col & 0x00ff0000) >> 16;
    u32 g = (Col & 0x0000ff00) >> 8;
    u32 b = (Col & 0x000000ff) >> 0;

    shu::vec3f Result = shu::Vec3f(r, g, b) * m;
    return Result;
}

u32
GetDebugColor(i32 Index)
{
    i32 i = Index % ARRAY_SIZE(DebugColors);
    return DebugColors[i];
}

u32
GetColorU32(const shu::vec3f &Color)
{
    u32 ColorU32 = 0xff000000;

    u32 r = (u32)(Color.r * 255.0f);
    ColorU32 |= (r << 16);
    u32 g = (u32)(Color.g * 255.0f);
    ColorU32 |= (g << 8);
    u32 b = (u32)(Color.b * 255.0f);
    ColorU32 |= (b << 0);

    return ColorU32;
}

u32
GetMaxValueIndex(u32 *NumsArray, u32 NumsCount)
{
    u32 MaxIndex = 0;
    u32 Max = 0;
    for(u32 Index = 0;
        Index < NumsCount;
        ++Index)
    {
        u32 Num = *(NumsArray + Index);
        if (Num > Max)
        {
            Max = Num;
            MaxIndex = Index;
        }
    }

    return MaxIndex;
}

u32
LogBase2(u32 Num)
{
    f32 FNum = (f32)Num;
    u32 Result = 0;
    while (FNum > 1.0f)
    {
        FNum /= 2;
        ++Result;
    }
    return Result;
}

u32
StringLength(const char *A)
{
    u32 Count = 0;
    const char *Ptr = A;
    while (*Ptr++ != '\0') {
        ++Count;
    }
    return Count;
}

void
StringConcat(const char *A, const char *B, char *Out)
{
    u32 Index = 0;
    while(A[Index] != '\0')
    {
        Out[Index] = A[Index];
        ++Index;
    }

    u32 sIndex = 0;
    while(B[sIndex] != '\0')
    {
        Out[Index++] = B[sIndex++];
    }

    Out[Index] = '\0';
}

void
StringConcat(char *A, const char *B)
{
    u32 Index = 0;

    u32 sIndex = 0;
    while(B[sIndex] != '\0')
    {
        A[Index++] = B[sIndex++];
    }
}

void
StringNCopy(const char *Src, char *Dst, i32 Num)
{
    i32 Index = 0;
    for(; Index < Num; ++Index)
    {
        Dst[Index] = Src[Index];
    }

    Dst[Index] = '\0';
}

void
StringCopy(const char *Src, char *Dst)
{
    u32 Index = 0;
    while(Src[Index] != '\0')
    {
        Dst[Index] = Src[Index];
        Index++;
    }
    Dst[Index] = '\0';
}

b32
StringsEqual(const char *A, const char *B)
{
    u32 LenA = StringLength(A);
    u32 LenB = StringLength(B);
    if(LenA != LenB)
        return false;

    for(u32 Index = 0; Index < LenA; ++Index)
    {
        if(A[Index] != B[Index])
        {
            return false;
        }
    }

    return true;
}

u32
StringFindLastOf(const char *String, char Separator)
{
    u32 StrLen = StringLength(String);

    while(String[StrLen] != Separator)
    {
        --StrLen;
    }

    return StrLen;
}

void
StringSubString(const char *SrcString, u32 StartIndex, u32 EndIndex, char *OutString)
{
    for(u32 Index = StartIndex; Index <= EndIndex; ++Index)
    {
        OutString[Index - StartIndex] = SrcString[Index];
    }

    OutString[EndIndex - StartIndex + 1] = '\0';
}

b32
IsPow2(size_t Num)
{
    ASSERT(Num > 0);

    if(Num == 1) return true;

    size_t Accum = 2;
    while(Accum < Num)
    {
        Accum *= 2;
    }

    return Accum == Num;
}

size_t
GetNextPow2(size_t Num)
{
    ASSERT(Num > 0);

    size_t Result = Num-1;

    for(;;)
    {
        if(IsPow2(++Result))
        {
            break;
        }
    }

    return Result;
}

#if _SHU_DEBUG
void
IsPow2Test(size_t MaxRange)
{
    LogInfo("Powers of 2 between 1 and %zu are: \n", MaxRange);
    for (size_t i = 1; i < MaxRange; ++i)
    {
        if (IsPow2(i))
        {
            LogDebug("%zu, ", i);
        }
    }
    LogInfoUnformatted("\n");
}
#endif