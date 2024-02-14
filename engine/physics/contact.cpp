#include "contact.h"

void
contact::ResolvePenetration()
{
    if(this->A->IsStatic() && this->B->IsStatic()) {
        return;
    }

    const Shu::vec3f n = this->Normal;
    const f32 VRelDotN = (B->LinearVelocity - A->LinearVelocity).Dot(n);
    const f32 ImpulseMag = (2.0f*VRelDotN) / (A->InvMass + B->InvMass);
    const Shu::vec3f Impulse = n * ImpulseMag;

    A->ApplyImpulseLinear(Impulse);
    B->ApplyImpulseLinear(-Impulse);

    // if B has infinity mass, B.invMass = 0, therefore, dB will be zero.
    // So, we will only move A by the collision depth if B has infinite mass.
    f32 dA = (this->Depth * A->InvMass) / (A->InvMass + B->InvMass);
    f32 dB = (this->Depth * B->InvMass) / (A->InvMass + B->InvMass);

    A->Position -= this->Normal*dA;
    B->Position += this->Normal*dB;

    A->UpdateWorldVertices();
    B->UpdateWorldVertices();
}

void
contact::ResolveCollision()
{
    if (this->A->IsStatic() && this->B->IsStatic()) {
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