#include "constraint.h"

void
penetration_constraint_3d::PreSolve(const f32 dt)
{
    shu::vec3f r1 = A->LocalToWorldSpace(this->AnchorPointLS_A);
    shu::vec3f r2 = B->LocalToWorldSpace(this->AnchorPointLS_B);

    shu::vec3f rA = r1 - A->GetCenterOfMassWS();
    shu::vec3f rB = r2 - B->GetCenterOfMassWS();

    f32 FrictionA = A->FrictionCoeff;
    f32 FrictionB = B->FrictionCoeff;
    this->Friction = FrictionA * FrictionB;

    shu::vec3f FrictionDirection1, FrictionDirection2;
    this->Normal_LocalSpaceA.GetOrtho(FrictionDirection1, FrictionDirection2);

    // NOTE: Normal in world space.
    shu::vec3f Normal = shu::QuatRotateVec(A->Rotation, this->Normal_LocalSpaceA);
    // NOTE: Friction Directions relative to the orientation of A.
    FrictionDirection1 = shu::QuatRotateVec(A->Rotation, FrictionDirection1);
    FrictionDirection2 = shu::QuatRotateVec(A->Rotation, FrictionDirection2);

    this->Jacobian.Zero();

    // NOTE: Penetration Jacobians.
    shu::vec3f J1 = -Normal;
    this->Jacobian.Rows[0][0] = J1.x;
    this->Jacobian.Rows[0][1] = J1.y;
    this->Jacobian.Rows[0][2] = J1.z;

    shu::vec3f J2 = -rA.Cross(Normal);
    this->Jacobian.Rows[0][3] = J2.x;
    this->Jacobian.Rows[0][4] = J2.y;
    this->Jacobian.Rows[0][5] = J2.z;

    shu::vec3f J3 = Normal;
    this->Jacobian.Rows[0][6] = J3.x;
    this->Jacobian.Rows[0][7] = J3.y;
    this->Jacobian.Rows[0][8] = J3.z;

    shu::vec3f J4 = rB.Cross(Normal);
    this->Jacobian.Rows[0][9] = J4.x;
    this->Jacobian.Rows[0][10] = J4.y;
    this->Jacobian.Rows[0][11] = J4.z;

    // NOTE: Friction Jacobians.
    if(this->Friction > 0.0f)
    {
        shu::vec3f J1 = -FrictionDirection1;
        this->Jacobian.Rows[1][0] = J1.x;
        this->Jacobian.Rows[1][1] = J1.y;
        this->Jacobian.Rows[1][2] = J1.z;

        shu::vec3f J2 = -rA.Cross(FrictionDirection1);
        this->Jacobian.Rows[1][3] = J2.x;
        this->Jacobian.Rows[1][4] = J2.y;
        this->Jacobian.Rows[1][5] = J2.z;

        shu::vec3f J3 = FrictionDirection1;
        this->Jacobian.Rows[1][6] = J3.x;
        this->Jacobian.Rows[1][7] = J3.y;
        this->Jacobian.Rows[1][8] = J3.z;
        
        shu::vec3f J4 = rB.Cross(FrictionDirection1);
        this->Jacobian.Rows[1][9] = J4.x;
        this->Jacobian.Rows[1][10] = J4.y;
        this->Jacobian.Rows[1][11] = J4.z;
    }
    if(this->Friction > 0.0f)
    {
        shu::vec3f J1 = -FrictionDirection2;
        this->Jacobian.Rows[2][0] = J1.x;
        this->Jacobian.Rows[2][1] = J1.y;
        this->Jacobian.Rows[2][2] = J1.z;

        shu::vec3f J2 = -rA.Cross(FrictionDirection2);
        this->Jacobian.Rows[2][3] = J2.x;
        this->Jacobian.Rows[2][4] = J2.y;
        this->Jacobian.Rows[2][5] = J2.z;

        shu::vec3f J3 = FrictionDirection2;
        this->Jacobian.Rows[2][6] = J3.x;
        this->Jacobian.Rows[2][7] = J3.y;
        this->Jacobian.Rows[2][8] = J3.z;

        shu::vec3f J4 = rB.Cross(FrictionDirection2);
        this->Jacobian.Rows[2][9] = J4.x;
        this->Jacobian.Rows[2][10] = J4.y;
        this->Jacobian.Rows[2][11] = J4.z;
    }

    // NOTE: Apply Warm starting using previous frame's Lambda.
    const shu::vecN<f32, 12> Impulses = this->Jacobian.Transposed() * this->PreviousFrameLambdas;
    this->ApplyImpulses(Impulses);

    // NOTE: Baumgarte Stabilization
    f32 ConstraintError = (r2 - r1).Dot(Normal);
    ConstraintError = MIN(0.0f, ConstraintError + 0.02f);
    f32 Beta = 0.25f;
    this->Baumgarte = -(Beta / dt) * ConstraintError;
}

