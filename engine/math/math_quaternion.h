#if !defined(MATH_QUATERNION_H)
#define MATH_QUATERNION_H

#include "defines.h"
#include "math_vector.h"

namespace Shu
{
    struct quat
    {
        union
        {
            struct
            {
                f32 real;
                vec3f complex;
            };
            struct
            {
                f32 w, vx, vy, vz;
            };
            struct
            {
                vec4f Quat;
            };
            f32 E[4];
        };

        f32 AngleDegrees() const;
        f32 AngleRadians() const;
        vec3f AxisNormalized() const;
        vec3f ToEuler() const;
    };

    SHU_EXPORT quat Quat(f32 w, f32 vx, f32 vy, f32 vz);
    SHU_EXPORT quat Quat(f32 Real, vec3f Complex);
    SHU_EXPORT quat QuatAngleAxis(f32 AngleInDegrees, vec3f Axis);
    SHU_EXPORT quat operator-(const quat &A, const quat &B);
    SHU_EXPORT quat operator+(const quat &A, const quat &B);
    SHU_EXPORT quat operator*(const quat &A, const quat &B);
    SHU_EXPORT quat operator*(const f32 S, const quat &A);
    SHU_EXPORT quat QuatDifference(const quat &A, const quat &B);
    SHU_EXPORT quat QuatConjugate(const quat &A);
    SHU_EXPORT quat QuatInverse(quat &A);
    SHU_EXPORT f32 QuatAngleBetweenRadians(const quat &A, const quat &B);
    SHU_EXPORT f32 QuatAngleBetweenDegrees(const quat &A, const quat &B);
    SHU_EXPORT void QuatNormalize(quat &A);
    SHU_EXPORT quat QuatNormalize(const quat &A);
    SHU_EXPORT f32 QuatSqMagnitude(const quat &A);
    SHU_EXPORT f32 QuatMagnitude(const quat &A);

    SHU_EXPORT vec3f QuatRotateVec(f32 AngleInDegrees, const vec3f &Axis, const vec3f &V);
    SHU_EXPORT vec3f QuatRotateVec(const quat &Q, const vec3f &V);
    SHU_EXPORT quat QuatSlerp(quat A, quat B, f32 T);
    SHU_EXPORT quat QuatFromEuler(f32 xDegrees, f32 yDegrees, f32 zDegrees);
}

#endif