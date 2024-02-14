#include "body.h"
#include <renderer/vulkan/graphics/vulkan_graphics.h>

shoora_body::shoora_body(const Shu::vec3f &Color, const Shu::vec3f &InitPos, f32 Mass, f32 Restitution,
                         std::unique_ptr<shoora_shape> Shape, Shu::vec3f eulerAngles)
{
    ASSERT(Mass >= 0.0f);
    ASSERT(Restitution >= 0.0f && Restitution <= 1.0f);

    this->Color = Color;
    this->Position = InitPos;
    this->Rotation = Shu::QuatFromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);

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

    this->SumForces = this->LinearVelocity = this->Acceleration = Shu::vec3f::Zero();
    this->SumTorques = 0.0f;

    this->UpdateWorldVertices();
}

shoora_body::shoora_body(shoora_body &&other) noexcept
    : IsColliding(other.IsColliding), Position(std::move(other.Position)), LinearVelocity(std::move(other.LinearVelocity)),
      Acceleration(std::move(other.Acceleration)), Rotation(std::move(other.Rotation)),
      CoeffRestitution(other.CoeffRestitution), SumForces(std::move(other.SumForces)),
      SumTorques(other.SumTorques), FrictionCoeff(other.FrictionCoeff), Mass(other.Mass), InvMass(other.InvMass),
      I(other.I), InvI(other.InvI), Scale(std::move(other.Scale)), Color(std::move(other.Color)),
      Shape(std::move(other.Shape))
{
    other.IsColliding = false;
    other.Shape = nullptr;
}

shoora_body &
shoora_body::operator=(shoora_body &&other) noexcept
{
    if (this != &other)
    {
        IsColliding = other.IsColliding;
        Position = std::move(other.Position);
        LinearVelocity = std::move(other.LinearVelocity);
        Acceleration = std::move(other.Acceleration);
        Rotation = std::move(other.Rotation);
        CoeffRestitution = other.CoeffRestitution;
        SumForces = std::move(other.SumForces);
        SumTorques = other.SumTorques;
        FrictionCoeff = other.FrictionCoeff;
        Mass = other.Mass;
        InvMass = other.InvMass;
        I = other.I;
        InvI = other.InvI;
        Scale = std::move(other.Scale);
        Color = std::move(other.Color);
        Shape = std::move(other.Shape);

        other.IsColliding = false;
        other.Shape = nullptr;
    }
    return *this;
}

Shu::vec3f
shoora_body::WorldToLocalSpace(const Shu::vec3f &PointWS) const
{
    // NOTE: Inverse of the model matrix. We are doing the 2d version of that here.
    Shu::vec3f invTranslation = PointWS - this->Position;

    Shu::vec3f invRotation = Shu::QuatRotateVec(Shu::QuatInverse(this->Rotation), invTranslation);

    f32 invScaleX = NearlyEqual(Scale.x, 0.0f) ? 0.0f : (1.0f/this->Scale.x);
    f32 invScaleY = NearlyEqual(Scale.y, 0.0f) ? 0.0f : (1.0f/this->Scale.y);
    f32 invScaleZ = NearlyEqual(Scale.z, 0.0f) ? 0.0f : (1.0f/this->Scale.z);
    Shu::vec3f invScaleXYZ = Shu::Vec3f(invScaleX, invScaleY, invScaleZ);

    Shu::vec3f invScale = invRotation * invScaleXYZ;

    return invScale;
}

Shu::vec3f
shoora_body::LocalToWorldSpace(const Shu::vec3f &PointLS) const
{
    // NOTE: This is the 2d version of the model matrix we calculate using Shu::TRS()
    Shu::vec3f Scaled = this->Scale * PointLS;
    Shu::vec3f Rotated = Shu::QuatRotateVec(this->Rotation, Scaled);
    Shu::vec3f Translated = Rotated + this->Position;

    return Translated;
}

