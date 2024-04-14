#include "constraint.h"

void
joint_constraint_3d::PreSolve(const f32 dt)
{
    // NOTE: World Space Position of the hinge from A's Local Space.
    shu::vec3f r1 = A->LocalToWorldSpace(this->AnchorPointLS_A);

    // NOTE: World Space Position of the hinge from B's Local Space.
    shu::vec3f r2 = B->LocalToWorldSpace(this->AnchorPointLS_B);

    shu::vec3f r2MinusR1 = r2 - r1;
    // LogInfo("R2 - R1 = {%f, %f, %f}.\n", r2MinusR1.x, r2MinusR1.y, r2MinusR1.z);

    shu::vec3f rA = r1 - A->GetCenterOfMassWS();
    shu::vec3f rB = r2 - B->GetCenterOfMassWS();

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

#if WARM_STARTING
    // NOTE: Warm starting the bodies using previous frame's Lagrange Lambda.
    auto Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambda;
    this->ApplyImpulses(Impulses);
#endif
}

void
joint_constraint_3d::Solve()
{
    shu::matMN<f32, 12, 1> JacobianTranspose = this->Jacobian.Transposed();

    auto V = GetVelocities();
    auto InvM = GetInverseMassMatrix();
    
    auto J_invM_Jt = this->Jacobian * InvM * JacobianTranspose;
    auto Rhs = this->Jacobian * V * -1.0f;

    auto LagrangeLambda = shu::LCP_GaussSeidel(J_invM_Jt, Rhs);

    auto Impulses = JacobianTranspose * LagrangeLambda;
    this->ApplyImpulses(Impulses);

#if WARM_STARTING
    this->PreviousFrameLambda += LagrangeLambda;
#endif
}

joint_constraint_2d::joint_constraint_2d() : constraint_2d(), Bias(0.0f)
{
    // Default Constructor. Not implemented yet!
    Jacobian.Zero();
    CachedLambda.Zero();
}

joint_constraint_2d::joint_constraint_2d(shoora_body *a, shoora_body *b, const shu::vec2f &anchorPointWS)
    : constraint_2d(), Bias(0.0f)
{
#if 0
    ASSERT(a != nullptr && b != nullptr);
    Jacobian.Zero();
    CachedLambda.Zero();

    this->A = a;
    this->B = b;

    this->AnchorPointLS_A = a->WorldToLocalSpace(anchorPointWS);
    this->AnchorPointLS_B = b->WorldToLocalSpace(anchorPointWS);
#endif
}

void
joint_constraint_2d::PreSolve(const f32 dt)
{
#if 0
    // NOTE: --Compute the Jacobian--
    // Already computed the jacobian in my notes. Only populating it here.
    // [2(pa - pb) | 2(ra X (pa - pb) | 2(pb - pa) | 2(rb X (pb - pa)))] = J
    Jacobian.Zero();

    // Get the anchor point for the joint in world space.
    const Shu::vec2f pA = A->LocalToWorldSpace(AnchorPointLS_A);
    const Shu::vec2f pB = B->LocalToWorldSpace(AnchorPointLS_B);

    const Shu::vec2f rA = pA - A->Position.xy;
    const Shu::vec2f rB = pB - B->Position.xy;

    // This is multiplied with A's linear velocity.
    Shu::vec2f J1 = (pA - pB) * 2.0f;
    Jacobian.Data[0][0] = J1.x;
    Jacobian.Data[1][0] = J1.y;
    // This one gets multiplied with the angular velocity of A.
    f32 J2 = rA.Cross(pA - pB) * 2.0f;
    Jacobian.Data[2][0] = J2;

    // This is multiplied with B's linear velocity.
    Shu::vec2f J3 = (pB - pA) * 2.0f;
    Jacobian.Data[3][0] = J3.x;
    Jacobian.Data[4][0] = J3.y;
    // This one gets multiplied with the angular velocity of A.
    f32 J4 = rB.Cross(pB - pA) * 2.0f;
    Jacobian.Data[5][0] = J4;

    // IMPORTANT: NOTE: This is where we do our actual Warm Starting.
    // We apply the impulse based on the solution for Lambda from the Previous Frame's solver iterations. Apply it
    // once here. Now, we are pretty much very close to the actual solution so it will require fewer iterations for
    // the solver to get to near correct lambda value for this current frame.
    auto Jt = Jacobian.Transposed();
    f32 ImpulseMagnitude = CachedLambda[0];
    Shu::vecN<f32, 6> ImpulseDirection = Jt.Rows[0];

    // NOTE: This is the final impulse we need to apply to all the bodies in this constraint to solve the
    // constraint.
    Shu::vecN<f32, 6> FinalImpulses = ImpulseDirection * ImpulseMagnitude;

    A->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[0], FinalImpulses[1]));
    A->ApplyImpulseAngular(FinalImpulses[2]);
    B->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[3], FinalImpulses[4]));
    B->ApplyImpulseAngular(FinalImpulses[5]);

    // Computing the bias term (using Baumgarte stabilization techinique)
    const f32 beta = 0.1f;
    // NOTE: The positional error "C". Here since this is a distance constraint. pB and pA should be the same. The
    // distance between them therefore is the error. And by adding this bias term, what we are saying is when this
    // is subtracted from our solution, it moves us closer to a solution with less error. if we chose beta as 1.0f
    // then correcting will be whole and sudded. We chose the beta to be 0.1f so the solution will "move" towards
    // the solution with the least amount of error.
    f32 C = (pB - pA).Dot(pB - pA);
    // NOTE: Adding a bias here leads to increase in the energy of the system. So we are adding a threshold, if the
    // positional error is too small, we dont want to add bias to the system.
    C = MAX(0.0f, C - 0.01f);
    Bias = (beta / dt) * C;
#endif
}

void
joint_constraint_2d::Solve()
{
#if 0
    // V = GetVelocities()
    const Shu::vecN<f32, 6> V = GetVelocities();
    const Shu::matN<f32, 6> InvM = GetInverseMassMatrix();

    // Calculate the Lagrange Multiplier - which is the impulse magnitude to be applied to the bodies to solve the
    // constraint.
    // NOTE: we calculated the lambda(Lagrange multiplier/Impulse magnitude) to be
    // -(JV + b) / (J*(M^-1)*Jt) where Jt is the transpose of J.
    const auto J = Jacobian;
    const auto Jt = J.Transposed();

    Shu::matN<f32, 1> lhs = ((Jt * InvM) * J); // A
    Shu::vecN<f32, 1> rhs = (V * J * -1.0f);   // b
    rhs[0] -= Bias;

    // NOTE: This is a linear eq where lhs is a mat, and rhs is a vector.
    // in the form Ax = b. So we pass in A and b and get back a solution x.
    Shu::vecN<f32, 1> Lambda = Shu::LCP_GaussSeidel(lhs, rhs);

    f32 ImpulseMagnitude = Lambda[0];
    Shu::vecN<f32, 6> ImpulseDirection = Jt.Rows[0];

    // NOTE: This is the final impulse we need to apply to all the bodies in this constraint to solve the
    // constraint.
    Shu::vecN<f32, 6> FinalImpulses = ImpulseDirection * ImpulseMagnitude;

    A->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[0], FinalImpulses[1]));
    A->ApplyImpulseAngular(FinalImpulses[2]);
    B->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[3], FinalImpulses[4]));
    B->ApplyImpulseAngular(FinalImpulses[5]);

    this->CachedLambda += Lambda;
#endif
}

void
joint_constraint_2d::PostSolve()
{
}
