#include "utils.h"

Shu::vec3f
GetColor(u32 Col)
{
    f32 m = 1.0f / 255.0f;

    u32 r = (Col & 0x00ff0000) >> 16;
    u32 g = (Col & 0x0000ff00) >> 8;
    u32 b = (Col & 0x000000ff) >> 0;

    Shu::vec3f Result = Shu::Vec3f(r, g, b) * m;
    return Result;
}

u32
GetColorU32(const Shu::vec3f &Color)
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
