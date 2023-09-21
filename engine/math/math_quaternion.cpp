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
        f32 Result = 2.0f*(CosInverse(this->real)*RAD_TO_DEG);
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
        T = ClampToRange(T, 0.0f, 1.0f);
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

    vec3f quat::
    ToEuler() const
    {
        f32 xAngle, yAngle, zAngle;

        f32 SinX = -2.0f*(vy*vz - w*vx);

        // NOTE: Check for Gimbal Lock
        if(fabsf(SinX) > .9999f)
        {
            // NOTE: Looking straight up or down
            xAngle = SHU_PI_BY_2 * SinX;

            yAngle = atan2f(-vx*vz + w*vy, .5f - vy*vy - vz*vz);
            zAngle = 0.0f;
        }
        else
        {
            xAngle = asinf(SinX);
            yAngle = atan2f(vx*vz + w*vy, .5f - vx*vx - vy*vy);
            zAngle = atan2f(vx*vy + w*vz, .5f - vx*vx - vz*vz);
        }

        vec3f Euler = Shu::Vec3f(xAngle, yAngle, zAngle) * RAD_TO_DEG;
        return Euler;
    }

    // Returns rotation in YXZ sequence
    quat
    QuatFromEuler(f32 xDegrees, f32 yDegrees, f32 zDegrees)
    {
        Shu::vec3f HalfAngles = Shu::Vec3f(xDegrees*0.5f, yDegrees*0.5f, zDegrees*0.5f);

        f32 CosXBy2 = Cos(HalfAngles.x); f32 SinXBy2 = Sin(HalfAngles.x);
        f32 CosYBy2 = Cos(HalfAngles.y); f32 SinYBy2 = Sin(HalfAngles.y);
        f32 CosZBy2 = Cos(HalfAngles.z); f32 SinZBy2 = Sin(HalfAngles.z);

        quat Result;

        Result.w  = CosYBy2*CosXBy2*CosZBy2 + SinYBy2*SinXBy2*SinZBy2;
        Result.vx = CosYBy2*SinXBy2*CosZBy2 + SinYBy2*CosXBy2*SinZBy2;
        Result.vy = SinYBy2*CosXBy2*CosZBy2 - CosYBy2*SinXBy2*SinZBy2;
        Result.vz = CosYBy2*CosXBy2*SinZBy2 - SinYBy2*SinXBy2*CosZBy2;

        return Result;
    }

    void
    QuaternionTest()
    {
        // A = 3 + i − 2j + k,
        // B = 2 − i + 2j + 3k.
        // Result = 8 − 9i − 2j + 11k.
        Shu::quat A = Shu::Quat(1, 2, 3, 4);
        Shu::quat B = Shu::Quat(2, 3, 4, 5);
        Shu::quat R = Shu::QuatConjugate(B);
        Shu::QuatNormalize(A);
        f32 AMag = Shu::QuatMagnitude(A);
        f32 AMag2 = Shu::QuatSqMagnitude(A);

        // Shu::vec3f P = Shu::Vec3f(0.7f, -0.3f, 0.649f);
        Shu::vec3f P = Shu::Vec3f(1, 0, 0);
        Shu::vec3f Axis = Shu::Normalize(Shu::Vec3f(0.235f, 0.235f, -0.942f));
        Shu::vec3f PT = Shu::QuatRotateVec(54.74f, Axis, P);

        Shu::quat Quat = Shu::QuatAngleAxis(54.74f, Shu::Vec3f(0.235f, 0.235f, -0.942f));
        f32 AngleInDegrees = Quat.AngleDegrees();
        Shu::vec3f At = Quat.AxisNormalized();

        Shu::quat SQ0 = Shu::QuatAngleAxis(90, Shu::Vec3f( 0, 0, 1));
        Shu::quat SQ1 = Shu::QuatAngleAxis(90, Shu::Vec3f( 0, 1, 0));
        Shu::quat SQ01 = Shu::QuatSlerp(SQ0, SQ1, 0.5f);
        Shu::vec3f sqEuler0 = SQ0.ToEuler();
        Shu::vec3f sqEuler1 = SQ1.ToEuler();
        Shu::vec3f SlerpedAxis = SQ01.AxisNormalized();
        f32 SlerpedAngle = SQ01.AngleDegrees();
        Shu::vec3f RotatedVec0 = Shu::QuatRotateVec(SQ01, Shu::Vec3f(1, 0, 0));

#if 0
        Shu::mat4f Identity = Shu::Mat4f(1.0f);
        Shu::mat4f QuatMat0 = Shu::GetRotationMatrix(Identity, SQ01);
        Identity = Shu::Mat4f(1.0f);
        Shu::mat4f QuatMat1 = Shu::RotateGimbalLock(Identity, SlerpedAxis, SlerpedAngle);
#endif

#if 1
        Shu::vec3f RotatedVec3s[20];
        Shu::quat SlerpedQuats[20];
        f32 SlerpedQuatAngles[20];
        Shu::vec3f SlerpedQuatAxiss[20];

        u32 TestQuatCount = ARRAY_SIZE(SlerpedQuats);
        for(u32 Index = 0;
            Index < TestQuatCount;
            ++Index)
        {
            f32 T = (f32)(Index + 1) / (f32)TestQuatCount;
            Shu::quat Slerped = Shu::QuatSlerp(SQ0, SQ1, T);
            SlerpedQuats[Index] = Slerped;
            Shu::vec3f R0 = Shu::QuatRotateVec(Slerped, Shu::Vec3f(1, 0, 0));
            RotatedVec3s[Index] = R0;
            SlerpedQuatAngles[Index] = Slerped.AngleDegrees();
            SlerpedQuatAxiss[Index] = Slerped.AxisNormalized();
        }

        Shu::quat qX = Shu::QuatAngleAxis(30, Shu::Vec3f(1, 0, 0)); // X-Axis
        Shu::quat qY = Shu::QuatAngleAxis(30, Shu::Vec3f(0, 1, 0)); // Y-Axis
        Shu::quat qZ = Shu::QuatAngleAxis(30, Shu::Vec3f(0, 0, 1)); // Z-Axis
        // NOTE: How Unity rotates a point. First around Y then X then Z
        Shu::quat FinalQuat = qY*qX*qZ;
        Shu::quat FinalQuat1 = Shu::QuatFromEuler(30, 30, 30);
        Shu::vec3f RotateVec = Shu::QuatRotateVec(FinalQuat, Shu::Vec3f(1, 0, 0));
        Shu::vec3f Euler = FinalQuat.ToEuler();
        f32 FinalAngle = FinalQuat.AngleDegrees();
        Shu::vec3f FinalAxis = FinalQuat.AxisNormalized();
#endif
    }
}