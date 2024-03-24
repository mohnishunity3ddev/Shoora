#include "math_quaternion.h"
#include "math_trig.h"

namespace shu
{
    quat
    Quat()
    {
        quat Result;

        Result.w = 1;
        Result.vx = 0;
        Result.vy = 0;
        Result.vz = 0;

        return Result;
    }

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
    QuatAngleAxisDeg(f32 AngleInDegrees, const vec3f &Axis)
    {
        quat Result;

        Result.real = shu::Cos(AngleInDegrees*0.5f);

        shu::vec3f axis = Normalize(Axis);
        Result.complex = axis*shu::Sin(AngleInDegrees*0.5f);

        return Result;
    }

    quat
    QuatIdentity()
    {
        return quat::Identity();
    }

    quat
    QuatAngleAxisRad(f32 AngleInRadians, const vec3f &Axis)
    {
        quat Result;

        Result.real = shu::CosRad(AngleInRadians*0.5f);

        shu::vec3f axis = Normalize(Axis);
        Result.complex = axis*shu::SinRad(AngleInRadians*0.5f);

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

        vec3f P = shu::Vec3f(A.vx, A.vy, A.vz);
        vec3f Q = shu::Vec3f(B.vx, B.vy, B.vz);

        f32 Dot = shu::Dot(P, Q);
        vec3f Cross = shu::Cross(P, Q);

        Result.w = A.w * B.w - Dot;
        vec3f T = Q*A.w + P*B.w + shu::Cross(P, Q);

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
    QuatInverse(const quat &A)
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

        vec3f Euler = shu::Vec3f(xAngle, yAngle, zAngle) * RAD_TO_DEG;
        return Euler;
    }

    mat3f
    quat::ToMat3f() const
    {
        quat Quat = QuatNormalize(*this);
        f32 w = this->real; // cos(theta / 2)
        f32 x = this->complex.x; // nx*sin(theta / 2)
        f32 y = this->complex.y; // ny*sin(theta / 2)
        f32 z = this->complex.z; // nz*sin(theta / 2)

        shu::vec3f Row0 = shu::Vec3f(1.0f - 2*y*y - 2*z*z,    2*x*y + 2*w*z,            2*x*z - 2*w*y);
        shu::vec3f Row1 = shu::Vec3f(2*x*y - 2*w*z,           1.0f - 2*x*x - 2*z*z,     2*y*z + 2*w*x);
        shu::vec3f Row2 = shu::Vec3f(2*x*z + 2*w*y,           2*y*z - 2*w*x,            1.0f - 2*x*x - 2*y*y);

        shu::mat3f Result = shu::Mat3f(Row0, Row1, Row2);
        return Result;
    }

    mat4f
    quat::ToMat4f() const
    {
        quat Quat = QuatNormalize(*this);
        f32 w = this->real; // cos(theta / 2)
        f32 x = this->complex.x; // nx*sin(theta / 2)
        f32 y = this->complex.y; // ny*sin(theta / 2)
        f32 z = this->complex.z; // nz*sin(theta / 2)

        shu::vec4f Row0 = shu::Vec4f(1.0f - 2.0f*y*y - 2.0f*z*z,    2.0f*x*y + 2.0f*w*z,            2.0f*x*z - 2.0f*w*y,            0.0f);
        shu::vec4f Row1 = shu::Vec4f(2.0f*x*y - 2.0f*w*z,           1.0f - 2.0f*x*x - 2.0f*z*z,     2.0f*y*z + 2.0f*w*x,            0.0f);
        shu::vec4f Row2 = shu::Vec4f(2.0f*x*z + 2.0f*w*y,           2.0f*y*z - 2.0f*w*x,            1.0f - 2.0f*x*x - 2.0f*y*y,     0.0f);
        shu::vec4f Row3 = shu::Vec4f(0.0f,                          0.0f,                           0.0f,                           1.0f);

        shu::mat4f Result = shu::Mat4f(Row0, Row1, Row2, Row3);
        return Result;
    }