void
penetration_constraint_3d::Solve()
{
    auto JacobianTranspose = this->Jacobian.Transposed();

    auto V = GetVelocities();
    auto InvM = GetInverseMassMatrix();
    auto J_InvM_Jt = this->Jacobian * InvM * JacobianTranspose;

    auto Rhs = this->Jacobian * V * -1.0f;
    Rhs += this->Baumgarte;

    auto LagrangeLambdas = shu::LCP_GaussSeidel(J_InvM_Jt, Rhs);

    auto OldLambdas = this->PreviousFrameLambdas;
    this->PreviousFrameLambdas += LagrangeLambdas;
    const f32 LambdaLimit = 0.0f;
    if(this->PreviousFrameLambdas[0] < LambdaLimit)
    {
        this->PreviousFrameLambdas[0] = LambdaLimit;
    }
    if(this->Friction > 0.0f)
    {
        f32 FrictionForce = this->Friction * 9.8f * 1.0f / (A->InvMass + B->InvMass);
        f32 NormalForce = SHU_ABSOLUTE(LagrangeLambdas[0] * this->Friction);
        f32 MaxForce = (FrictionForce > NormalForce) ? FrictionForce : NormalForce;

        if(PreviousFrameLambdas[1] > MaxForce) {
            PreviousFrameLambdas[1] = MaxForce;
        }
        if(PreviousFrameLambdas[1] < -MaxForce) {
            PreviousFrameLambdas[1] = -MaxForce;
        }

        if(PreviousFrameLambdas[2] > MaxForce) {
            PreviousFrameLambdas[2] = MaxForce;
        }
        if(PreviousFrameLambdas[2] < -MaxForce) {
            PreviousFrameLambdas[2] = -MaxForce;
        }
    }
    LagrangeLambdas = this->PreviousFrameLambdas - OldLambdas;

    auto Impulses = JacobianTranspose * LagrangeLambdas;
    this->ApplyImpulses(Impulses);
}

penetration_constraint_2d::penetration_constraint_2d() : constraint_2d()
{
    Jacobian.Zero();
    CachedLambda.Zero();
    Bias = 0.0f;
    Friction = 0.0f;
}

penetration_constraint_2d::penetration_constraint_2d(shoora_body *a, shoora_body *b, const contact &contact)
    : penetration_constraint_2d()
{
#if 0
    this->A = a;
    this->B = b;
    this->AnchorPointLS_A = a->WorldToLocalSpace(contact.Start.xy);
    this->AnchorPointLS_B = b->WorldToLocalSpace(contact.End.xy);
    this->Normal = a->WorldToLocalSpace(contact.Normal.xy);
#endif
}

void
penetration_constraint_2d::PreSolve(const f32 dt)
{
#if 0
    Jacobian.Zero();

    const Shu::vec2f pA = A->LocalToWorldSpace(AnchorPointLS_A);
    const Shu::vec2f pB = B->LocalToWorldSpace(AnchorPointLS_B);
    const Shu::vec2f n = A->LocalToWorldSpace(Normal);

    const Shu::vec2f rA = pA - A->Position.xy;
    const Shu::vec2f rB = pB - B->Position.xy;

    // Populating the first column of the Jacobian. Normal vector along collision normal.
    Shu::vec2f J1 = -n;
    Jacobian.Data[0][0] = J1.x;
    Jacobian.Data[1][0] = J1.y;
    f32 J2 = -rA.Cross(n);
    Jacobian.Data[2][0] = J2;
    Shu::vec2f J3 = n;
    Jacobian.Data[3][0] = J3.x;
    Jacobian.Data[4][0] = J3.y;
    f32 J4 = rB.Cross(n);
    Jacobian.Data[5][0] = J4;

    // Populating the second column of the jacobian. This is the tangent vector - for the friction.
    this->Friction = MAX(A->FrictionCoeff, B->FrictionCoeff);
    if(this->Friction > 0.0f)
    {
        // The tangent vector is the normal of the normal vector.
        const Shu::vec2f Tangent = n.Normal();
        Jacobian.Data[0][1] = -Tangent.x;
        Jacobian.Data[1][1] = -Tangent.y;
        Jacobian.Data[2][1] = rA.Cross(-Tangent);
        Jacobian.Data[3][1] = Tangent.x;
        Jacobian.Data[4][1] = Tangent.y;
        Jacobian.Data[5][1] = rB.Cross(Tangent);
    }

    // Shu::matMN<f32, 2, 6> Jt = Jacobian.Transposed();
    // f32 ImpulseMagnitude = CachedLambda[0];
    // Shu::vecN<f32, 6> ImpulseDirection = Jt.Rows[0];

    // Shu::vecN<f32, 6> FinalImpulses = ImpulseDirection * ImpulseMagnitude;

    // A->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[0], FinalImpulses[1]));
    // A->ApplyImpulseAngular(FinalImpulses[2]);
    // B->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[3], FinalImpulses[4]));
    // B->ApplyImpulseAngular(FinalImpulses[5]);

    // NOTE: Doing this for adding bounciness based on the coefficient of restitution between the two bodies.
    // if one wants "bouncy" collisions, the constraint for the post-impulse velocity does not require a zero
    // normal velocity, but a fraction of the pre-impulse normal velocity

    // Baumgarte Stabilization!
    const f32 beta = 0.2f;
    f32 C = (pB - pA).Dot(-n);
    C = MIN(0.0f, C + 0.02f);

    // Calculate pre-collision velocity along the collision normal
    Shu::vec2f LinearVa = A->Velocity.xy;
    Shu::vec2f AngularVa = Shu::Vec2f(-A->AngularVelocity * rA.y, A->AngularVelocity * rA.x); // <-- NOTE: this is W.Cross(R)
    Shu::vec2f Va = LinearVa + AngularVa;
    Shu::vec2f LinearVb = B->Velocity.xy;
    Shu::vec2f AngularVb = Shu::Vec2f(-B->AngularVelocity * rB.y, B->AngularVelocity * rB.x);
    Shu::vec2f Vb = LinearVb + AngularVb;

    Shu::vec2f RelV = Va - Vb;
    f32 RelVDotNormal = RelV.Dot(n);

    f32 Restitution = MIN(A->CoeffRestitution, B->CoeffRestitution);
    Bias = ((beta / dt) * C) + (Restitution*RelVDotNormal);
#endif
}

