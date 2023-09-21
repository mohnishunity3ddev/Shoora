#if !defined(MATH_MATRIX_TRANSFORMS_H)

#include "defines.h"
#include "math_vector.h"
#include "math_matrix.h"
#include "math_quaternion.h"

namespace Shu
{
    template <typename T> SHU_EXPORT mat4<T> Translate(mat4<T> &Mat, const vec3<T> &Tv);
    template <typename T> SHU_EXPORT mat4<T> Scale(mat4<T> &Mat, const vec3<T> &Sv);

    SHU_EXPORT mat4f RotateGimbalLock(mat4f &Mat, const vec3f &Axis, f32 AngleInDegrees);
    SHU_EXPORT mat4f GetRotationMatrix(mat4f &Mat, const quat &Q);
    SHU_EXPORT mat4f LookAt(const vec3f &CamPos, const vec3f &LookingTowards, const vec3f WorldUp, mat4f &M);
    SHU_EXPORT mat4f Perspective(f32 FOVy, f32 Aspect, f32 ZNear, f32 ZFar);

    template <typename T>
    mat4<T>
    Translate(mat4<T> &Mat, const vec3<T> &Tv)
    {
        Mat.m30 = Tv.x;
        Mat.m31 = Tv.y;
        Mat.m32 = Tv.z;

        // Mat = Transpose(Mat);

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