    // Returns rotation in YXZ sequence
    quat
    QuatFromEuler(f32 xDegrees, f32 yDegrees, f32 zDegrees)
    {
        shu::vec3f HalfAngles = shu::Vec3f(xDegrees*0.5f, yDegrees*0.5f, zDegrees*0.5f);

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
        shu::quat A = shu::Quat(1, 2, 3, 4);
        shu::quat B = shu::Quat(2, 3, 4, 5);
        shu::quat R = shu::QuatConjugate(B);
        shu::QuatNormalize(A);
        f32 AMag = shu::QuatMagnitude(A);
        f32 AMag2 = shu::QuatSqMagnitude(A);

        // Shu::vec3f P = Shu::Vec3f(0.7f, -0.3f, 0.649f);
        shu::vec3f P = shu::Vec3f(1, 0, 0);
        shu::vec3f Axis = shu::Normalize(shu::Vec3f(0.235f, 0.235f, -0.942f));
        shu::vec3f PT = shu::QuatRotateVec(54.74f, Axis, P);

        shu::quat Quat = shu::QuatAngleAxisDeg(54.74f, shu::Vec3f(0.235f, 0.235f, -0.942f));
        f32 AngleInDegrees = Quat.AngleDegrees();
        shu::vec3f At = Quat.AxisNormalized();

        shu::quat SQ0 = shu::QuatAngleAxisDeg(90, shu::Vec3f( 0, 0, 1));
        shu::quat SQ1 = shu::QuatAngleAxisDeg(90, shu::Vec3f( 0, 1, 0));
        shu::quat SQ01 = shu::QuatSlerp(SQ0, SQ1, 0.5f);
        shu::vec3f sqEuler0 = SQ0.ToEuler();
        shu::vec3f sqEuler1 = SQ1.ToEuler();
        shu::vec3f SlerpedAxis = SQ01.AxisNormalized();
        f32 SlerpedAngle = SQ01.AngleDegrees();
        shu::vec3f RotatedVec0 = shu::QuatRotateVec(SQ01, shu::Vec3f(1, 0, 0));

#if 0
        Shu::mat4f Identity = Shu::Mat4f(1.0f);
        Shu::mat4f QuatMat0 = Shu::GetRotationMatrix(Identity, SQ01);
        Identity = Shu::Mat4f(1.0f);
        Shu::mat4f QuatMat1 = Shu::RotateGimbalLock(Identity, SlerpedAxis, SlerpedAngle);
#endif

#if 1
        shu::vec3f RotatedVec3s[20];
        shu::quat SlerpedQuats[20];
        f32 SlerpedQuatAngles[20];
        shu::vec3f SlerpedQuatAxiss[20];

        u32 TestQuatCount = ARRAY_SIZE(SlerpedQuats);
        for(u32 Index = 0;
            Index < TestQuatCount;
            ++Index)
        {
            f32 T = (f32)(Index + 1) / (f32)TestQuatCount;
            shu::quat Slerped = shu::QuatSlerp(SQ0, SQ1, T);
            SlerpedQuats[Index] = Slerped;
            shu::vec3f R0 = shu::QuatRotateVec(Slerped, shu::Vec3f(1, 0, 0));
            RotatedVec3s[Index] = R0;
            SlerpedQuatAngles[Index] = Slerped.AngleDegrees();
            SlerpedQuatAxiss[Index] = Slerped.AxisNormalized();
        }

        shu::quat qX = shu::QuatAngleAxisDeg(30, shu::Vec3f(1, 0, 0)); // X-Axis
        shu::quat qY = shu::QuatAngleAxisDeg(30, shu::Vec3f(0, 1, 0)); // Y-Axis
        shu::quat qZ = shu::QuatAngleAxisDeg(30, shu::Vec3f(0, 0, 1)); // Z-Axis
        // NOTE: How Unity rotates a point. First around Y then X then Z
        shu::quat FinalQuat = qY*qX*qZ;
        shu::quat FinalQuat1 = shu::QuatFromEuler(30, 30, 30);
        shu::vec3f RotateVec = shu::QuatRotateVec(FinalQuat, shu::Vec3f(1, 0, 0));
        shu::vec3f Euler = FinalQuat.ToEuler();
        f32 FinalAngle = FinalQuat.AngleDegrees();
        shu::vec3f FinalAxis = FinalQuat.AxisNormalized();
#endif
    }
}