#if !defined(MATH_QUATERNION_H)
#define MATH_QUATERNION_H

#include "defines.h"
#include "math_vector.h"
#include "math_matrix.h"

namespace shu
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

        quat() : w(1), vx(0), vy(0), vz(0) {}

        f32 AngleDegrees() const;
        f32 AngleRadians() const;
        vec3f AxisNormalized() const;
        vec3f ToEuler() const;
        mat3f ToMat3f() const;
        mat4f ToMat4f() const;
        vec4f ToVec4f() const;
        mat4f LeftOp() const;
        mat4f RightOp() const;
        inline b32 operator==(const quat &rhs) const;

        shu::quat Inverse() const;

        static quat Identity()
        {
            quat Result;
            Result.Quat = shu::Vec4f(1, 0, 0, 0);
            return Result;
        }
    };

    b32
    quat::operator==(const quat &rhs) const
    {
        if (!NearlyEqual(this->w,  rhs.w,  1e-4)) return false;
        if (!NearlyEqual(this->vx, rhs.vx, 1e-4)) return false;
        if (!NearlyEqual(this->vy, rhs.vy, 1e-4)) return false;
        if (!NearlyEqual(this->vz, rhs.vz, 1e-4)) return false;

        return true;
    }

    SHU_EXPORT quat Quat();
    SHU_EXPORT quat Quat(f32 w, f32 vx, f32 vy, f32 vz);
    SHU_EXPORT quat Quat(f32 Real, vec3f Complex);
    SHU_EXPORT quat QuatAngleAxisDeg(f32 AngleInDegrees, const vec3f &Axis);
    SHU_EXPORT quat QuatAngleAxisRad(f32 AngleInRadians, const vec3f &Axis);
    SHU_EXPORT quat QuatIdentity();
    SHU_EXPORT quat operator-(const quat &A, const quat &B);
    SHU_EXPORT quat operator+(const quat &A, const quat &B);
    SHU_EXPORT quat operator*(const quat &A, const quat &B);
    SHU_EXPORT quat operator*(const f32 S, const quat &A);
    SHU_EXPORT quat QuatDifference(const quat &A, const quat &B);
    SHU_EXPORT quat QuatConjugate(const quat &A);
    SHU_EXPORT quat QuatInverse(const quat &A);
    SHU_EXPORT f32 QuatAngleBetweenRadians(const quat &A, const quat &B);
    SHU_EXPORT void DecomposeSwingTwist(const quat &Q, const vec3f TwistAxis, quat &SwingQuat, quat &TwistQuat);
    SHU_EXPORT f32 QuatAngleBetweenDegrees(const quat &A, const quat &B);
    SHU_EXPORT void QuatNormalize(quat &A);
    SHU_EXPORT quat QuatNormalize(const quat &A);
    SHU_EXPORT f32 QuatSqMagnitude(const quat &A);
    SHU_EXPORT f32 QuatMagnitude(const quat &A);

    SHU_EXPORT quat QuatFromToRotation(const vec3f &v1, const vec3f &v2);
    SHU_EXPORT void DecomposeSwingTwist(const quat &q1, const quat &q2, const vec3f &localN1, const vec3f &localN2,
                                        quat &swingQuat, quat &twistQuat);
    SHU_EXPORT void DecomposeSwingTwist(const quat &relativeQuat, const vec3f twistAxis_WS, quat &swingQuat,
                                        quat &twistQuat);

    SHU_EXPORT vec3f QuatRotateVec(f32 AngleInDegrees, const vec3f &Axis, const vec3f &V);
    SHU_EXPORT vec3f QuatRotateVec(const quat &Q, const vec3f &V);
    SHU_EXPORT quat QuatSlerp(quat A, quat B, f32 T);
    SHU_EXPORT quat QuatFromEuler(f32 xDegrees, f32 yDegrees, f32 zDegrees);
    } // namespace shu

#endif