b32
shoora_body::CheckIfClicked(const Shu::vec2f &ClickedWorldPos)
{
    b32 Result = false;
    if (this->Shape->Type == shoora_mesh_type::CIRCLE)
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

shoora_body::~shoora_body()
{
    LogUnformatted("shoora_body desctructor called!\n");
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
#if 0
    if (Shape->Type == POLYGON_2D || Shape->Type == RECT_2D)
    {
        shoora_shape_polygon *polygon = (shoora_shape_polygon *)this->Shape.get();
        // TODO: Calculate Model Matrix here.
        polygon->UpdateWorldVertices(this->Model);
    }
#endif
}

// NOTE: Impulse denotes a change in velocity for the body.
void
shoora_body::ApplyImpulseLinear(const Shu::vec3f &Impulse)
{
    if(this->IsStatic()) {
        return;
    }

    this->LinearVelocity += Impulse * this->InvMass;
}

void
shoora_body::ApplyImpulseAngular(f32 Impulse)
{
#if 0
    if(this->IsStatic()) {
        return;
    }

    this->AngularVelocity += Impulse * this->InvI;
#endif
}

void
shoora_body::ApplyImpulseAtPoint(const Shu::vec2f &Impulse, const Shu::vec2f &R)
{
#if 0
    if(this->IsStatic()) {
        return;
    }

    this->Velocity += Shu::Vec3f(Impulse * this->InvMass, 0.0f);
    this->AngularVelocity += R.Cross(Impulse) * InvI;
#endif
}

void
shoora_body::AddForce(const Shu::vec3f &Force)
{
    this->SumForces += Force;
}
void
shoora_body::AddTorque(f32 Torque)
{
    this->SumTorques += Torque;
}

void
shoora_body::ClearForces()
{
    this->SumForces = Shu::Vec3f(0.0f);
}
void
shoora_body::ClearTorques()
{
    this->SumTorques = 0.0f;
}

void
shoora_body::IntegrateForces(const f32 deltaTime)
{
    if(IsStatic()) {
        return;
    }

    this->Acceleration = this->SumForces * this->InvMass;

    // alpha = Tau / Moment of Inertia
#if 0
    this->AngularAcceleration = this->SumTorques * InvI;
#endif

    ClearForces();
    ClearTorques();
}

void
shoora_body::IntegrateVelocities(const f32 deltaTime)
{
    if(IsStatic()) {
        return;
    }

    this->LinearVelocity += this->Acceleration * deltaTime;
    this->Position += this->LinearVelocity * deltaTime;

#if 0
    this->AngularVelocity += this->AngularAcceleration * deltaTime;
    this->RotationRadians += this->AngularVelocity * deltaTime;
#endif

    this->UpdateWorldVertices();
}

void
shoora_body::KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor)
{
    Shu::vec2f boundX = Shu::Vec2f(ViewBounds.x - ViewBounds.width / 2, ViewBounds.x + ViewBounds.width / 2);
    Shu::vec2f boundY = Shu::Vec2f(ViewBounds.y - ViewBounds.height / 2, ViewBounds.y + ViewBounds.height / 2);

    Shu::vec2f Dim = this->Shape->GetDim().xy;
    switch(this->Shape->Type)
    {
        case shoora_mesh_type::CIRCLE: { }
        break;
        case shoora_mesh_type::RECT_2D:
        break;

        SHU_INVALID_DEFAULT
    }

    if ((this->Position.y - Dim.y) < boundY.x)
    {
        this->Position.y = boundY.x + Dim.y;
        this->LinearVelocity.y *= DampFactor;
    }
    if ((this->Position.y + Dim.y) > boundY.y)
    {
        this->Position.y = boundY.y - Dim.y;
        this->LinearVelocity.y *= DampFactor;
    }
    if ((this->Position.x - Dim.x) < boundX.x)
    {
        this->Position.x = boundX.x + Dim.x;
        this->LinearVelocity.x *= DampFactor;
    }
    if ((this->Position.x + Dim.x) > boundX.y)
    {
        this->Position.x = boundX.y - Dim.x;
        this->LinearVelocity.x *= DampFactor;
    }
}

void
shoora_body::DrawWireframe(const Shu::mat4f &model, f32 thickness, u32 color)
{
    shoora_mesh_filter *mesh = this->Shape->MeshFilter;
    shoora_mesh_type Type = this->Shape->Type;

    if(Type == shoora_mesh_type::CIRCLE)
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
    else if (Type == shoora_mesh_type::POLYGON_2D || Type == shoora_mesh_type::RECT_2D)
    {
        ASSERT(mesh->VertexCount >= 3);
        auto *Poly = (shoora_shape_polygon *)this->Shape.get();
        auto *WorldVertices = Poly->WorldVertices;

        // NOTE: No need to calculate TRS here since that's already been done in the physics loop for polygons.
        for (i32 i = 0; i < mesh->VertexCount; ++i)
        {
            Shu::vec2f p0 = WorldVertices[i].xy;
            Shu::vec2f p1 = WorldVertices[(i+1) % mesh->VertexCount].xy;
            shoora_graphics::DrawLine(p0, p1, color, thickness);
        }
    }
}

void
shoora_body::Draw()
{
#if 0
    if(this->Shape->isPrimitive)
    {
        auto *mesh = (shoora_mesh *)this->Shape->MeshFilter;
        shoora_mesh_info Info = mesh->GetInfo();
        shoora_graphics::Draw(Info.IndexCount, Info.IndexOffset, Info.VertexOffset);
    }
    else
    {
        ASSERT(!"TO-DO");
    }
#endif
    auto *mesh = (shoora_mesh *)this->Shape->MeshFilter;
    shoora_mesh_info Info = mesh->GetInfo();
    shoora_graphics::Draw(Info.IndexCount, Info.IndexOffset, Info.VertexOffset);
}
