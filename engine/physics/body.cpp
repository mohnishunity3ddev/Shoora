#include "body.h"
#include <renderer/vulkan/graphics/vulkan_graphics.h>

shoora_body::shoora_body(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Mass, f32 Restitution,
                         std::unique_ptr<shoora_shape> Shape, f32 InitialRotation)
{
    ASSERT(Mass >= 0.0f);
    ASSERT(Restitution > 0.0f && Restitution <= 1.0f);

    this->Color = Color;
    this->Position = Shu::Vec3f(InitPos, 0.0f);

    this->RotationRadians = InitialRotation;
    this->AngularVelocity = 0.0f;
    this->AngularAcceleration = 0.0f;

    this->FrictionCoeff = 0.7f;

    this->CoeffRestitution = Restitution;

    this->Mass = Mass;
    // Epsilon is an infinitesimally small value.
    this->InvMass = (this->Mass < FLT_EPSILON) ? 0.0f : (1.0f/this->Mass);
    this->I = (Mass * Shape->GetMomentOfInertia());
    ASSERT(this->I >= 0.0f);
    this->InvI = (this->I < FLT_EPSILON) ? 0.0f : (1.0f/this->I);

    this->Shape = std::move(Shape);
    this->Scale = (this->Shape)->GetDim();

    this->SumForces = this->Velocity = this->Acceleration = Shu::vec3f::Zero();
    this->SumTorques = 0.0f;
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
    else if(this->Shape->Type == RECT_2D)
    {
        Shu::rect2d rect = Shu::rect2d(Position.x, Position.y, this->Shape->GetDim().x, this->Shape->GetDim().y);
        if (ClickedWorldPos.x >= (rect.x - rect.width / 2) && ClickedWorldPos.x <= rect.x + rect.width / 2 &&
            ClickedWorldPos.y >= rect.y - rect.height / 2 && ClickedWorldPos.y <= rect.y + rect.height / 2)
        {
            Result = true;
        }
    }

    return Result;
}

b32
shoora_body::IsStatic() const
{
    b32 Result = (ABSOLUTE(this->InvMass - 0.0f) < FLT_EPSILON);
    return Result;
}

void
shoora_body::UpdateWorldVertices()
{
    if (Shape->Type == POLYGON_2D || Shape->Type == RECT_2D)
    {
        shoora_shape_polygon *polygon = (shoora_shape_polygon *)this->Shape.get();
        auto ModelMatrix = Shu::TRS(this->Position, this->Scale, this->RotationRadians * RAD_TO_DEG,
                                    Shu::Vec3f(0, 0, 1));
        if(polygon->isPrimitive)
        {
            polygon->UpdateWorldVertices(ModelMatrix);
        }
    }
}

// NOTE: Impulse denotes a change in velocity for the body.
void
shoora_body::ApplyImpulse(const Shu::vec2f &Impulse)
{
    if(this->IsStatic()) {
        return;
    }

    this->Velocity += Shu::Vec3f(Impulse * this->InvMass, 0.0f);
}

void
shoora_body::ApplyImpulse(const Shu::vec2f &Impulse, const Shu::vec2f &R)
{
    if(this->IsStatic()) {
        return;
    }

    this->Velocity += Shu::Vec3f(Impulse * this->InvMass, 0.0f);
    this->AngularVelocity += R.Cross(Impulse) * InvI;
}

void
shoora_body::AddForce(const Shu::vec2f &Force)
{
    this->SumForces += Shu::Vec3f(Force, 0.0f);
}
void
shoora_body::AddTorque(f32 Torque)
{
    this->SumTorques += Torque;
}

void
shoora_body::ClearForces()
{
    this->SumForces = Shu::vec3f::Zero();
}
void
shoora_body::ClearTorque()
{
    this->SumTorques = 0.0f;
}

void
shoora_body::IntegrateLinear(f32 DeltaTime)
{
    if(this->IsStatic()) {
        return;
    }

    this->Acceleration = this->SumForces * InvMass;
    this->Acceleration.z = 0.0f;

    this->Velocity += this->Acceleration * DeltaTime;
    this->Velocity.z = 0.0f;

    this->Position += this->Velocity * DeltaTime;
    this->Position.z = 0.0f;

    ClearForces();
}

void
shoora_body::IntegrateAngular(f32 dt)
{
    if(this->IsStatic()) {
        return;
    }

    // alpha = Tau / Moment of Inertia
    this->AngularAcceleration = this->SumTorques * InvI;

    this->AngularVelocity += this->AngularAcceleration * dt;
    this->RotationRadians += this->AngularVelocity * dt;

    ClearTorque();
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

void
shoora_body::DrawWireframe(const Shu::mat4f &model, f32 thickness, u32 color)
{
    shoora_mesh_filter *mesh = &this->Shape->Primitive->MeshFilter;
    shoora_primitive_type Type = this->Shape->Primitive->PrimitiveType;

    if (Type == shoora_primitive_type::CIRCLE)
    {
        for (i32 i = 1; i < mesh->VertexCount; ++i)
        {
            Shu::vec3f pos0 = mesh->Vertices[i - 1].Pos;
            Shu::vec3f pos1 = mesh->Vertices[i].Pos;

            Shu::vec2f p0 = (model * pos0).xy;
            Shu::vec2f p1 = (model * pos1).xy;
            shoora_graphics::DrawLine(p0, p1, color, 2.5f);
        }

        Shu::vec2f p0 = (model * mesh->Vertices[mesh->VertexCount - 1].Pos).xy;
        Shu::vec2f p1 = (model * mesh->Vertices[1].Pos).xy;
        shoora_graphics::DrawLine(p0, p1, color, thickness);
    }
    else if (Type == shoora_primitive_type::RECT_2D)
    {
        ASSERT(mesh->VertexCount == 4);
        auto *Poly = (shoora_shape_polygon *)this->Shape.get();
        auto *WorldVertices = Poly->WorldVertices;

        // NOTE: No need to do this for the polygon case, since we are already doing this for polygon in their
        // physics loop.
        // Shu::vec2f p0 = (model * mesh->Vertices[2].Pos).xy;
        // Shu::vec2f p1 = (model * mesh->Vertices[1].Pos).xy;

        Shu::vec2f p0 = WorldVertices[2].xy;
        Shu::vec2f p1 = WorldVertices[1].xy;
        shoora_graphics::DrawLine(p0, p1, color, thickness);
        p0 = WorldVertices[1].xy;
        p1 = WorldVertices[0].xy;
        shoora_graphics::DrawLine(p0, p1, color, thickness);
        p0 = WorldVertices[0].xy;
        p1 = WorldVertices[3].xy;
        shoora_graphics::DrawLine(p0, p1, color, thickness);
        p0 = WorldVertices[3].xy;
        p1 = WorldVertices[2].xy;
        shoora_graphics::DrawLine(p0, p1, color, thickness);
    }
}

void
shoora_body::Draw()
{
    if (this->Shape->isPrimitive)
    {
        shoora_primitive_info Info = this->Shape->Primitive->GetInfo();
        shoora_graphics::Draw(Info.IndexCount, Info.IndexOffset, Info.VertexOffset);
    }
    else
    {
        ASSERT(!"TO-DO");
    }
}
