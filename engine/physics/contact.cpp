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

    // NOTE: Here, we move the two bodies along the contact normal so that they move away from each other so that
    // penetration depth between them is zero, and they are not colliding anymore.
    A->Position -= this->Normal*dA;
    B->Position += this->Normal*dB;
}

// NOTE: The aim is to calculate the Impulse vector acting on the two colliding bodies in this contact
// Impulse(J) acting on both bodies will be along the collision contact normal vector.
// Change in momentum in body A is dP(a) = Ma*V'a - Ma*Va. dP(a) is also Impulse acting along the collision normal.
// Hence,  J.N = Ma*V'a - Ma*Va; V'a = Va + (J.N/Ma) <-- (1)
// For body B, the impulse will be acting in the opposite direction as that of body A.
// Hence, -J.N = Mb*V'b - Mb*Vb; V'b = Vb - (J.N/Mb) <-- (2)
// Relative velocity of a w.r.t. b is Va - Vb pre-collision -> VRela = Va - Vb
// Relative velocity post-collision of a w.r.t. b will be V'a - V'b -> V'Rela = V'a - V'b
// For elastic collisions => V'Rela = -VRela,
// for normal scenarios => V'Rela = -E*VRela <---(3)
// where E is the coefficient of restitution, which is already present in the body structs.
// Subtracting (2) from (1), we get:
// V'Rela = VRela + (J.N)(1/Ma + 1/Mb)
// We want to find the relative velocity along the collision normal, so doing a dot product on both sides:
// V'Rela.N = VRela.N + J.N(1/Ma + 1/Mb)N
// Substituting (3) here, we get:
// -E*VRela.N = VRela.N + (J.N)(1/Ma + 1/Mb)N
// (- 1 - E)*VRela.N / (1/Ma + 1/Mb)*(N.N) = J
// N.N = 1 since N is unit vector, we get:
// J = (1 - E)*VRela.N / (1/Ma + 1/Mb)
// NOTE: This is the Impulse Method for resolving collisions
void
contact::ResolveCollision()
{
    if (this->A->IsStatic() && this->B->IsStatic()) {
        return;
    }

    // Separate out the bodies so that there is no penetration
    this->ResolvePenetration();

    f32 E = MIN(A->CoeffRestitution, B->CoeffRestitution);

    Shu::vec3f RelA = this->A->Velocity - this->B->Velocity;
    f32 RelADotN = Shu::Dot(RelA, this->Normal);

    f32 ImpulseMagnitude = (-(1.0f + E) * RelADotN) / (A->InvMass + B->InvMass );
    Shu::vec3f Impulse = this->Normal * ImpulseMagnitude;

    A->ApplyImpulse(Impulse.xy);
    B->ApplyImpulse(-Impulse.xy);
}