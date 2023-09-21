#include "math_quaternion.h"
#include "math_trig.h"

namespace Shu
{
    quat
    Quat(f32 w, f32 vx, f32 vy, f32 vz)
    {
        quat Result;

        Result.w = w;
        Result.vx = vx;
        Result.vy = vy;
        Result.vz = vz;

        return Result;
    }

    quat
    Quat(f32 Real, vec3f Complex)
    {
        quat Result;
        Result.real = Real;
        Result.complex = Complex;

        return Result;
    }

    quat
    QuatAngleAxis(f32 AngleInDegrees, vec3f Axis)
    {
        quat Result;

        Result.real = Shu::Cos(AngleInDegrees*0.5f);

        Axis = Normalize(Axis);
        Result.complex = Axis*Shu::Sin(AngleInDegrees*0.5f);

        return Result;
    }

    quat
    operator-(const quat &A, const quat &B)
    {
        quat BInv = QuatConjugate(QuatNormalize(B));

        quat Result = QuatNormalize(A) * BInv;
        return Result;
    }

    quat
    operator+(const quat &A, const quat &B)
    {
        quat Result;

        Result.w = A.w + B.w;
        Result.vx = A.vx + B.vx;
        Result.vy = A.vy + B.vy;
        Result.vz = A.vz + B.vz;

        return Result;
    }

    // A = 3 + i − 2j + k,
    // B = 2 − i + 2j + 3k.
    // Result = 8 − 9i − 2j + 11k.
    quat
    operator*(const quat &A, const quat &B)
    {
        quat Result;

        vec3f P = Shu::Vec3f(A.vx, A.vy, A.vz);
        vec3f Q = Shu::Vec3f(B.vx, B.vy, B.vz);

        f32 Dot = Shu::Dot(P, Q);
        vec3f Cross = Shu::Cross(P, Q);

        Result.w = A.w * B.w - Dot;
        vec3f T = Q*A.w + P*B.w + Shu::Cross(P, Q);

        Result.vx = T.x;
        Result.vy = T.y;
        Result.vz = T.z;

        return Result;
    }

    quat
    operator*(const f32 S, const quat &A)
    {
        quat Result = A;

        Result.real *= S;
        Result.complex *= S;

        return Result;
    }

    quat
    QuatDifference(const quat &A, const quat &B)
    {
        quat Result = A - B;
        return Result;
    }


    quat
    QuatLog(const quat &A)
    {
        f32 Angle = A.AngleRadians();
        vec3f Axis = A.AxisNormalized()*Angle;

        quat Result = Quat(0, Axis.x, Axis.y, Axis.z);
        return Result;
    }

    quat
    QuatExp(const quat &A)
    {
        ASSERT(A.real == 0.0f);

        vec3f AxisNormalized = Normalize(A.complex);
        f32 Angle = A.complex.Magnitude();

        quat Result;
        Result.real = Cos(Angle * RAD_TO_DEG);
        Result.complex = AxisNormalized * Sin(Angle * RAD_TO_DEG);

        return Result;
    }

    quat
    QuatConjugate(const quat &A)
    {
        quat Result;

        Result.w = A.w;
        Result.vx = -A.vx;
        Result.vy = -A.vy;
        Result.vz = -A.vz;

        return Result;
    }

    quat
    QuatInverse(quat &A)
    {
        if (QuatSqMagnitude(A) != 1.0f)
        {
            QuatNormalize(A);
        }

        quat Result = QuatConjugate(A);

        return Result;
    }

    f32
    QuatDot(const quat &A, const quat &B)
    {
        f32 Result = A.w*B.w + A.vx*B.vx + A.vy*B.vy + A.vz*B.vz;
        return Result;
    }

    f32
    QuatAngleBetweenRadians(const quat &A, const quat &B)
    {
        f32 Dot = QuatDot(A, B);

        f32 Result = CosInverse(Dot);
        return Result;
    }

    f32
    QuatAngleBetweenDegrees(const quat &A, const quat &B)
    {
        f32 Dot = QuatDot(A, B);

        f32 Result = CosInverse(Dot)*RAD_TO_DEG;
        return Result;
    }

    void
    QuatNormalize(quat &A)
    {
        f32 Mag = QuatMagnitude(A);

        A.w /= Mag;
        A.vx /= Mag;
        A.vy /= Mag;
        A.vz /= Mag;
    }

    quat
    QuatNormalize(const quat &A)
    {
        f32 Mag = QuatMagnitude(A);

        quat Result = A;
        Result.w /= Mag;
        Result.vx /= Mag;
        Result.vy /= Mag;
        Result.vz /= Mag;

        return Result;
    }

    f32
    QuatSqMagnitude(const quat &A)
    {
        f32 Result;
        Result = A.w*A.w + A.vx*A.vx + A.vy*A.vy + A.vz*A.vz;
        return Result;
    }

    f32
    QuatMagnitude(const quat &A)
    {
        f32 Result;

        Result = sqrtf(QuatSqMagnitude(A));

        return Result;
    }

    f32 quat::
    AngleDegrees() const
    {
        f32 Result = CosInverse(this->real)*RAD_TO_DEG;

        return Result;
    }

    f32 quat::
    AngleRadians() const
    {
        f32 Result = CosInverse(this->real);
        return Result;
    }

    vec3f quat::
    AxisNormalized() const
    {
        QuatNormalize(*this);

        f32 Angle = CosInverse(this->real)*RAD_TO_DEG;
        vec3f Result = this->complex / Sin(Angle);

        return Result;
    }

    vec3f
    QuatRotateVec(const quat &Q, const vec3f &V)
    {
        f32 Mag = QuatMagnitude(Q);

        quat InvQ = QuatConjugate(Q);
        quat qVec = Quat(0.0f, V.x, V.y, V.z);
        quat qProduct = (Q*qVec)*InvQ;

        vec3f Result = qProduct.complex;
        return Result;
    }

    vec3f
    QuatRotateVec(f32 AngleInDegrees, const vec3f &Axis, const vec3f &V)
    {
        quat Q = Quat(AngleInDegrees*0.5f, Axis);

        vec3f Result = QuatRotateVec(Q, V);
        return Result;
    }

    quat
    QuatSlerp(quat A, quat B, f32 T)
    {
        quat Result;

        // NOTE: Cos of the angle between A and B
        f32 CosOmega = QuatDot(A, B);

        // NOTE: Take the shorter arc
        if(CosOmega < 0.0f)
        {
            A.w = -A.w;
            A.vx = -A.vx;
            A.vy = -A.vy;
            A.vz = -A.vz;
            CosOmega = -CosOmega;
        }

        // NOTE: Check if they are very close together
        f32 K0, K1;
        if(CosOmega > 0.9999f)
        {
            K0 = 1.0f - T;
            K1 = T;
        }
        else
        {
            f32 SinOmega = sqrtf(1.0f - CosOmega*CosOmega);
            f32 Omega = atan2f(SinOmega, CosOmega);

            f32 OneOverSinOmega = 1.0f / SinOmega;

            K0 = sinf((1.0f - T) * Omega) * OneOverSinOmega;
            K1 = sinf(T * Omega) * OneOverSinOmega;
        }

        Result.w = A.w*K0 + B.w*K1;
        Result.vx = A.vx*K0 + B.vx*K1;
        Result.vy = A.vy*K0 + B.vy*K1;
        Result.vz = A.vz*K0 + B.vz*K1;

        return Result;
    }
}