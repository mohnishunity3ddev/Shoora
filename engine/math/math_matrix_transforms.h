#if !defined(MATH_MATRIX_TRANSFORMS_H)

#include "defines.h"
#include "math_vector.h"
#include "math_matrix.h"

namespace Shu
{
    template <typename T> SHU_EXPORT mat4<T> Translate(mat4<T> &Mat, const vec3<T> &Tv);
    template <typename T> SHU_EXPORT mat4<T> Scale(mat4<T> &Mat, const vec3<T> &Sv);

    SHU_EXPORT mat4f RotateGimbalLock(mat4f &Mat, const vec3f &Axis, f32 AngleInDegrees);

    template <typename T>
    mat4<T>
    Translate(mat4<T> &Mat, const vec3<T> &Tv)
    {
        Mat.m30 = Tv.x;
        Mat.m31 = Tv.y;
        Mat.m32 = Tv.z;

        return Mat;
    }

    template <typename T>
    mat4<T>
    Scale(mat4<T> &Mat, const vec3<T> &Sv)
    {
        Mat.m00 = Sv.x;
        Mat.m11 = Sv.y;
        Mat.m22 = Sv.z;

        return Mat;
    }
}

#define MATH_MATRIX_TRANSFORMS_H
#endif // MATH_MATRIX_TRANSFORMS_H