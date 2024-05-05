#include "constraint.h"

void
ball_constraint_3d::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();

    shu::vec3f r1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    shu::vec3f r2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f r2MinusR1 = r2 - r1;

    shu::vec3f pA = r1 - this->A->GetCenterOfMassWS();
    shu::vec3f pB = r2 - this->B->GetCenterOfMassWS();
    auto pCapA = shu::CrossProductMatrix(pA);
    auto pCapB = shu::CrossProductMatrix(pB);

    this->Jacobian.Rows[0][0]  =  1.0f;
    this->Jacobian.Rows[0][1]  =  0.0f;
    this->Jacobian.Rows[0][2]  =  0.0f;
    this->Jacobian.Rows[0][3]  = -pCapA.Rows[0][0];
    this->Jacobian.Rows[0][4]  = -pCapA.Rows[0][1];
    this->Jacobian.Rows[0][5]  = -pCapA.Rows[0][2];
    this->Jacobian.Rows[0][6]  = -1.0f;
    this->Jacobian.Rows[0][7]  =  0.0f;
    this->Jacobian.Rows[0][8]  =  0.0f;
    this->Jacobian.Rows[0][9]  =  pCapB.Rows[0][0];
    this->Jacobian.Rows[0][10] =  pCapB.Rows[0][1];
    this->Jacobian.Rows[0][11] =  pCapB.Rows[0][2];

    this->Jacobian.Rows[1][0]  =  0.0f;
    this->Jacobian.Rows[1][1]  =  1.0f;
    this->Jacobian.Rows[1][2]  =  0.0f;
    this->Jacobian.Rows[1][3]  = -pCapA.Rows[1][0];
    this->Jacobian.Rows[1][4]  = -pCapA.Rows[1][1];
    this->Jacobian.Rows[1][5]  = -pCapA.Rows[1][2];
    this->Jacobian.Rows[1][6]  =  0.0f;
    this->Jacobian.Rows[1][7]  = -1.0f;
    this->Jacobian.Rows[1][8]  =  0.0f;
    this->Jacobian.Rows[1][9]  =  pCapB.Rows[1][0];
    this->Jacobian.Rows[1][10] =  pCapB.Rows[1][1];
    this->Jacobian.Rows[1][11] =  pCapB.Rows[1][2];

    this->Jacobian.Rows[2][0]  =  0.0f;
    this->Jacobian.Rows[2][1]  =  0.0f;
    this->Jacobian.Rows[2][2]  =  1.0f;
    this->Jacobian.Rows[2][3]  = -pCapA.Rows[2][0];
    this->Jacobian.Rows[2][4]  = -pCapA.Rows[2][1];
    this->Jacobian.Rows[2][5]  = -pCapA.Rows[2][2];
    this->Jacobian.Rows[2][6]  =  0.0f;
    this->Jacobian.Rows[2][7]  =  0.0f;
    this->Jacobian.Rows[2][8]  = -1.0f;
    this->Jacobian.Rows[2][9]  =  pCapB.Rows[2][0];
    this->Jacobian.Rows[2][10] =  pCapB.Rows[2][1];
    this->Jacobian.Rows[2][11] =  pCapB.Rows[2][2];

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    auto Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    shu::vec3f PositionError = -r2MinusR1;
    const f32 Beta = 0.2f;
    this->Baumgarte = -(Beta / dt) * PositionError;
}

void
ball_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 3> JacobianTranspose = this->Jacobian.Transposed();

    auto V = GetVelocities();
    auto InvM = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs[0] += this->Baumgarte.x;
    Rhs[1] += this->Baumgarte.y;
    Rhs[2] += this->Baumgarte.z;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
ball_constraint_3d::PostSolve()
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
