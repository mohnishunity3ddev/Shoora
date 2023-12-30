#include "body.h"
#include <float.h>

void
shoora_body::Initialize(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Mass, shoora_shape *Shape)
{
    if(ABSOLUTE(Mass - FLT_EPSILON) <= 0.0f) Mass = 1.0f;

    this->Color = Color;
    this->Position = Shu::Vec3f(InitPos, 0.0f);
    this->Mass = Mass;
    this->InvMass = 1.0f / Mass;
    this->Shape = Shape;
    this->Scale = Shape->GetDim();

    this->Velocity = this->SumForces = this->Acceleration = Shu::vec3f::Zero();
}

b32
shoora_body::CheckIfClicked(const Shu::vec2f &ClickedWorldPos)
{
    b32 Result = false;
    if (this->Shape->Type == shoora_primitive_type::CIRCLE)
    {
        Shu::vec2f l = ClickedWorldPos - Shu::ToVec2(this->Position);
        u32 Radius = this->Shape->GetDim().x;
        Result = (l.SqMagnitude() < (Radius*Radius));
    }

    return Result;
}

void
shoora_body::IntegrateAngular(f32 dt)
{
    this->AngularVelocity += this->AngularAcceleration * dt;
    this->Rotation += this->AngularVelocity * dt;
}

void
shoora_body::AddForce(const Shu::vec2f &Force)
{
    this->SumForces += Shu::Vec3f(Force, 0.0f);
}

void
shoora_body::ClearForces()
{
    this->SumForces = Shu::vec3f::Zero();
}

void
shoora_body::IntegrateLinear(f32 DeltaTime)
{
    this->Acceleration = this->SumForces * InvMass;
    this->Acceleration.z = 0.0f;

    this->Velocity += this->Acceleration * DeltaTime;
    this->Velocity.z = 0.0f;

    this->Position += this->Velocity * DeltaTime;
    this->Position.z = 0.0f;

    ClearForces();
}

void
shoora_body::KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor)
{
    Shu::vec2f boundX = Shu::Vec2f(ViewBounds.x - ViewBounds.width / 2, ViewBounds.x + ViewBounds.width / 2);
    Shu::vec2f boundY = Shu::Vec2f(ViewBounds.y - ViewBounds.height / 2, ViewBounds.y + ViewBounds.height / 2);

    Shu::vec2f Dim = this->Shape->GetDim().xy;
    switch(this->Shape->Type)
    {
        case shoora_primitive_type::CIRCLE: { }
        break;
        case shoora_primitive_type::RECT_2D:
        case shoora_primitive_type::TRIANGLE: { Dim *= 0.5f; }
        break;

        SHU_INVALID_DEFAULT
    }

    if ((this->Position.y - Dim.y) < boundY.x)
    {
        this->Position.y = boundY.x + Dim.y;
        this->Velocity.y *= DampFactor;
    }
    if ((this->Position.y + Dim.y) > boundY.y)
    {
        this->Position.y = boundY.y - Dim.y;
        this->Velocity.y *= DampFactor;
    }
    if ((this->Position.x - Dim.x) < boundX.x)
    {
        this->Position.x = boundX.x + Dim.x;
        this->Velocity.x *= DampFactor;
    }
    if ((this->Position.x + Dim.x) > boundX.y)
    {
        this->Position.x = boundX.y - Dim.x;
        this->Velocity.x *= DampFactor;
    }
}
