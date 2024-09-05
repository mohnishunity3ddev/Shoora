#include "constraint.h"

f32
Eta(f32 theta)
{
    f32 Result = 0.0f;
    if ( !NearlyEqual(theta, 0.0f, 1e-4) )
    {
        f32 a = shu::SinRad(theta);
        f32 b = theta * shu::CosRad(theta);
        f32 c = 1.0f / (a * a * a);
        Result = (a - b) * c;
    }
    else
    {
        Result = 1.0f / 3.0f;
    }

    return Result;
}

void
hinge_swing_twist_constraint_3d::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();

    const shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f pA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f pB = r2 - this->B->GetCenterOfMassWS();
    shu::mat3f skewPA = shu::CrossProductMatrix(pA);
    shu::mat3f skewPB = shu::CrossProductMatrix(pB);

    this->Jacobian.Rows[0][0]  = -1.0f;
    this->Jacobian.Rows[0][1]  = 0.0f;
    this->Jacobian.Rows[0][2]  = 0.0f;
    this->Jacobian.Rows[0][3]  = skewPA.Rows[0][0];
    this->Jacobian.Rows[0][4]  = skewPA.Rows[0][1];
    this->Jacobian.Rows[0][5]  = skewPA.Rows[0][2];
    this->Jacobian.Rows[0][6]  = 1.0f;
    this->Jacobian.Rows[0][7]  = 0.0f;
    this->Jacobian.Rows[0][8]  = 0.0f;
    this->Jacobian.Rows[0][9]  = -skewPB.Rows[0][0];
    this->Jacobian.Rows[0][10] = -skewPB.Rows[0][1];
    this->Jacobian.Rows[0][11] = -skewPB.Rows[0][2];

    this->Jacobian.Rows[1][0]  = 0.0f;
    this->Jacobian.Rows[1][1]  = -1.0f;
    this->Jacobian.Rows[1][2]  = 0.0f;
    this->Jacobian.Rows[1][3]  = skewPA.Rows[1][0];
    this->Jacobian.Rows[1][4]  = skewPA.Rows[1][1];
    this->Jacobian.Rows[1][5]  = skewPA.Rows[1][2];
    this->Jacobian.Rows[1][6]  = 0.0f;
    this->Jacobian.Rows[1][7]  = 1.0f;
    this->Jacobian.Rows[1][8]  = 0.0f;
    this->Jacobian.Rows[1][9]  = -skewPB.Rows[1][0];
    this->Jacobian.Rows[1][10] = -skewPB.Rows[1][1];
    this->Jacobian.Rows[1][11] = -skewPB.Rows[1][2];

    this->Jacobian.Rows[2][0]  = 0.0f; this->Jacobian.Rows[2][1]  = 0.0f; this->Jacobian.Rows[2][2]  = -1.0f;
    this->Jacobian.Rows[2][3]  = skewPA.Rows[2][0];
    this->Jacobian.Rows[2][4]  = skewPA.Rows[2][1];
    this->Jacobian.Rows[2][5]  = skewPA.Rows[2][2];
    this->Jacobian.Rows[2][6]  = 0.0f; this->Jacobian.Rows[2][7]  = 0.0f; this->Jacobian.Rows[2][8]  = 1.0f;
    this->Jacobian.Rows[2][9]  = -skewPB.Rows[2][0];
    this->Jacobian.Rows[2][10] = -skewPB.Rows[2][1];
    this->Jacobian.Rows[2][11] = -skewPB.Rows[2][2];

    // const shu::vec3f n1 = shu::QuatRotateVec(this->A->Rotation, this->AxisLS_A);
    // shu::vec3f u1, v1; n1.GetOrtho(u1, v1);
    // const shu::vec3f n2 = shu::QuatRotateVec(this->B->Rotation, this->AxisLS_B);
    // shu::vec3f u2, v2; n2.GetOrtho(u2, v2);

    // f32 swingAngle = RAD_TO_DEG * shu::CosInverse(sqrtf(0.5f * (1.0f + n1.Dot(n2))));
    shu::vec3f RotationError = shu::Vec3f(0.0f);
    // if (swingAngle <= this->SwingLimit1.x || swingAngle >= this->SwingLimit1.y)
    // {
    //     f32 sinc = NearlyEqual(swingAngle, 0.0f, 1e-4) ? 1.0f
    //                                                    : (shu::SinRad(swingAngle) / swingAngle);
    //     f32 e = Eta(swingAngle);
    //     shu::vec3f A = -e * (n2.Cross(n1)) + sinc * (u2.Cross(n2));
    //     this->Jacobian.Rows[3][3]  = A.x;
    //     this->Jacobian.Rows[3][4]  = A.y;
    //     this->Jacobian.Rows[3][5]  = A.z;
    //     this->Jacobian.Rows[3][9]  = -A.x;
    //     this->Jacobian.Rows[3][10] = -A.y;
    //     this->Jacobian.Rows[3][11] = -A.z;
    //     RotationError.x = u2.Dot(n1) / sinc;

    //     shu::vec3f B = -e * (n2.Cross(n1)) + sinc * (v2.Cross(n1));
    //     this->Jacobian.Rows[4][3] = B.x;
    //     this->Jacobian.Rows[4][4] = B.y;
    //     this->Jacobian.Rows[4][5] = B.z;
    //     this->Jacobian.Rows[4][9] = -B.x;
    //     this->Jacobian.Rows[4][10] = -B.y;
    //     this->Jacobian.Rows[4][11] = -B.z;
    //     RotationError.y = v2.Dot(n1) / sinc;
    // }
    // if (swingAngle <= this->SwingLimit2.x || swingAngle >= this->SwingLimit2.y)
    // {
    //     f32 sinc = NearlyEqual(swingAngle, 0.0f, 1e-4) ? 1.0f
    //                                                    : (shu::SinRad(swingAngle) / swingAngle);
    //     f32 e = Eta(swingAngle);
    // }

    // shu::quat qrel = this->A->Rotation * shu::QuatInverse(this->B->Rotation);
    // shu::QuatNormalize(qrel);
    // f32 twistAngle = RAD_TO_DEG * 2.0f * shu::TanInverse(n1.Dot(qrel.complex), qrel.w);
    // if (twistAngle <= this->TwistLimit.x || twistAngle >= this->TwistLimit.y)
    // {
    //     f32 alpha = n1.Dot(n2);
    //     shu::vec3f C = (n1 + n2) * (1.0f / (1 + alpha));
    //     this->Jacobian.Rows[5][3] = C.x;
    //     this->Jacobian.Rows[5][4] = C.y;
    //     this->Jacobian.Rows[5][5] = C.z;
    //     this->Jacobian.Rows[5][9] = -C.x;
    //     this->Jacobian.Rows[5][10] = -C.y;
    //     this->Jacobian.Rows[5][11] = -C.z;
    //     RotationError.z = twistAngle;
    // }
    const shu::vec3f WorldUp = shu::Vec3f(0, 1, 0);
    const shu::vec3f v1_world = shu::QuatRotateVec(this->A->Rotation, WorldUp);
    const shu::vec3f v2_world = shu::QuatRotateVec(this->B->Rotation, WorldUp);
    const f32 AngleLimitDegrees = 45.0f;
    f32 constraint = v2_world.Dot(v1_world) - shu::CosDeg(AngleLimitDegrees);
    if (constraint < 0.0f)
    {
        shu::vec3f c = v2_world.Cross(v1_world);
        this->Jacobian.Rows[3][3] = c.x;
        this->Jacobian.Rows[3][4] = c.y;
        this->Jacobian.Rows[3][5] = c.z;
        this->Jacobian.Rows[3][9] =  -c.x;
        this->Jacobian.Rows[3][10] = -c.y;
        this->Jacobian.Rows[3][11] = -c.z;

        f32 angleDegrees = shu::CosInverse(constraint) * RAD_TO_DEG;
        // RotationError.x = (angleDegrees < -AngleLimitDegrees) ? shu::CosDeg(angleDegrees + AngleLimitDegrees)
        //                                                       : shu::CosDeg(angleDegrees - AngleLimitDegrees);
        RotationError.x = v2_world.Dot(v1_world);
        // RotationError.x = a1 - 45.0f;
    }

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    shu::vecN<f32, 12> Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    shu::vec3f PositionError = r2MinusR1;
    f32 Beta = 0.6f;
    this->TransBaumgarte = -(Beta / dt) * PositionError;

    Beta = 0.005f;
    this->RotBaumgarte = -(Beta / dt) * RotationError;
}

