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
cone_twist_constraint::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();

    const shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    // NOTE: Translation Jacobian.
    shu::vec3f pA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f pB = r2 - this->B->GetCenterOfMassWS();
    shu::mat3f skewPA = shu::CrossProductMatrix(pA);
    shu::mat3f skewPB = shu::CrossProductMatrix(pB);

    this->Jacobian.Rows[0][0]  = -1.0f; this->Jacobian.Rows[0][1]  = 0.0f; this->Jacobian.Rows[0][2]  = 0.0f;
    this->Jacobian.Rows[0][3]  = skewPA.Rows[0][0];
    this->Jacobian.Rows[0][4]  = skewPA.Rows[0][1];
    this->Jacobian.Rows[0][5]  = skewPA.Rows[0][2];
    this->Jacobian.Rows[0][6]  = 1.0f; this->Jacobian.Rows[0][7]  = 0.0f; this->Jacobian.Rows[0][8]  = 0.0f;
    this->Jacobian.Rows[0][9]  = -skewPB.Rows[0][0];
    this->Jacobian.Rows[0][10] = -skewPB.Rows[0][1];
    this->Jacobian.Rows[0][11] = -skewPB.Rows[0][2];

    this->Jacobian.Rows[1][0]  = 0.0f; this->Jacobian.Rows[1][1]  = -1.0f; this->Jacobian.Rows[1][2]  = 0.0f;
    this->Jacobian.Rows[1][3]  = skewPA.Rows[1][0];
    this->Jacobian.Rows[1][4]  = skewPA.Rows[1][1];
    this->Jacobian.Rows[1][5]  = skewPA.Rows[1][2];
    this->Jacobian.Rows[1][6]  = 0.0f; this->Jacobian.Rows[1][7]  = 1.0f; this->Jacobian.Rows[1][8]  = 0.0f;
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

    // NOTE: Rotation Jacobian.

    auto twistAxisA_FrameA = shu::Vec3f(1, 0, 0);
    auto twistAxisB_FrameB = shu::Vec3f(1, 0, 0);

    // B's Rotation in A's frame. A is the static Anchor Body.
    shu::quat qRelative = shu::QuatInverse(A->Rotation) * B->Rotation;
    shu::quat_st stQuat = shu::DecomposeSwingTwist(qRelative, twistAxisB_FrameB);
    shu::vec2f RotationError = shu::Vec2f(0.0f);

    shu::vec3f rotatedTwistAxisB_FrameA = stQuat.rotatedTwistAxis;
    rotatedTwistAxisB_FrameA.Normalize();
    // LogDebug("swing Angle: %f.\n", stQuat.swingAngle);

    if (stQuat.swingAngle > this->ConeLimit)
    {
        // LogWarnUnformatted("Constraint Violation!\n");

        auto swingAxis_FrameA = rotatedTwistAxisB_FrameA.Cross(twistAxisA_FrameA);
        auto swingAxis_WorldFrame = shu::QuatRotateVec(A->Rotation, swingAxis_FrameA);

        this->Jacobian.Rows[3][0] = 0.0f;
        this->Jacobian.Rows[3][1] = 0.0f;
        this->Jacobian.Rows[3][2] = 0.0f;
        this->Jacobian.Rows[3][3] = -swingAxis_WorldFrame.x;
        this->Jacobian.Rows[3][4] = -swingAxis_WorldFrame.y;
        this->Jacobian.Rows[3][5] = -swingAxis_WorldFrame.z;
        this->Jacobian.Rows[3][6] = 0.0f;
        this->Jacobian.Rows[3][7] = 0.0f;
        this->Jacobian.Rows[3][8] = 0.0f;
        this->Jacobian.Rows[3][9] =  swingAxis_WorldFrame.x;
        this->Jacobian.Rows[3][10] = swingAxis_WorldFrame.y;
        this->Jacobian.Rows[3][11] = swingAxis_WorldFrame.z;

        RotationError.x = -(stQuat.swingAngle - this->ConeLimit);
    }

#if ENABLE_TWIST_CONSTRAINT
    // LogDebug("Twist Angle: %f.\n", stQuat.twistAngle);
    if (SHU_ABSOLUTE(stQuat.twistAngle) > this->TwistLimit)
    {
        auto twistAxis_FrameA = rotatedTwistAxisB_FrameA;
        auto twistAxis_WorldFrame = shu::QuatRotateVec(A->Rotation, twistAxis_FrameA);

        this->Jacobian.Rows[4][0] = 0.0f;
        this->Jacobian.Rows[4][1] = 0.0f;
        this->Jacobian.Rows[4][2] = 0.0f;
        this->Jacobian.Rows[4][3] = -twistAxis_WorldFrame.x;
        this->Jacobian.Rows[4][4] = -twistAxis_WorldFrame.y;
        this->Jacobian.Rows[4][5] = -twistAxis_WorldFrame.z;
        this->Jacobian.Rows[4][6] = 0.0f;
        this->Jacobian.Rows[4][7] = 0.0f;
        this->Jacobian.Rows[4][8] = 0.0f;
        this->Jacobian.Rows[4][9] = twistAxis_WorldFrame.x;
        this->Jacobian.Rows[4][10] = twistAxis_WorldFrame.y;
        this->Jacobian.Rows[4][11] = twistAxis_WorldFrame.z;

        RotationError.y = -SIGN(stQuat.twistAngle) * (stQuat.twistAngle - this->TwistLimit) * DEG_TO_RAD;
    }
#endif

    // LogDebug("Swing Angle: %f.\n", stQuat.swingAngle);
    // Bringing the "Swung" Twist Axis(which was in A's frame) back to world space.
    shu::vec3f r_world = shu::QuatRotateVec(A->Rotation, stQuat.rotatedTwistAxis);

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    shu::vecN<f32, 12> Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    f32 dtInv = 1.0f / dt;

    shu::vec3f PositionError = r2MinusR1;
    f32 Beta = 0.6f;
    this->TransBaumgarte = -(Beta * dtInv) * PositionError;

    Beta = .0025f;
    this->RotBaumgarte = -(Beta * dtInv) * RotationError;
}

void
cone_twist_constraint::Solve()
{
    shu::matMN<f32, 12, 5> JacobianTranspose = this->Jacobian.Transposed();
    shu::vecN<f32, 12>     V                 = GetVelocities();
    shu::matN<f32, 12>     InvM              = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->TransBaumgarte.x;
    Rhs[1] += this->TransBaumgarte.y;
    Rhs[2] += this->TransBaumgarte.z;
    Rhs[3] += this->RotBaumgarte.x;
    Rhs[4] += this->RotBaumgarte.y;

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
cone_twist_constraint::PostSolve()
{
#if WARM_STARTING
    // NOTE: Limit warm starting to reasonable limits.
    for (i32 i = 0; i < 5; ++i)
    {
        if (this->PreviousFrameLambda[i] * 0.0f != this->PreviousFrameLambda[i] * 0.0f)
        {
            this->PreviousFrameLambda[i] = 0.0f;
        }

        const f32 Limit = 5.0f;
        if (this->PreviousFrameLambda[i] > Limit)
        {
            this->PreviousFrameLambda[i] = Limit;
        }
        else if (this->PreviousFrameLambda[i] < -Limit)
        {
            this->PreviousFrameLambda[i] = 0.0f;
        }
    }
#endif
}
