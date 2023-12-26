#include "force.h"


// NOTE: Drag Formula
// Drag = 0.5f*(density)*(Kd)*(Area)*(magnitude(v)^2)*(-v.direction())
Shu::vec2f
force::GenerateDragForce(const shoora_particle *Particle, f32 DragCoefficient)
{
    Shu::vec2f DragForce = Shu::Vec2f(0, 0);

    if(Particle->Velocity.SqMagnitude() > 0)
    {
        Shu::vec3f VelocityNormalized = Shu::Normalize(Particle->Velocity);
        Shu::vec2f DragDirection = Shu::Vec2f(VelocityNormalized)*-1.0f;

        f32 DragMagnitude = DragCoefficient * Particle->Velocity.SqMagnitude();

        DragForce = DragDirection * DragMagnitude;
    }

    return DragForce;
}

Shu::vec2f
force::GenerateFrictionForce(const shoora_particle *Particle, f32 FrictionCoefficient)
{
    Shu::vec2f FrictionForce = Shu::Vec2f(0.0f);

    Shu::vec3f VelocityNormalized = Shu::Normalize(Particle->Velocity);
    Shu::vec2f FrictionDir = Shu::Vec2f(VelocityNormalized) * -1.0f;

    f32 FrictionMagnitude = FrictionCoefficient;

    FrictionForce = FrictionDir * FrictionMagnitude;

    return FrictionForce;
}
