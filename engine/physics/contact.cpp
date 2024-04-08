#include "contact.h"

void
contact::ResolvePenetration()
{
    if(this->ReferenceBodyA->IsStatic() && this->IncidentBodyB->IsStatic()) {
        return;
    }

#if 0
    auto as = this->ReferenceBodyA->Shape->GetType();
    auto bs = this->IncidentBodyB->Shape->GetType();
    b32 ShouldCheck = as == CONVEX || as == CONVEX_DIAMOND || bs == CONVEX || bs == CONVEX_DIAMOND;
#endif

    // Bounciness
    f32 Elasticity = ClampToRange(ReferenceBodyA->CoeffRestitution * IncidentBodyB->CoeffRestitution, 0.0f, 1.0f);
    const shu::vec3f n = this->Normal;

    // I^-1
    const shu::mat3f invWorldInertiaA = ReferenceBodyA->GetInverseInertiaTensorWS();
    const shu::mat3f invWorldInertiaB = IncidentBodyB->GetInverseInertiaTensorWS();

    // Useful in calculating torque/angular impulses.
    shu::vec3f rA = this->ReferenceHitPointA - ReferenceBodyA->GetCenterOfMassWS();
    shu::vec3f rB = this->IncidentHitPointB - IncidentBodyB->GetCenterOfMassWS();

    // factors due to rotation.
    const shu::vec3f angularJA = (rA.Cross(n) * invWorldInertiaA).Cross(rA);
    const shu::vec3f angularJB = (rB.Cross(n) * invWorldInertiaB).Cross(rB);
    f32 angularFactor = (angularJA + angularJB).Dot(n);

    // NOTE: Calculating the collision impulse.

    // NOTE: Velocities before the collision
    const shu::vec3f VA = ReferenceBodyA->LinearVelocity + ReferenceBodyA->AngularVelocity.Cross(rA);
    const shu::vec3f VB = IncidentBodyB->LinearVelocity + IncidentBodyB->AngularVelocity.Cross(rB);
    shu::vec3f VRel = VB - VA;
    // Relative velocity component along the normal
    f32 VRelDotN = VRel.Dot(n);

    f32 numerator = (1.0f + Elasticity) * VRelDotN;
    f32 denominator = ReferenceBodyA->InvMass + IncidentBodyB->InvMass + angularFactor;
    if(!NearlyEqual(denominator, 0.0f))
    {
        f32 ImpulseNormalMagnitude = numerator / denominator;
        const shu::vec3f ImpulseNormal = n * ImpulseNormalMagnitude;

        // Applying Linear + Angular impulses to the contact points on the two bodies.
        ReferenceBodyA->ApplyImpulseAtPoint(ImpulseNormal, ReferenceHitPointA);
        IncidentBodyB->ApplyImpulseAtPoint(-ImpulseNormal, IncidentHitPointB);
    }

    // NOTE: Impulse due to friction
    // Check https://web.archive.org/web/20211022100604/http://myselph.de/gamePhysics/friction.html
    // IMPORTANT: NOTE:
    // The main idea here is that the relative velocity of these two touching bodies are decremented by a factor of
    // Mu which is the coeff of restitution, so that the system keeps losing energy. And the direction of the
    // friction impulse is along the tangent to the collision normal so that it is opposing the direction of the
    // velocity. All the other impulse calculation is the same as we did for the collision impulse above.
    const f32 frictionA = ReferenceBodyA->FrictionCoeff;
    const f32 frictionB = IncidentBodyB->FrictionCoeff;
    const f32 friction = frictionA * frictionB;
    ASSERT(friction >= 0.0f && friction < 1.0f);

    // NOTE: find the component of the velocity that is normal to the collision normal / Tangential to the
    // collision point.
    shu::vec3f vN = VRelDotN * n;
    shu::vec3f vT = VRel - vN;
    shu::vec3f VRelTangential = shu::Normalize(vT);

    const shu::vec3f angularFA = (rA.Cross(VRelTangential) * invWorldInertiaA).Cross(rA);
    const shu::vec3f angularFB = (rB.Cross(VRelTangential) * invWorldInertiaB).Cross(rB);
    f32 angularFactorFriction = (angularFA + angularFB).Dot(VRelTangential);

    if(!NearlyEqual(angularFactorFriction, 0.0f))
    {
        f32 ImpulseFrictionMagnitude = (friction / angularFactorFriction);
        const shu::vec3f ImpulseFriction = VRelTangential * ImpulseFrictionMagnitude;

        ReferenceBodyA->ApplyImpulseAtPoint(ImpulseFriction, ReferenceHitPointA);
        IncidentBodyB->ApplyImpulseAtPoint(-ImpulseFriction, IncidentHitPointB);
    }

    // NOTE: Manually Move out bodies so that they are not colliding using the projection method ONLY when the
    // timeOfImpact is zero.
    // IMPORTANT: Explanation of above statement:
    // toi = 0 means CCD(Continuous Collision Detection) did not need to advance time ahead by the fraction of
    // deltaTime. if toi is non-zero, that means the bodies in the CCD Routine were moved JUST enough so that they
    // are just touching each other - meaning the penetration depth is zero. If the penetration depth is zero, then
    // we do not need to manually move the objects to remove the penetration since there is none in the first
    // place. If However, the toi is zero, then that means that there might be some penetration existing between
    // the bodies. Only in this case, we need to move the objects manually which is what we are doing here.
    if(NearlyEqual(this->TimeOfImpact, 0.0f))
    {
        // if B has infinity mass, B.invMass = 0, therefore, dB will be zero.
        // So, we will only move A by the collision depth if B has infinite mass.
        f32 dA = (this->Depth * ReferenceBodyA->InvMass) / (ReferenceBodyA->InvMass + IncidentBodyB->InvMass);
        f32 dB = (this->Depth * IncidentBodyB->InvMass) / (ReferenceBodyA->InvMass + IncidentBodyB->InvMass);

        ReferenceBodyA->Position -= this->Normal*dA;
        IncidentBodyB->Position += this->Normal*dB;

#if 0
        if(SHU_ABSOLUTE(dA) > 1.0f)
        {
            int x = 0;
        }
        if(ShouldCheck && (ReferenceBodyA->IsStatic() || IncidentBodyB->IsStatic()))
        {
            LogDebug("dA: %f || dB: %f.\n", dA, dB);
        }
#endif

        ReferenceBodyA->UpdateWorldVertices();
        IncidentBodyB->UpdateWorldVertices();
    }
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

b32
CompareContacts(const contact &c1, const contact &c2)
{
    b32 Result = false;
    if(c1.TimeOfImpact <= c2.TimeOfImpact)
    {
        Result = true;
    }

    return Result;
#if 0
    if(c1.TimeOfImpact == c2.TimeOfImpact)
    {
        return 0;
    }
    return 1;
#endif
}
