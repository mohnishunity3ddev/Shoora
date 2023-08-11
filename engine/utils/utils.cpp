#include "utils.h"

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