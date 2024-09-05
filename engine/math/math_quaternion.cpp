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

        Result.real = shu::CosDeg(AngleInDegrees*0.5f);

        shu::vec3f axis = Normalize(Axis);
        Result.complex = axis*shu::SinDeg(AngleInDegrees*0.5f);

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

    // TODO: QuatLog is Incomplete!
    quat
    QuatLog(const quat &A)
    {
        f32 Angle = A.AngleRadians();
        vec3f Axis = A.AxisNormalized()*Angle;

        quat Result = Quat(0, Axis.x, Axis.y, Axis.z);
        return Result;
    }

    // TODO: QuatExp is Incomplete!
    quat
    QuatExp(const quat &A)
    {
        ASSERT(A.real == 0.0f);

        vec3f AxisNormalized = Normalize(A.complex);
        f32 Angle = A.complex.Magnitude();

        quat Result;
        Result.real = CosDeg(Angle * RAD_TO_DEG);
        Result.complex = AxisNormalized * SinDeg(Angle * RAD_TO_DEG);

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
        f32 Result = AngleRadians() * RAD_TO_DEG;
        return Result;
    }

    f32 quat::
    AngleRadians() const
    {
        shu::quat q = *this;

        f32 cosTheta = q.w;
        f32 sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

        if (NearlyEqual(cosTheta, 0.0f, 1e-5))
            return SHU_PI_BY_2;
        else if (NearlyEqual(sinTheta, 0.0f, 1e-5))
            return 0.0f;

        f32 angle = 2.0f * atan2f(sinTheta, cosTheta);
        return angle;
    }

    vec3f quat::
    AxisNormalized() const
    {
        shu::vec3f Result = shu::Normalize(this->complex);
        return Result;
    }

    void
    DecomposeSwingTwist(const quat &q1, const quat &q2, const vec3f &localN1, const vec3f &localN2,
                        quat &swingQuat, quat &twistQuat, f32 *swingAngle, f32 *twistAngle)
    {
        quat relQuat = q2 * QuatInverse(q1);

        vec3f n1 = QuatRotateVec(q1, localN1);
        vec3f n2 = QuatRotateVec(q2, localN2);

        float d = n1.Dot(n2);
        float c = 1.0f / (sqrtf(2.0f * (1.0f + d)));
        float qw = (1 + d) * c;
        vec3f qv = Cross(n1, n2) * c;

        // IMPORTANT: NOTE: First we swing, then twist
        // qt * qs = qr = q2 * q^-1
        // q2 = qr * q1
        // q2 = qt*qs * q1
        swingQuat = Quat(qw, qv.x, qv.y, qv.z);
        twistQuat = relQuat * QuatInverse(swingQuat);
        shu::QuatNormalize(twistQuat);

        if (swingAngle != nullptr) {
            *swingAngle = 2.0f * CosInverse(sqrtf(0.5f * (1 + d))) * RAD_TO_DEG;
        }
        if (twistAngle != nullptr)
        {
            ASSERT( !NearlyEqual(twistQuat.w, 0.0f, 1e-5) );
            // NOTE: n2.Dot(twistQuat.complex) will spit out sin(phi/2)
            *twistAngle = (2 * TanInverse(n2.Dot(twistQuat.complex), twistQuat.w)) * RAD_TO_DEG;
        }
    }

    void
    DecomposeSwingTwist(const quat &relativeQuat, const vec3f &twistAxis_WS, quat &swingQuat, quat &twistQuat,
                        f32 *swingAngle, f32 *twistAngle)
    {
        shu::vec3f rotatedTwistAxis_WS = shu::QuatRotateVec(relativeQuat, twistAxis_WS);

        swingQuat = QuatFromToRotation(twistAxis_WS, rotatedTwistAxis_WS);
        twistQuat = relativeQuat * QuatInverse(swingQuat);

        if (swingAngle != nullptr) {
            f32 d = twistAxis_WS.Dot(rotatedTwistAxis_WS);
            *swingAngle = 2.0f * CosInverse(sqrtf(0.5f * (1 + d))) * RAD_TO_DEG;
        }

        if (twistAngle != nullptr) {
            ASSERT( !NearlyEqual(twistQuat.w, 0.0f, 1e-5) );
            *twistAngle = (2 * TanInverse(rotatedTwistAxis_WS.Dot(twistQuat.complex), twistQuat.w)) *
                          RAD_TO_DEG;
        }
    }

    quat
    QuatFromToRotation(const vec3f &from, const vec3f &to)
    {
        quat Result;
        vec3f vec1 = from, vec2 = to;
        if (from.SqMagnitude() > 1.0f) { vec1 =  shu::Normalize(from); }
        if (to.SqMagnitude() > 1.0f)   { vec2 =  shu::Normalize(to);   }

        f32 d = vec1.Dot(vec2);
        vec3f cross = vec1.Cross(to);

        // vec1 and vec2 are parallel
        if (NearlyEqual(cross.SqMagnitude(), 0.0f, 1e-6))
        {
            if (NearlyEqual(d, 1.0f, 1e-4))
            {
                Result = QuatIdentity();
            }
            else if(NearlyEqual(d, -1.0f, 1e-4)) // 180 degree rotation
            {
                shu::vec3f rotAxis = shu::Cross(Vec3f(1, 0, 0), vec1);
                if (NearlyEqual(rotAxis.SqMagnitude(), 0.0f, 1e-4))
                {
                    rotAxis = shu::Cross(Vec3f(0, 1, 0), vec1);
                    if (NearlyEqual(rotAxis.SqMagnitude(), 0.0f, 1e-4))
                    {
                        rotAxis = shu::Cross(Vec3f(0, 0, 1), vec1);
                    }
                }

                ASSERT(!NearlyEqual(rotAxis.SqMagnitude(), 0.0f, 1e-4));
                rotAxis.Normalize();

                Result = shu::Quat(0.0f, rotAxis);
            }
        }
        else
        {
            f32 c = 1.0f / (sqrtf(2.0f*(1.0f + d)));
            f32 w = (1 + d) * c;
            vec3f v = cross * c;

            Result = shu::Quat(w, v);
        }

        return Result;
    }

    vec3f
    QuatRotateVec(const quat &Q, const vec3f &V)
    {
        shu::quat qNormalized = shu::QuatNormalize(Q);

        quat InvQ = QuatConjugate(qNormalized);
        quat qVec = Quat(0.0f, V.x, V.y, V.z);
        quat qProduct = (qNormalized*qVec)*InvQ;

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

    vec4f
    quat::ToVec4f() const
    {
        vec4f Result = shu::Vec4f(w, vx, vy, vz);
        return Result;
    }

    quat
    quat::Inverse() const
    {
        return QuatInverse(*this);
    }

    mat4f
    quat::LeftOp() const
    {
        shu::mat4f M;

        M.Row0 = shu::Vec4f(w,  -vx, -vy, -vz);
        M.Row1 = shu::Vec4f(vx,   w, -vz,  vy);
        M.Row2 = shu::Vec4f(vy,  vz,   w, -vx);
        M.Row3 = shu::Vec4f(vz, -vy,  vx,   w);
        M.Transposed();

        return M;
    }

    mat4f
    quat::RightOp() const
    {
        shu::mat4f M;

        M.Row0 = shu::Vec4f(w,  -vx, -vy, -vz);
        M.Row1 = shu::Vec4f(vx,   w,  vz, -vy);
        M.Row2 = shu::Vec4f(vy, -vz,   w,  vx);
        M.Row3 = shu::Vec4f(vz,  vy, -vx,  w);
        M.Transposed();

        return M;

    }

    // Returns rotation in YXZ sequence
    quat
    QuatFromEuler(f32 xDegrees, f32 yDegrees, f32 zDegrees)
    {
        shu::vec3f HalfAngles = shu::Vec3f(xDegrees*0.5f, yDegrees*0.5f, zDegrees*0.5f);

        f32 CosXBy2 = CosDeg(HalfAngles.x); f32 SinXBy2 = SinDeg(HalfAngles.x);
        f32 CosYBy2 = CosDeg(HalfAngles.y); f32 SinYBy2 = SinDeg(HalfAngles.y);
        f32 CosZBy2 = CosDeg(HalfAngles.z); f32 SinZBy2 = SinDeg(HalfAngles.z);

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

        // NOTE: Swing-Twist Quaternion

        // Swing Decomposition Test
        shu::quat q1 = shu::Quat(0.06148f, 0.32513f, -0.01026f, 0.94361f);
        shu::quat q2 = shu::Quat(0.11593f, 0.04829f,  0.64212f, 0.75625f);
        shu::vec3f localN1 = shu::Vec3f(1, 0, 0); // n1_worldSpace = (-0.78, 0.11, 0.61)
        shu::vec3f localN2 = shu::Vec3f(1, 0, 0); // n2_worldSpace = (-0.97, 0.24, -0.08)
        shu::quat swingQuat, twistQuat;
        f32 twistAngle, swingAngle;
        DecomposeSwingTwist(q1, q2, localN1, localN2, swingQuat, twistQuat, &swingAngle, &twistAngle);
        shu::quat expectSwing = shu::Quat(0.93159f, -0.08278f, -0.35139f, -0.04266f);
        shu::quat expectTwist = shu::Quat(0.78345f, -0.60186f,  0.14751f, -0.04713f);
        ASSERT(swingQuat == expectSwing);
        ASSERT(twistQuat == expectTwist);

        shu::vec3f twistAxis_WS = shu::Vec3f(-0.781017f, 0.1093521f, 0.6148615f);
        shu::quat relativeQuat = q2 * shu::QuatInverse(q1);
        DecomposeSwingTwist(relativeQuat, twistAxis_WS, swingQuat, twistQuat, &swingAngle, &twistAngle);
        ASSERT(swingQuat == expectSwing);
        ASSERT(twistQuat == expectTwist);

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
        shu::quat   FinalQuat   = qY*qX*qZ;
        shu::quat   FinalQuat1  = shu::QuatFromEuler(30, 30, 30);
        shu::vec3f  RotateVec   = shu::QuatRotateVec(FinalQuat, shu::Vec3f(1, 0, 0));
        shu::vec3f  Euler       = FinalQuat.ToEuler();
        f32         FinalAngle  = FinalQuat.AngleDegrees();
        shu::vec3f  FinalAxis   = FinalQuat.AxisNormalized();

        // NOTE: FromToRotation
        shu::vec3f n1 = shu::Vec3f(-0.781017f, 0.1093521f, 0.6148615f);
        shu::vec3f n2 = shu::Vec3f(-0.18811208f, 0.9740919f, 0.12553495f);
        // q = (-0.35873, -0.01080, -0.45375, 0.81567)
        shu::quat Q = shu::QuatFromToRotation(n1, n2);
#endif
    }
}