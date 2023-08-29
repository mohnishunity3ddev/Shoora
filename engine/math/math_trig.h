#if !defined(MATH_TRIG_H)
#include <cmath>

#include "defines.h"
#define SHU_PI 3.14159265359f
#define SHU_PI_BY_2 1.57079632679f
#define DEG_TO_RAD SHU_PI / 180.0f
#define RAD_TO_DEG 180.0f / SHU_PI

namespace Shu
{
    inline f32
    Sin(f32 Degrees)
    {
        f32 Result = sinf(Degrees*DEG_TO_RAD);
        return Result;
    }

    inline f32
    Cos(f32 Degrees)
    {
        f32 Result = cosf(Degrees*DEG_TO_RAD);
        if(abs(Result) < 0.001f)
        {
            Result = 0;
        }
        return Result;
    }

    inline f32
    Tan(f32 Degrees)
    {
        f32 Result = tanf(Degrees*DEG_TO_RAD);
        return Result;
    }

    inline f32
    Cosec(f32 Degrees)
    {
        f32 Result = 1.0f / Sin(Degrees);
        return Result;
    }

    inline f32
    Sec(f32 Degrees)
    {
        f32 Result = 1.0f / Cos(Degrees);
        return Result;
    }

    inline f32
    Cot(f32 Degrees)
    {
        f32 Result = 1.0f / Tan(Degrees);
        return Result;
    }
}


#define MATH_TRIG_H
#endif