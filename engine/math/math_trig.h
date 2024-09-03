#if !defined(MATH_TRIG_H)
#include <cmath>

#include "defines.h"

namespace shu
{
    inline f32
    Sin(f32 Degrees)
    {
        f32 Result = sinf(Degrees*DEG_TO_RAD);
        return Result;
    }

    inline f32
    SinRad(f32 Radians)
    {
        f32 Result = sinf(Radians);
        return Result;
    }

    inline f32
    CosDeg(f32 Degrees)
    {
        f32 Result = cosf(Degrees*DEG_TO_RAD);
        if(abs(Result) < 0.001f)
        {
            Result = 0;
        }
        return Result;
    }

    inline f32
    CosRad(f32 Radians)
    {
        f32 Result = cosf(Radians);
        if(abs(Result) < 0.001f)
        {
            Result = 0;
        }
        return Result;
    }

    inline f32
    CosInverse(f32 Value)
    {
        f32 Result = acosf(Value);
        return Result;
    }

    inline f32
    SinInverse(f32 Value)
    {
        f32 Result = asinf(Value);
        return Result;
    }

    inline f32 TanInverse(f32 y, f32 x) {
        f32 Result = atan2f(y, x);
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
        f32 Result = 1.0f / CosDeg(Degrees);
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