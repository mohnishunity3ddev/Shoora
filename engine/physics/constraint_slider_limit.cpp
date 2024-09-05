#include "constraint.h"

void
slider_constraint_limit_3d::PreSolve(const f32 dt)
{
    this->Jacobian.Zero();
    this->LimitJacobian.Zero();
    this->EnforceLimits = false;

    const shu::vec3f p1 = this->A->LocalToWorldSpace(this->AnchorPointLS_A);
    const shu::vec3f p2 = this->B->LocalToWorldSpace(this->AnchorPointLS_B);
    shu::vec3f u = p2 - p1;

    shu::vec3f x1 = this->A->Position;
    shu::vec3f x2 = this->B->Position;
    const shu::vec3f r1 = p1 - x1;
    const shu::vec3f r2 = p2 - x2;

    shu::vec3f SliderAxis_WS = shu::QuatRotateVec(this->A->Rotation, this->AxisLS_A);
    SliderAxis_WS.Normalize();
    shu::vec3f n1, n2;
    SliderAxis_WS.GetOrtho(n1, n2);

    // NOTE: Translation Jacobian
    shu::vec3f j_v1_0 = -n1;
    shu::vec3f j_w1_0 = -(r1 + u).Cross(n1);
    shu::vec3f j_v2_0 = n1;
    shu::vec3f j_w2_0 = r2.Cross(n1);
    {
        this->Jacobian.Rows[0][0]  = j_v1_0.x;
        this->Jacobian.Rows[0][1]  = j_v1_0.y;
        this->Jacobian.Rows[0][2]  = j_v1_0.z;

        this->Jacobian.Rows[0][3]  = j_w1_0.x;
        this->Jacobian.Rows[0][4]  = j_w1_0.y;
        this->Jacobian.Rows[0][5]  = j_w1_0.z;

        this->Jacobian.Rows[0][6]  = j_v2_0.x;
        this->Jacobian.Rows[0][7]  = j_v2_0.y;
        this->Jacobian.Rows[0][8]  = j_v2_0.z;

        this->Jacobian.Rows[0][9]  = j_w2_0.x;
        this->Jacobian.Rows[0][10] = j_w2_0.y;
        this->Jacobian.Rows[0][11] = j_w2_0.z;
    }

    shu::vec3f j_v1_1 = -n2;
    shu::vec3f j_w1_1 = -(r1 + u).Cross(n2);
    shu::vec3f j_v2_1 = n2;
    shu::vec3f j_w2_1 = r2.Cross(n2);
    {
        this->Jacobian.Rows[1][0]  = j_v1_1.x;
        this->Jacobian.Rows[1][1]  = j_v1_1.y;
        this->Jacobian.Rows[1][2]  = j_v1_1.z;

        this->Jacobian.Rows[1][3]  = j_w1_1.x;
        this->Jacobian.Rows[1][4]  = j_w1_1.y;
        this->Jacobian.Rows[1][5]  = j_w1_1.z;

        this->Jacobian.Rows[1][6]  = j_v2_1.x;
        this->Jacobian.Rows[1][7]  = j_v2_1.y;
        this->Jacobian.Rows[1][8]  = j_v2_1.z;

        this->Jacobian.Rows[1][9]  = j_w2_1.x;
        this->Jacobian.Rows[1][10] = j_w2_1.y;
        this->Jacobian.Rows[1][11] = j_w2_1.z;
    }

    // NOTE: LockRotation Jacobian (Claude Lacoursiere)
    shu::quat  q          = shu::QuatInverse(this->A->Rotation) * this->B->Rotation;
    shu::mat3f R1         = shu::QuatRotationMatrix_Left(this->A->Rotation);
    shu::mat3f Eta        = shu::Mat3f(1.0f) * q.w + shu::CrossProductMatrix(q.complex);
    shu::mat3f R1TimesEta = (R1 * Eta).Transposed() * 0.5f;
    {
        this->Jacobian.Rows[2][0]  = 0.0f;
        this->Jacobian.Rows[2][1]  = 0.0f;
        this->Jacobian.Rows[2][2]  = 0.0f;
        this->Jacobian.Rows[2][3]  = -R1TimesEta.m00;
        this->Jacobian.Rows[2][4]  = -R1TimesEta.m01;
        this->Jacobian.Rows[2][5]  = -R1TimesEta.m02;
        this->Jacobian.Rows[2][6]  = 0.0f;
        this->Jacobian.Rows[2][7]  = 0.0f;
        this->Jacobian.Rows[2][8]  = 0.0f;
        this->Jacobian.Rows[2][9]  = R1TimesEta.m00;
        this->Jacobian.Rows[2][10] = R1TimesEta.m01;
        this->Jacobian.Rows[2][11] = R1TimesEta.m02;
    }
    {
        this->Jacobian.Rows[3][0]  = 0.0f;
        this->Jacobian.Rows[3][1]  = 0.0f;
        this->Jacobian.Rows[3][2]  = 0.0f;
        this->Jacobian.Rows[3][3]  = -R1TimesEta.m10;
        this->Jacobian.Rows[3][4]  = -R1TimesEta.m11;
        this->Jacobian.Rows[3][5]  = -R1TimesEta.m12;
        this->Jacobian.Rows[3][6]  = 0.0f;
        this->Jacobian.Rows[3][7]  = 0.0f;
        this->Jacobian.Rows[3][8]  = 0.0f;
        this->Jacobian.Rows[3][9]  = R1TimesEta.m10;
        this->Jacobian.Rows[3][10] = R1TimesEta.m11;
        this->Jacobian.Rows[3][11] = R1TimesEta.m12;
    }
    {
        this->Jacobian.Rows[4][0]  = 0.0f;
        this->Jacobian.Rows[4][1]  = 0.0f;
        this->Jacobian.Rows[4][2]  = 0.0f;
        this->Jacobian.Rows[4][3]  = -R1TimesEta.m20;
        this->Jacobian.Rows[4][4]  = -R1TimesEta.m21;
        this->Jacobian.Rows[4][5]  = -R1TimesEta.m22;
        this->Jacobian.Rows[4][6]  = 0.0f;
        this->Jacobian.Rows[4][7]  = 0.0f;
        this->Jacobian.Rows[4][8]  = 0.0f;
        this->Jacobian.Rows[4][9]  = R1TimesEta.m20;
        this->Jacobian.Rows[4][10] = R1TimesEta.m21;
        this->Jacobian.Rows[4][11] = R1TimesEta.m22;
    }

    // NOTE: Limit Jacobian.
    shu::vec3f sliderAxis = SliderAxis_WS;
    f32 d = u.Dot(sliderAxis);
    if (d <= this->MinLimit)
    {
        EnforceLimits = true;
        shu::vec3f j_v1_minLim0 = -sliderAxis;
        shu::vec3f j_w1_minLim0 = -(r1 + u).Cross(sliderAxis);
        shu::vec3f j_v2_minLim0 = sliderAxis;
        shu::vec3f j_w2_minLim0 = r2.Cross(sliderAxis);
        {
            this->LimitJacobian.Rows[0][0] = j_v1_minLim0.x;
            this->LimitJacobian.Rows[0][1] = j_v1_minLim0.y;
            this->LimitJacobian.Rows[0][2] = j_v1_minLim0.z;

            this->LimitJacobian.Rows[0][3] = j_w1_minLim0.x;
            this->LimitJacobian.Rows[0][4] = j_w1_minLim0.y;
            this->LimitJacobian.Rows[0][5] = j_w1_minLim0.z;

            this->LimitJacobian.Rows[0][6] = j_v2_minLim0.x;
            this->LimitJacobian.Rows[0][7] = j_v2_minLim0.y;
            this->LimitJacobian.Rows[0][8] = j_v2_minLim0.z;

            this->LimitJacobian.Rows[0][9]  = j_w2_minLim0.x;
            this->LimitJacobian.Rows[0][10] = j_w2_minLim0.y;
            this->LimitJacobian.Rows[0][11] = j_w2_minLim0.z;
        }
        
        f32 beta = 0.3f;
        this->LimitBaumgarte = -(beta / dt) * (d - this->MinLimit);
    }
    else if (d >= this->MaxLimit)
    {
        EnforceLimits = true;
        shu::vec3f j_v1_minLim1 = sliderAxis;
        shu::vec3f j_w1_minLim1 = (r1 + u).Cross(sliderAxis);
        shu::vec3f j_v2_minLim1 = -sliderAxis;
        shu::vec3f j_w2_minLim1 = -r2.Cross(sliderAxis);
        {
            this->LimitJacobian.Rows[0][0] = j_v1_minLim1.x;
            this->LimitJacobian.Rows[0][1] = j_v1_minLim1.y;
            this->LimitJacobian.Rows[0][2] = j_v1_minLim1.z;

            this->LimitJacobian.Rows[0][3] = j_w1_minLim1.x;
            this->LimitJacobian.Rows[0][4] = j_w1_minLim1.y;
            this->LimitJacobian.Rows[0][5] = j_w1_minLim1.z;

            this->LimitJacobian.Rows[0][6] = j_v2_minLim1.x;
            this->LimitJacobian.Rows[0][7] = j_v2_minLim1.y;
            this->LimitJacobian.Rows[0][8] = j_v2_minLim1.z;

            this->LimitJacobian.Rows[0][9] = j_w2_minLim1.x;
            this->LimitJacobian.Rows[0][10] = j_w2_minLim1.y;
            this->LimitJacobian.Rows[0][11] = j_w2_minLim1.z;
        }
        f32 beta = 0.3f;
        this->LimitBaumgarte = -(beta / dt) * (this->MaxLimit - d);
    }

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    shu::vecN<f32, 12> Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif

    shu::vec2f TranslationError = shu::Vec2f(u.Dot(n1), u.Dot(n2));
    f32 Beta = 0.6f;
    this->TransBaumgarte = -(Beta / dt) * TranslationError;

    // NOTE: Relative Quaternion Qr = (q1^-1)*q2 = q.complex.
    shu::vec3f RotationError = q.complex;
    Beta = 0.6f;
    this->RotBaumgarte = -(Beta / dt) * RotationError;

}

