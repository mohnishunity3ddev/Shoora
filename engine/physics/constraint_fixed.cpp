#include "constraint.h"

void
fixed_constraint_3d::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();

    const shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f pA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f pB = r2 - this->B->GetCenterOfMassWS();
    shu::mat3f skewPA = shu::CrossProductMatrix(pA);
    shu::mat3f skewPB = shu::CrossProductMatrix(pB);

    this->Jacobian.Rows[0][0] = -1.0f;
    this->Jacobian.Rows[0][1] = 0.0f;
    this->Jacobian.Rows[0][2] = 0.0f;
    this->Jacobian.Rows[0][3] = skewPA.Rows[0][0];
    this->Jacobian.Rows[0][4] = skewPA.Rows[0][1];
    this->Jacobian.Rows[0][5] = skewPA.Rows[0][2];
    this->Jacobian.Rows[0][6] = 1.0f;
    this->Jacobian.Rows[0][7] = 0.0f;
    this->Jacobian.Rows[0][8] = 0.0f;
    this->Jacobian.Rows[0][9] = -skewPB.Rows[0][0];
    this->Jacobian.Rows[0][10] = -skewPB.Rows[0][1];
    this->Jacobian.Rows[0][11] = -skewPB.Rows[0][2];

    this->Jacobian.Rows[1][0] = 0.0f;
    this->Jacobian.Rows[1][1] = -1.0f;
    this->Jacobian.Rows[1][2] = 0.0f;
    this->Jacobian.Rows[1][3] = skewPA.Rows[1][0];
    this->Jacobian.Rows[1][4] = skewPA.Rows[1][1];
    this->Jacobian.Rows[1][5] = skewPA.Rows[1][2];
    this->Jacobian.Rows[1][6] = 0.0f;
    this->Jacobian.Rows[1][7] = 1.0f;
    this->Jacobian.Rows[1][8] = 0.0f;
    this->Jacobian.Rows[1][9] = -skewPB.Rows[1][0];
    this->Jacobian.Rows[1][10] = -skewPB.Rows[1][1];
    this->Jacobian.Rows[1][11] = -skewPB.Rows[1][2];

    this->Jacobian.Rows[2][0] = 0.0f;
    this->Jacobian.Rows[2][1] = 0.0f;
    this->Jacobian.Rows[2][2] = -1.0f;
    this->Jacobian.Rows[2][3] = skewPA.Rows[2][0];
    this->Jacobian.Rows[2][4] = skewPA.Rows[2][1];
    this->Jacobian.Rows[2][5] = skewPA.Rows[2][2];
    this->Jacobian.Rows[2][6] = 0.0f;
    this->Jacobian.Rows[2][7] = 0.0f;
    this->Jacobian.Rows[2][8] = 1.0f;
    this->Jacobian.Rows[2][9] = -skewPB.Rows[2][0];
    this->Jacobian.Rows[2][10] = -skewPB.Rows[2][1];
    this->Jacobian.Rows[2][11] = -skewPB.Rows[2][2];

    // NOTE: Rotation Jacobian
    shu::quat q = shu::QuatInverse(this->A->Rotation) * this->B->Rotation;
    shu::mat3f R1 = shu::QuatRotationMatrix_Left(this->A->Rotation);
    shu::mat3f Eta = shu::Mat3f(1.0f) * q.w + shu::CrossProductMatrix(q.complex);
    shu::mat3f R1TimesEta = (R1 * Eta).Transposed() * 0.5f;
    {
        this->Jacobian.Rows[3][0] = 0.0f;
        this->Jacobian.Rows[3][1] = 0.0f;
        this->Jacobian.Rows[3][2] = 0.0f;
        this->Jacobian.Rows[3][3] = -R1TimesEta.m00;
        this->Jacobian.Rows[3][4] = -R1TimesEta.m01;
        this->Jacobian.Rows[3][5] = -R1TimesEta.m02;
        this->Jacobian.Rows[3][6] = 0.0f;
        this->Jacobian.Rows[3][7] = 0.0f;
        this->Jacobian.Rows[3][8] = 0.0f;
        this->Jacobian.Rows[3][9] = R1TimesEta.m00;
        this->Jacobian.Rows[3][10] = R1TimesEta.m01;
        this->Jacobian.Rows[3][11] = R1TimesEta.m02;
    }
    {
        this->Jacobian.Rows[4][0] = 0.0f;
        this->Jacobian.Rows[4][1] = 0.0f;
        this->Jacobian.Rows[4][2] = 0.0f;
        this->Jacobian.Rows[4][3] = -R1TimesEta.m10;
        this->Jacobian.Rows[4][4] = -R1TimesEta.m11;
        this->Jacobian.Rows[4][5] = -R1TimesEta.m12;
        this->Jacobian.Rows[4][6] = 0.0f;
        this->Jacobian.Rows[4][7] = 0.0f;
        this->Jacobian.Rows[4][8] = 0.0f;
        this->Jacobian.Rows[4][9] = R1TimesEta.m10;
        this->Jacobian.Rows[4][10] = R1TimesEta.m11;
        this->Jacobian.Rows[4][11] = R1TimesEta.m12;
    }
    {
        this->Jacobian.Rows[5][0] = 0.0f;
        this->Jacobian.Rows[5][1] = 0.0f;
        this->Jacobian.Rows[5][2] = 0.0f;
        this->Jacobian.Rows[5][3] = -R1TimesEta.m20;
        this->Jacobian.Rows[5][4] = -R1TimesEta.m21;
        this->Jacobian.Rows[5][5] = -R1TimesEta.m22;
        this->Jacobian.Rows[5][6] = 0.0f;
        this->Jacobian.Rows[5][7] = 0.0f;
        this->Jacobian.Rows[5][8] = 0.0f;
        this->Jacobian.Rows[5][9] = R1TimesEta.m20;
        this->Jacobian.Rows[5][10] = R1TimesEta.m21;
        this->Jacobian.Rows[5][11] = R1TimesEta.m22;
    }
#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    auto Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    shu::vec3f PositionError = r2MinusR1;
    f32 Beta = 0.5f;
    this->TransBaumgarte = -(Beta / dt) * PositionError;

    // NOTE: Relative Quaternion Qr = (q1^-1)*q2.
    shu::vec3f RotationError = q.complex;
    Beta = 0.5f;
    this->RotBaumgarte = -(Beta / dt) * RotationError;
}

void
fixed_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 6> JacobianTranspose = this->Jacobian.Transposed();

    auto V = GetVelocities();
    auto InvM = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->TransBaumgarte.x;
    Rhs[1] += this->TransBaumgarte.y;
    Rhs[2] += this->TransBaumgarte.z;
    Rhs[3] += this->RotBaumgarte.x;
    Rhs[4] += this->RotBaumgarte.y;
    Rhs[5] += this->RotBaumgarte.z;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
fixed_constraint_3d::PostSolve()
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
