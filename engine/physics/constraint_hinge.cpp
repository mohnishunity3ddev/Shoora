#include "constraint.h"

void
hinge_constraint_3d::PreSolve(const f32 dt)
{
    const shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f rA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f rB = r2 - this->B->GetCenterOfMassWS();

    shu::vec3f world_w1 = shu::QuatRotateVec(this->A->Rotation, this->AxisA);
    world_w1.Normalize();

    shu::vec3f world_w2 = shu::QuatRotateVec(this->B->Rotation, this->AxisA);
    shu::vec3f world_u2, world_v2;
    world_w2.GetOrtho(world_u2, world_v2);

    this->Jacobian.Zero();

    shu::vec3f J1 = -2.0f * r2MinusR1;
    this->Jacobian.Rows[0][0] = J1.x;
    this->Jacobian.Rows[0][1] = J1.y;
    this->Jacobian.Rows[0][2] = J1.z;

    shu::vec3f J2 = -2.0f * rA.Cross(r2MinusR1);
    this->Jacobian.Rows[0][3] = J2.x;
    this->Jacobian.Rows[0][4] = J2.y;
    this->Jacobian.Rows[0][5] = J2.z;

    shu::vec3f J3 = 2.0f * r2MinusR1;
    this->Jacobian.Rows[0][6] = J3.x;
    this->Jacobian.Rows[0][7] = J3.y;
    this->Jacobian.Rows[0][8] = J3.z;

    shu::vec3f J4 = 2.0f * rB.Cross(r2MinusR1);
    this->Jacobian.Rows[0][9] = J4.x;
    this->Jacobian.Rows[0][10] = J4.y;
    this->Jacobian.Rows[0][11] = J4.z;

    // shu::vec3f u2_cross_w1 = shu::Normalize(world_u2.Cross(world_w1));
    shu::vec3f u2_cross_w1 = world_u2.Cross(world_w1);
    {
        this->Jacobian.Rows[1][0] = 0.0f;
        this->Jacobian.Rows[1][1] = 0.0f;
        this->Jacobian.Rows[1][2] = 0.0f;

        this->Jacobian.Rows[1][3] = -u2_cross_w1.x;
        this->Jacobian.Rows[1][4] = -u2_cross_w1.y;
        this->Jacobian.Rows[1][5] = -u2_cross_w1.z;

        this->Jacobian.Rows[1][6] = 0.0f;
        this->Jacobian.Rows[1][7] = 0.0f;
        this->Jacobian.Rows[1][8] = 0.0f;

        this->Jacobian.Rows[1][9] = u2_cross_w1.x;
        this->Jacobian.Rows[1][10] = u2_cross_w1.y;
        this->Jacobian.Rows[1][11] = u2_cross_w1.z;
    }

    // shu::vec3f v2_cross_w1 = shu::Normalize(world_v2.Cross(world_w1));
    shu::vec3f v2_cross_w1 = world_v2.Cross(world_w1);
    {
        this->Jacobian.Rows[2][0] = 0.0f;
        this->Jacobian.Rows[2][1] = 0.0f;
        this->Jacobian.Rows[2][2] = 0.0f;

        this->Jacobian.Rows[2][3] = -v2_cross_w1.x;
        this->Jacobian.Rows[2][4] = -v2_cross_w1.y;
        this->Jacobian.Rows[2][5] = -v2_cross_w1.z;

        this->Jacobian.Rows[2][6] = 0.0f;
        this->Jacobian.Rows[2][7] = 0.0f;
        this->Jacobian.Rows[2][8] = 0.0f;

        this->Jacobian.Rows[2][9] = v2_cross_w1.x;
        this->Jacobian.Rows[2][10] = v2_cross_w1.y;
        this->Jacobian.Rows[2][11] = v2_cross_w1.z;
    }

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    auto Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    f32 ConstraintError = r2MinusR1.Dot(r2MinusR1);
    ConstraintError = MAX(0.0f, ConstraintError - 0.01f);
    f32 Beta = 0.05f;
    this->Baumgarte = -(Beta / dt) * ConstraintError;

    f32 C1 = world_u2.Dot(world_w1);
    f32 C2 = world_v2.Dot(world_w1);
    shu::vec2f RotationError = shu::Vec2f(C1, C2);
    Beta = 0.2f;
    this->RotBaumgarte = -(Beta / dt) * RotationError;
}

void
hinge_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 3> JacobianTranspose = this->Jacobian.Transposed();

    auto V = GetVelocities();
    auto InvM = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->Baumgarte;
    Rhs[1] += this->RotBaumgarte.x;
    Rhs[2] += this->RotBaumgarte.y;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);

    for (i32 i = 0; i < 3; ++i)
    {
        if (LagrangeLambda[i] * 0.0f != LagrangeLambda[i] * 0.0f)
        {
            LagrangeLambda[i] = 0.0f;
        }
        const f32 Limit = 4.0f;
        if (LagrangeLambda[i] > Limit)
        {
            LagrangeLambda[i] = Limit;
        }
        else if (LagrangeLambda[i] < -Limit)
        {
            LagrangeLambda[i] = -Limit;
        }
    }

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
hinge_constraint_3d::PostSolve()
{
#if WARM_STARTING
    // NOTE: Limit warm starting to reasonable limits.
    for (i32 i = 0; i < 3; ++i)
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
#endif
}

