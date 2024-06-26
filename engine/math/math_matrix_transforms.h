#if !defined(MATH_MATRIX_TRANSFORMS_H)

#include "defines.h"
#include "math_vector.h"
#include "math_matrix.h"
#include "math_quaternion.h"

namespace shu
{
    template <typename T> SHU_EXPORT mat4<T> Translate(mat4<T> &Mat, const vec3<T> &Tv);
    template <typename T> SHU_EXPORT mat4<T> Scale(mat4<T> &Mat, const vec3<T> &Sv);

    SHU_EXPORT mat4f TRSInverse(const shu::vec3f &Pos, const shu::vec3f &Scale,
                                const f32 RotationAngleDegrees, const shu::vec3f &RotationAxis);
    SHU_EXPORT mat4f TRS(const shu::vec3f &Pos, const shu::vec3f &Scale, const f32 RotationAngle,
                         const shu::vec3f &RotationAxis);
    SHU_EXPORT mat4f TRS(const shu::vec3f &Pos, const shu::vec3f &Scale, const shu::quat Rotation);
    SHU_EXPORT mat4f RotateGimbalLock(mat4f &Mat, const vec3f &Axis, f32 AngleInDegrees);
    SHU_EXPORT mat4f Rotate(mat4f &Mat, const quat &Q);
    SHU_EXPORT mat3f QuatRotationMatrix_Left(const quat &Q);

    SHU_EXPORT mat4f LookAt(const vec3f &CamPos, const vec3f &LookingTowards, const vec3f WorldUp, mat4f &M);
    SHU_EXPORT mat4f Perspective(f32 FOVy, f32 Aspect, f32 ZNear, f32 ZFar);
    SHU_EXPORT mat4f Orthographic(f32 Width, f32 Height, f32 Near, f32 Far);

    template <typename T>
    mat4<T>
    Translate(mat4<T> &Mat, const vec3<T> &Tv)
    {
        // We are following row major form. Where vectors takes as rows are PRE-Multiplied by the matrix. That's
        // why the translation values in x,y,z and written in the rwos of the matrix.
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