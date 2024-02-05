#include "constraint.h"

penetration_constraint_2d::penetration_constraint_2d() : constraint_2d()
{
    Jacobian.Zero();
    CachedLambda.Zero();
    Bias = 0.0f;
}

penetration_constraint_2d::penetration_constraint_2d(shoora_body *a, shoora_body *b, const contact &contact)
    : penetration_constraint_2d()
{
    this->A = a;
    this->B = b;
    this->AnchorPointLS_A = a->WorldToLocalSpace(contact.Start.xy);
    this->AnchorPointLS_B = b->WorldToLocalSpace(contact.End.xy);
    this->Normal = a->WorldToLocalSpace(contact.Normal.xy);
}

void
penetration_constraint_2d::PreSolve(const f32 dt)
{
    Jacobian.Zero();

    const Shu::vec2f pA = A->LocalToWorldSpace(AnchorPointLS_A);
    const Shu::vec2f pB = B->LocalToWorldSpace(AnchorPointLS_B);
    const Shu::vec2f n = A->LocalToWorldSpace(Normal);

    const Shu::vec2f rA = pA - A->Position.xy;
    const Shu::vec2f rB = pB - B->Position.xy;

    Shu::vec2f J1 = -n;
    Jacobian.Data[0][0] = J1.x;
    Jacobian.Data[1][0] = J1.y;
    f32 J2 = -rA.Cross(n);
    Jacobian.Data[2][0] = J2;

    // This is multiplied with B's linear velocity.
    Shu::vec2f J3 = n;
    Jacobian.Data[3][0] = J3.x;
    Jacobian.Data[4][0] = J3.y;
    f32 J4 = rB.Cross(n);
    Jacobian.Data[5][0] = J4;

    /*
    auto Jt = Jacobian.Transposed();
    f32 ImpulseMagnitude = CachedLambda[0];
    Shu::vecN<f32, 6> ImpulseDirection = Jt.Rows[0];

    Shu::vecN<f32, 6> FinalImpulses = ImpulseDirection * ImpulseMagnitude;

    A->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[0], FinalImpulses[1]));
    A->ApplyImpulseAngular(FinalImpulses[2]);
    B->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[3], FinalImpulses[4]));
    B->ApplyImpulseAngular(FinalImpulses[5]);
    */

    const f32 beta = 0.1f;
    f32 C = (pB - pA).Dot(-n);
    C = MIN(0.0f, C + 0.01f);
    Bias = (beta / dt) * C;
}

void
penetration_constraint_2d::Solve()
{
    const Shu::vecN<f32, 6> V = GetVelocities();
    const Shu::matN<f32, 6> InvM = GetInverseMassMatrix();

    const auto J = Jacobian;
    const auto Jt = J.Transposed();

    Shu::matN<f32, 1> lhs = ((Jt * InvM) * J);
    Shu::vecN<f32, 1> rhs = (V * J * -1.0f);
    rhs[0] -= Bias;

    Shu::vecN<f32, 1> Lambda = Shu::LCP_GaussSeidel(lhs, rhs);

    f32 ImpulseMagnitude = Lambda[0];
    Shu::vecN<f32, 6> ImpulseDirection = Jt.Rows[0];

    Shu::vecN<f32, 6> FinalImpulses = ImpulseDirection * ImpulseMagnitude;

    A->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[0], FinalImpulses[1]));
    A->ApplyImpulseAngular(FinalImpulses[2]);
    B->ApplyImpulseLinear(Shu::Vec2f(FinalImpulses[3], FinalImpulses[4]));
    B->ApplyImpulseAngular(FinalImpulses[5]);

    /*this->CachedLambda += Lambda;*/
}

void
penetration_constraint_2d::PostSolve()
{

}