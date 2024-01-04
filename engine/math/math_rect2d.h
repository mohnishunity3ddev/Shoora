#if !defined(MATH_RECT_2D)

#include <defines.h>
#include "math_vector.h"

namespace Shu
{
    struct rect2d
    {
        union
        {
            struct
            {
                f32 x;
                f32 y;
                f32 width;
                f32 height;
            };
            struct
            {
                vec4f rect;
            };
        };
        rect2d() {}
        rect2d(f32 x, f32 y, f32 w, f32 h) : x(x), y(y), width(w), height(h) {}
    };
}

#define MATH_RECT_2D
#endif