void
hinge_swing_twist_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 6> JacobianTranspose = this->Jacobian.Transposed();
    shu::vecN<f32, 12>     V                 = GetVelocities();
    shu::matN<f32, 12>     InvM              = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->TransBaumgarte.x;
    Rhs[1] += this->TransBaumgarte.y;
    Rhs[2] += this->TransBaumgarte.z;
    Rhs[3] += this->RotBaumgarte.x;
    // Rhs[4] += this->RotBaumgarte.y;
    // Rhs[5] += this->RotBaumgarte.z;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);
    // if(LagrangeLambda[3] < 0)
    //     LagrangeLambda[3] *= -1.0f;

    auto Impulses       = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
hinge_swing_twist_constraint_3d::PostSolve()
{
#if WARM_STARTING
    // NOTE: Limit warm starting to reasonable limits.
    for (i32 i = 0; i < 6; ++i)
    {
        if (this->PreviousFrameLambda[i] * 0.0f != this->PreviousFrameLambda[i] * 0.0f)
        {
            this->PreviousFrameLambda[i] = 0.0f;
        }

        const f32 Limit = 20.0f;
        if (this->PreviousFrameLambda[i] > Limit)
        {
            this->PreviousFrameLambda[i] = Limit;
        }
        else if (this->PreviousFrameLambda[i] < -Limit)
        {
            this->PreviousFrameLambda[i] = -Limit;
        }
    }

    // const f32 Limit = 1.0f;
    // if (this->PreviousFrameLambda[3] > Limit)
    // {
    //     this->PreviousFrameLambda[3] = Limit;
    // }
    // else if (this->PreviousFrameLambda[3] < -Limit)
    // {
    //     this->PreviousFrameLambda[3] = -Limit;
    // }
#endif
}
