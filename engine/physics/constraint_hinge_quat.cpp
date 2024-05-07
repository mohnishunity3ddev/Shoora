#include "constraint.h"

void
hinge_quat_constraint_3d::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();

    const shu::vec3f r1  = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2  = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f pA       = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f pB       = r2 - this->B->GetCenterOfMassWS();
    shu::mat3f skewPA   = shu::CrossProductMatrix(pA);
    shu::mat3f skewPB   = shu::CrossProductMatrix(pB);

    this->Jacobian.Rows[0][0]  = -1.0f;
    this->Jacobian.Rows[0][1]  =  0.0f;
    this->Jacobian.Rows[0][2]  =  0.0f;
    this->Jacobian.Rows[0][3]  =  skewPA.Rows[0][0];
    this->Jacobian.Rows[0][4]  =  skewPA.Rows[0][1];
    this->Jacobian.Rows[0][5]  =  skewPA.Rows[0][2];
    this->Jacobian.Rows[0][6]  =  1.0f;
    this->Jacobian.Rows[0][7]  =  0.0f;
    this->Jacobian.Rows[0][8]  =  0.0f;
    this->Jacobian.Rows[0][9]  = -skewPB.Rows[0][0];
    this->Jacobian.Rows[0][10] = -skewPB.Rows[0][1];
    this->Jacobian.Rows[0][11] = -skewPB.Rows[0][2];

    this->Jacobian.Rows[1][0]  =  0.0f;
    this->Jacobian.Rows[1][1]  = -1.0f;
    this->Jacobian.Rows[1][2]  =  0.0f;
    this->Jacobian.Rows[1][3]  =  skewPA.Rows[1][0];
    this->Jacobian.Rows[1][4]  =  skewPA.Rows[1][1];
    this->Jacobian.Rows[1][5]  =  skewPA.Rows[1][2];
    this->Jacobian.Rows[1][6]  =  0.0f;
    this->Jacobian.Rows[1][7]  =  1.0f;
    this->Jacobian.Rows[1][8]  =  0.0f;
    this->Jacobian.Rows[1][9]  = -skewPB.Rows[1][0];
    this->Jacobian.Rows[1][10] = -skewPB.Rows[1][1];
    this->Jacobian.Rows[1][11] = -skewPB.Rows[1][2];

    this->Jacobian.Rows[2][0]  =  0.0f;
    this->Jacobian.Rows[2][1]  =  0.0f;
    this->Jacobian.Rows[2][2]  = -1.0f;
    this->Jacobian.Rows[2][3]  =  skewPA.Rows[2][0];
    this->Jacobian.Rows[2][4]  =  skewPA.Rows[2][1];
    this->Jacobian.Rows[2][5]  =  skewPA.Rows[2][2];
    this->Jacobian.Rows[2][6]  =  0.0f;
    this->Jacobian.Rows[2][7]  =  0.0f;
    this->Jacobian.Rows[2][8]  =  1.0f;
    this->Jacobian.Rows[2][9]  = -skewPB.Rows[2][0];
    this->Jacobian.Rows[2][10] = -skewPB.Rows[2][1];
    this->Jacobian.Rows[2][11] = -skewPB.Rows[2][2];

    // NOTE: The Jacobians are all in A's Frame.
    // IMPORTANT: NOTE: check engine/physics/concepts/constraints/quaternion_constraints for Jacobian Derivation
    shu::vec3f local_w1 = this->AxisLS_A;
    local_w1.Normalize();

    shu::vec3f local_u1, local_v1;
    local_w1.GetOrtho(local_u1, local_v1);

    shu::quat q = shu::QuatInverse(this->A->Rotation) * this->B->Rotation;
    shu::mat3f R1 = shu::QuatRotationMatrix_Left(this->A->Rotation);
    shu::mat3f Eta = shu::Mat3f(1.0f)*q.w + shu::CrossProductMatrix(q.complex);
    shu::mat3f R1TimesEta = (R1 * Eta).Transposed() * 0.5f;

    shu::vec3f JacobianU1 = local_u1 * R1TimesEta;
    {
        this->Jacobian.Rows[3][0]  = 0.0f;
        this->Jacobian.Rows[3][1]  = 0.0f;
        this->Jacobian.Rows[3][2]  = 0.0f;

        this->Jacobian.Rows[3][3]  = -JacobianU1.x;
        this->Jacobian.Rows[3][4]  = -JacobianU1.y;
        this->Jacobian.Rows[3][5]  = -JacobianU1.z;

        this->Jacobian.Rows[3][6]  = 0.0f;
        this->Jacobian.Rows[3][7]  = 0.0f;
        this->Jacobian.Rows[3][8]  = 0.0f;

        this->Jacobian.Rows[3][9]  = JacobianU1.x;
        this->Jacobian.Rows[3][10] = JacobianU1.y;
        this->Jacobian.Rows[3][11] = JacobianU1.z;
    }

    shu::vec3f JacobianV1 = local_v1 * R1TimesEta;
    {
        this->Jacobian.Rows[4][0]  =  0.0f;
        this->Jacobian.Rows[4][1]  =  0.0f;
        this->Jacobian.Rows[4][2]  =  0.0f;

        this->Jacobian.Rows[4][3]  = -JacobianV1.x;
        this->Jacobian.Rows[4][4]  = -JacobianV1.y;
        this->Jacobian.Rows[4][5]  = -JacobianV1.z;

        this->Jacobian.Rows[4][6]  =  0.0f;
        this->Jacobian.Rows[4][7]  =  0.0f;
        this->Jacobian.Rows[4][8]  =  0.0f;

        this->Jacobian.Rows[4][9]  =  JacobianV1.x;
        this->Jacobian.Rows[4][10] =  JacobianV1.y;
        this->Jacobian.Rows[4][11] =  JacobianV1.z;
    }

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    shu::vecN<f32, 12> Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    shu::vec3f PositionError = r2MinusR1;
    f32 Beta = 0.25f;
    this->Baumgarte = -(Beta / dt) * PositionError;

    f32 C1 = local_u1.Dot(q.complex);
    f32 C2 = local_v1.Dot(q.complex);
    shu::vec2f RotationError = shu::Vec2f(C1, C2);
    Beta = 0.2f;
    this->RotBaumgarte = -(Beta / dt) * RotationError;
}

void
hinge_quat_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 5> JacobianTranspose = this->Jacobian.Transposed();

    shu::vecN<f32, 12> V = GetVelocities();
    shu::matN<f32, 12> InvM = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->Baumgarte.x;
    Rhs[1] += this->Baumgarte.y;
    Rhs[2] += this->Baumgarte.z;
    Rhs[3] += this->RotBaumgarte.x;
    Rhs[4] += this->RotBaumgarte.y;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
hinge_quat_constraint_3d::PostSolve()
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
