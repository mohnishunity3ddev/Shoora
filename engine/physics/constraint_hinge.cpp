#include "constraint.h"

void
hinge_constraint_3d::PreSolve(const f32 dt)
{
    const shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f rA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f rB = r2 - this->B->GetCenterOfMassWS();

    shu::quat q1 = this->A->Rotation;
    shu::quat q2 = this->B->Rotation;
    shu::quat q0_inv = shu::QuatInverse(this->q0);
    shu::quat q1_inv = shu::QuatInverse(q1);

    shu::vec3f u_AxisA, v_AxisA;
    shu::vec3f AxisAInW = shu::QuatRotateVec(this->A->Rotation, this->AxisA);
    AxisAInW.GetOrtho(u_AxisA, v_AxisA);

    shu::vec3f AxisBInW = shu::QuatRotateVec(this->B->Rotation, this->AxisB);
    shu::vec3f u_AxisB, v_AxisB;
    AxisBInW.GetOrtho(u_AxisB, v_AxisB);

    shu::vec3f a1 = AxisAInW;
    shu::vec3f b2 = u_AxisB;
    shu::vec3f c2 = v_AxisB;

    shu::mat4f P;
    P.Row0 = shu::Vec4f(0, 0, 0, 0);
    P.Row1 = shu::Vec4f(0, 1, 0, 0);
    P.Row2 = shu::Vec4f(0, 0, 1, 0);
    P.Row3 = shu::Vec4f(0, 0, 0, 1);
    shu::mat4f Pt = P.Transposed();

    const shu::mat4f MatA = P * ((q1_inv).LeftOp() * (q2 * q0_inv).RightOp()) * -0.5f;
    const shu::mat4f MatB = P * ((q1_inv).LeftOp() * (q2 * q0_inv).RightOp()) * 0.5f;

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

    shu::vec4f Temp;
    shu::vec3f t2;
    {
        J1.Zero();
        this->Jacobian.Rows[1][0] = J1.x;
        this->Jacobian.Rows[1][1] = J1.y;
        this->Jacobian.Rows[1][2] = J1.z;

        Temp = MatA * shu::Vec4f(0, u_AxisA.x, u_AxisA.y, u_AxisA.z);
        t2 = -b2.Cross(a1);
        // J2 = shu::Vec3f(Temp.y, Temp.z, Temp.w);
        J2 = shu::Normalize(t2);
        this->Jacobian.Rows[1][3] = J2.x;
        this->Jacobian.Rows[1][4] = J2.y;
        this->Jacobian.Rows[1][5] = J2.z;

        J3.Zero();
        this->Jacobian.Rows[1][6] = J3.x;
        this->Jacobian.Rows[1][7] = J3.y;
        this->Jacobian.Rows[1][8] = J3.z;


        Temp = MatB * shu::Vec4f(0, u_AxisA.x, u_AxisA.y, u_AxisA.z);
        t2 = b2.Cross(a1);
        // J4 = shu::Vec3f(Temp.y, Temp.z, Temp.w);
        J4 = shu::Normalize(t2);
        this->Jacobian.Rows[1][9]  = J4.x;
        this->Jacobian.Rows[1][10] = J4.y;
        this->Jacobian.Rows[1][11] = J4.z;
    }
    {
        J1.Zero();
        this->Jacobian.Rows[2][0] = J1.x;
        this->Jacobian.Rows[2][1] = J1.y;
        this->Jacobian.Rows[2][2] = J1.z;

        Temp = MatA * shu::Vec4f(0, v_AxisA.x, v_AxisA.y, v_AxisA.z);
        t2 = -c2.Cross(a1);
        // J2 = shu::Vec3f(Temp.y, Temp.z, Temp.w);
        J2 = shu::Normalize(t2);
        this->Jacobian.Rows[2][3] = J2.x;
        this->Jacobian.Rows[2][4] = J2.y;
        this->Jacobian.Rows[2][5] = J2.z;

        J3.Zero();
        this->Jacobian.Rows[2][6] = J3.x;
        this->Jacobian.Rows[2][7] = J3.y;
        this->Jacobian.Rows[2][8] = J3.z;


        Temp = MatB * shu::Vec4f(0, v_AxisA.x, v_AxisA.y, v_AxisA.z);
        t2 = c2.Cross(a1);
        J4 = shu::Normalize(t2);
        this->Jacobian.Rows[2][9]  = J4.x;
        this->Jacobian.Rows[2][10] = J4.y;
        this->Jacobian.Rows[2][11] = J4.z;
    }

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    auto Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    f32 ConstraintError = r2MinusR1.Dot(r2MinusR1);
    ConstraintError = MAX(0.0f, ConstraintError - 0.01f);
    const f32 Beta = 0.05f;
    this->Baumgarte = -(Beta / dt) * ConstraintError;

    shu::vec2f RotationError = shu::Vec2f(a1.Dot(b2), a1.Dot(c2));
    RotationError.x = MAX(0.0f, RotationError.x - 0.01f);
    RotationError.y = MAX(0.0f, RotationError.y - 0.01f);
    const f32 Beta2 = 0.05f;
    this->RotBaumgarte = -(Beta2 / dt) * RotationError;
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

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
hinge_constraint_3d::PostSolve()
{
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
}