void
slider_constraint_limit_3d::Solve()
{
    shu::matMN<f32, 12, 5> JacobianTranspose = this->Jacobian.Transposed();
    shu::vecN <f32, 12>    V                 = GetVelocities();
    shu::matN <f32, 12>    InvM              = GetInverseMassMatrix();

    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs       = this->Jacobian * V * -1.0f;
    Rhs[0] += this->TransBaumgarte.x;
    Rhs[1] += this->TransBaumgarte.y;
    Rhs[2] += this->RotBaumgarte.x;
    Rhs[3] += this->RotBaumgarte.y;
    Rhs[4] += this->RotBaumgarte.z;
    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);
    auto Impulses = JacobianTranspose * LagrangeLambda;

    // NOTE: Limit Solver
    if(this->EnforceLimits)
    {
        // TODO: This can be optimized since the Jacobian "Matrix" is just a row vector in this case.
        // TODO: Just one row of the limit constraint gradient(Min/Max).
        shu::matMN<f32, 12, 1> LimitJ_T = this->LimitJacobian.Transposed();
        // TODO: This is just the gradient divided by Mi and Ii.(?)
        auto JInvJt_Limit = this->LimitJacobian * InvM * LimitJ_T;
        auto Rhs_Limit = this->LimitJacobian * V * -1.0f;
        Rhs_Limit[0] += this->LimitBaumgarte;
        // TODO: No need to use GS here. Optimize this!
        auto LagrangeLimit = shu::LCP_GaussSeidel(JInvJt_Limit, Rhs_Limit);
        auto LimitImpulses = LimitJ_T * LagrangeLimit;

        // NOTE: Getting the aggregate impulses with limits included.
        for (int i = 0; i < 12; ++i)
            Impulses[i] += LimitImpulses[i];
    }

    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

void
slider_constraint_limit_3d::PostSolve()
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
