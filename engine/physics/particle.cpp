#include "particle.h"
#include <float.h>

void
shoora_particle::Initialize(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Size, f32 Mass,
                            const shoora_primitive_type Type, shoora_primitive *Primitive)
{
    if(ABSOLUTE(Mass - FLT_EPSILON) <= 0.0f) Mass = 1.0f;
    if(ABSOLUTE(Size - FLT_EPSILON) <= 0.0f) Size = 1.0f;

    this->Color = Color;
    this->Position = Shu::Vec3f(InitPos, 0.0f);
    this->Size = Size;
    this->Mass = Mass;
    this->InvMass = 1.0f / Mass;
    this->Primitive = Primitive;

    this->Velocity = this->SumForces = this->Acceleration = Shu::vec3f::Zero();
}

void
shoora_particle::AddForce(const Shu::vec2f &Force)
{
    this->SumForces += Shu::Vec3f(Force, 0.0f);
}

void
shoora_particle::ClearForces()
{
    this->SumForces = Shu::vec3f::Zero();
}

void
shoora_particle::Integrate(f32 DeltaTime)
{
    this->Acceleration = this->SumForces * InvMass;

    Shu::vec3f deltaV = this->Acceleration*DeltaTime;
    this->Position += this->Velocity*DeltaTime + this->Acceleration * (DeltaTime*DeltaTime * 0.5f);
    this->Velocity += deltaV;

    this->Velocity.z = this->Position.z = 0.0f;

    ClearForces();
}
