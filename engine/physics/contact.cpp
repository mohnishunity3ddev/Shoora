#include "contact.h"

void
contact::ResolvePenetration()
{
    if(this->ReferenceBodyA->IsStatic() && this->IncidentBodyB->IsStatic()) {
        return;
    }

    f32 Elasticity = ClampToRange(ReferenceBodyA->CoeffRestitution*IncidentBodyB->CoeffRestitution, 0.0f, 1.0f);
    const Shu::vec3f n = this->Normal;

    const Shu::mat3f invWorldInertiaA = ReferenceBodyA->GetInverseInertiaTensorWS();
    const Shu::mat3f invWorldInertiaB = IncidentBodyB->GetInverseInertiaTensorWS();

    Shu::vec3f rA = this->ReferenceContactPointA - ReferenceBodyA->GetCenterOfMassWS();
    Shu::vec3f rB = this->IncidentContactPointB - IncidentBodyB->GetCenterOfMassWS();

    const Shu::vec3f angularJA = (rA.Cross(n) * invWorldInertiaA).Cross(rA);
    const Shu::vec3f angularJB = (rB.Cross(n) * invWorldInertiaB).Cross(rB);
    f32 angularFactor = (angularJA + angularJB).Dot(n);

    // NOTE: Calculating the collision impulse.

    // NOTE: Velocities before the collision
    const Shu::vec3f VA = ReferenceBodyA->LinearVelocity + ReferenceBodyA->AngularVelocity.Cross(rA);
    const Shu::vec3f VB = IncidentBodyB->LinearVelocity + IncidentBodyB->AngularVelocity.Cross(rB);
    Shu::vec3f VRel = VB - VA;
    // Relative velocity component along the normal
    f32 VRelDotN = VRel.Dot(n);

    f32 numerator = (1.0f + Elasticity) * VRelDotN;
    f32 denominator = ReferenceBodyA->InvMass + IncidentBodyB->InvMass + angularFactor;
    ASSERT(denominator > 0.0f);
    f32 ImpulseMagnitude = numerator / denominator;
    const Shu::vec3f Impulse = n * ImpulseMagnitude;

    ReferenceBodyA->ApplyImpulseAtPoint(Impulse, ReferenceContactPointA);
    IncidentBodyB->ApplyImpulseAtPoint(-Impulse, IncidentContactPointB);

    // NOTE: Impulse due to friction

    const f32 frictionA = ReferenceBodyA->FrictionCoeff;
    const f32 frictionB = IncidentBodyB->FrictionCoeff;
    const f32 friction = frictionA * frictionB;
    ASSERT(friction >= 0.0f && friction < 1.0f);

    // NOTE: find the component of the velocity that is normal to the collision normal / Tangential to the
    // collision point.
    Shu::vec3f vN = VRelDotN * n;
    Shu::vec3f vT = VRel - vN;
    Shu::vec3f VRelTangential = Shu::Normalize(vT);

    const Shu::vec3f angularFA = (rA.Cross(VRelTangential) * invWorldInertiaA).Cross(rA);
    const Shu::vec3f angularFB = (rB.Cross(VRelTangential) * invWorldInertiaB).Cross(rB);
    f32 angularFactorFriction = (angularFA + angularFB).Dot(VRelTangential);
    Shu::vec3f ImpulseFriction = VRelTangential * (friction / angularFactorFriction);

    ReferenceBodyA->ApplyImpulseAtPoint(ImpulseFriction, ReferenceContactPointA);
    IncidentBodyB->ApplyImpulseAtPoint(-ImpulseFriction, IncidentContactPointB);

    // if B has infinity mass, B.invMass = 0, therefore, dB will be zero.
    // So, we will only move A by the collision depth if B has infinite mass.
    f32 dA = (this->Depth * ReferenceBodyA->InvMass) / (ReferenceBodyA->InvMass + IncidentBodyB->InvMass);
    f32 dB = (this->Depth * IncidentBodyB->InvMass) / (ReferenceBodyA->InvMass + IncidentBodyB->InvMass);

    ReferenceBodyA->Position -= this->Normal*dA;
    IncidentBodyB->Position += this->Normal*dB;

    ReferenceBodyA->UpdateWorldVertices();
    IncidentBodyB->UpdateWorldVertices();
}

void
contact::ResolveCollision()
{
    if (this->ReferenceBodyA->IsStatic() && this->IncidentBodyB->IsStatic()) {
        return;
    }

    // Separate out the bodies so that there is no penetration
    this->ResolvePenetration();




#if 0
    f32 E = MIN(A->CoeffRestitution, B->CoeffRestitution);
    f32 F = MIN(A->FrictionCoeff, B->FrictionCoeff);

    // calculate the relative velocity of contact point A wrt the contact point of body B
    // v = linearV + (AngularV X ra) -> ra is the distance vector from center of body to the collision contact point.
    Shu::vec2f Ra = End.xy - A->Position.xy;
    Shu::vec2f Rb = Start.xy - B->Position.xy;

    Shu::vec2f LinearVa = A->Velocity.xy;
    // The Omega Vector(W) points in the Z Direction, which is the axis of rotation in the 2D case.
    // [0]   [Ra.x]   [0.0 - W*Ra.y]
    // [0] X [Ra.y] = [W*Ra.x - 0*0]
    // [W]   [0]      [0*Ra.y - 0*Ra.x]
    Shu::vec2f AngularVa = Shu::Vec2f(-A->AngularVelocity * Ra.y, A->AngularVelocity * Ra.x); // <-- NOTE: this is W.Cross(R)
    Shu::vec2f Va = LinearVa + AngularVa;

    Shu::vec2f LinearVb = B->Velocity.xy;
    Shu::vec2f AngularVb = Shu::Vec2f(-B->AngularVelocity * Rb.y, B->AngularVelocity * Rb.x);
    Shu::vec2f Vb = LinearVb + AngularVb;

    Shu::vec2f RelV = Va - Vb;

    Shu::vec2f N = this->Normal.xy;
    f32 RelVDotNormal = RelV.Dot(N);
    // IMPORTANT: NOTE: See physics/concepts/concepts.md
    // f32 Denominator = A->InvMass + B->InvMass +
    f32 ImpulseNDenom = A->InvMass + B->InvMass +
                        Ra.Cross(N)*Ra.Cross(N) * A->InvI +
                        Rb.Cross(N)*Rb.Cross(N) * B->InvI;
    f32 ImpulseN = (-(1.0f + E) * RelVDotNormal) / ImpulseNDenom;
    // Impulse acts along the normal of the collision.
    Shu::vec2f ImpulseAlongNormal = N * ImpulseN;

    Shu::vec2f Tangent = N.Normal();
    f32 RelVDotTangent = RelV.Dot(Tangent);
    f32 ImpulseTDenom = A->InvMass + B->InvMass +
                        Ra.Cross(Tangent)*Ra.Cross(Tangent) * A->InvI +
                        Rb.Cross(Tangent)*Rb.Cross(Tangent) * B->InvI;
    f32 ImpulseT = (F * -(1.0f + E) * RelVDotTangent) / ImpulseTDenom;
    Shu::vec2f ImpulseAlongTangent = Tangent * ImpulseT;

    Shu::vec2f Impulse = ImpulseAlongNormal + ImpulseAlongTangent;

    A->ApplyImpulseAtPoint(Impulse, Ra);
    B->ApplyImpulseAtPoint(-Impulse, Rb);
#endif
}