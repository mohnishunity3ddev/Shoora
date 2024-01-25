#include "contact.h"

// NOTE:
// If two bodies have collided and penetrated inside of each other in a frame, we want to resolve that penetration,
// so that the bodies are now not colliding/penetrating. Here, we do this using the projection method which is we
// change the position of both bodies so that they move along the contact normals until their penetration depth
// becomes zero(in other words, not colliding). How much we move the body depends on its mass. the more the mass,
// the less we move it. The sum of the total distance moved by both of the bodies is equal to the penetration
// depth. Instead of using the mass of the bodies, we use their inverseMass as done by most physics engines.
// NOTE: This is the Projection Method for resolving collisions.
void
contact::ResolvePenetration()
{
    if(this->A->IsStatic() && this->B->IsStatic()) {
        return;
    }

    f32 dA = (this->Depth * A->InvMass) / (A->InvMass + B->InvMass);
    f32 dB = (this->Depth * B->InvMass) / (A->InvMass + B->InvMass);

    if(this->Depth > 50) {
        int x = 0;
    }

    // LogWarn("Position Before resolving is: [%.2f, %.2f]\n", B->Position.x, B->Position.y);
    // NOTE: Here, we move the two bodies along the contact normal so that they move away from each other so that
    // penetration depth between them is zero, and they are not colliding anymore.
    A->Position -= this->Normal*dA;
    B->Position += this->Normal*dB;
    // LogDebug("Position After resolving is: [%.2f, %.2f]\n", B->Position.x, B->Position.y);

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

    A->ApplyImpulse(Impulse, Ra);
    B->ApplyImpulse(-Impulse, Rb);
}