// IMPORTANT: NOTE: Check
// https://web.archive.org/web/20220801182628/http://myselph.de/gamePhysics/inequalityConstraints.html
void
penetration_constraint_2d::Solve()
{
#if 0
    const Shu::vecN<f32, 6> V = GetVelocities();
    const Shu::matN<f32, 6> InvM = GetInverseMassMatrix();

    const Shu::matMN<f32, 6, 2> J = Jacobian;
    const Shu::matMN<f32, 2, 6> Jt = J.Transposed();

    Shu::matN<f32, 2> lhs = ((Jt * InvM) * J);
    Shu::vecN<f32, 2> rhs = (V * J * -1.0f);
    rhs[0] -= Bias;
    // rhs[1] -= Bias;

    Shu::vecN<f32, 2> Lambda = Shu::LCP_GaussSeidel(lhs, rhs);

    Shu::vecN<f32, 2> OldLambda = this->CachedLambda;
    this->CachedLambda += Lambda;
    // This is the inequality constraint here. We wanted C(dot) derivation of C(error) in the contraint equation in
    // presolve >= 0. if C < 0 then bodies are colliding, and we want to calculate the impulse which makes C >=
    // 0(i.e. not colliding). " if lambda turned out to be negative, that was because the bodies would otherwise
    // separate ( C(dot) > 0 ), in which case we set lambda=0 to let them separate ( C(dot)> 0 )"
    // IMPORTANT: NOTE:
    // The Idea here is this:
    // We have a constraint here that states that the penetration between two bodies should be zero.
    // in math terms:- (pA - pB)dot(collision normal) should be greater than zero. (Check link for diagram)
    // this is the "C" constraint eq which we want to hold at all points.
    // C will be < 0 if there is collision.
    // if Lambda is negative here, that would mean that we will apply an attracting impulse between bodies A and B.
    // We want them to repel from one another if they are colliding, that is why setting the Lambda to zero here so
    // that the constraint eq C(dot) >= 0 holds.
    this->CachedLambda[0] = (this->CachedLambda[0] < 0.0f) ? 0.0f : this->CachedLambda[0];
    // check https://web.archive.org/web/20220801182625/http://myselph.de/gamePhysics/friction.html
    if(this->Friction > 0.0f) {
        const f32 Max = this->CachedLambda[0] * this->Friction;
        this->CachedLambda[1] = ClampToRange(Lambda[1], -Max, Max);
    }
    Lambda = this->CachedLambda - OldLambda;


    auto Impulses = J * Lambda;
    A->ApplyImpulseLinear(Shu::Vec2f(Impulses[0], Impulses[1]));
    A->ApplyImpulseAngular(Impulses[2]);
    B->ApplyImpulseLinear(Shu::Vec2f(Impulses[3], Impulses[4]));
    B->ApplyImpulseAngular(Impulses[5]);
#endif
}

void
penetration_constraint_2d::PostSolve()
{

}