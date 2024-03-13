#include "body.h"
#include <renderer/vulkan/graphics/vulkan_graphics.h>

shoora_body::shoora_body(const shu::vec3f &Color, const shu::vec3f &InitPos, f32 Mass, f32 Restitution,
                         shoora_shape *Shape, shu::vec3f eulerAngles)
{
    ASSERT(Mass >= 0.0f);
    ASSERT(Restitution >= 0.0f && Restitution <= 1.0f);

    this->Color = Color;
    this->Position = InitPos;
    this->Rotation = shu::QuatFromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);

    this->FrictionCoeff = 0.7f;

    this->CoeffRestitution = Restitution;

    this->Mass = Mass;
    // Epsilon is an infinitesimally small value.
    this->InvMass = (this->Mass < FLT_EPSILON) ? 0.0f : (1.0f/this->Mass);
    this->InertiaTensor = (Shape->InertiaTensor() * Mass);
    this->InverseInertiaTensor = this->InertiaTensor.IsZero() ? shu::Mat3f(0.0f) : this->InertiaTensor.Inverse();

    this->Shape = Shape;
    this->Scale = (this->Shape)->GetDim();

    this->SumForces = this->LinearVelocity = this->Acceleration = this->AngularVelocity = shu::vec3f::Zero();

    this->SumTorques = 0.0f;

    this->UpdateWorldVertices();
}

shoora_body::shoora_body(shoora_body &&other) noexcept
    : IsColliding(other.IsColliding), Position(std::move(other.Position)),
      LinearVelocity(std::move(other.LinearVelocity)), AngularVelocity(std::move(other.AngularVelocity)),
      Acceleration(std::move(other.Acceleration)), Rotation(std::move(other.Rotation)),
      CoeffRestitution(other.CoeffRestitution), SumForces(std::move(other.SumForces)),
      SumTorques(other.SumTorques), FrictionCoeff(other.FrictionCoeff), Mass(other.Mass), InvMass(other.InvMass),
      InertiaTensor(other.InertiaTensor), InverseInertiaTensor(other.InverseInertiaTensor),
      Scale(std::move(other.Scale)), Color(std::move(other.Color)), Shape(other.Shape)
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
        AngularVelocity = std::move(other.AngularVelocity);
        Acceleration = std::move(other.Acceleration);
        Rotation = std::move(other.Rotation);
        CoeffRestitution = other.CoeffRestitution;
        SumForces = std::move(other.SumForces);
        SumTorques = other.SumTorques;
        FrictionCoeff = other.FrictionCoeff;
        Mass = other.Mass;
        InvMass = other.InvMass;
        InertiaTensor = other.InertiaTensor;
        InverseInertiaTensor = other.InverseInertiaTensor;
        Scale = std::move(other.Scale);
        Color = std::move(other.Color);
        Shape = std::move(other.Shape);

        other.IsColliding = false;
        other.Shape = nullptr;
    }
    return *this;
}

shu::vec3f
shoora_body::WorldToLocalSpace(const shu::vec3f &PointWS) const
{
    // NOTE: Inverse of the model matrix. We are doing the 2d version of that here.
    shu::vec3f invTranslation = PointWS - this->Position;

    shu::vec3f invRotation = shu::QuatRotateVec(shu::QuatInverse(this->Rotation), invTranslation);

    f32 invScaleX = NearlyEqual(Scale.x, 0.0f) ? 0.0f : (1.0f/this->Scale.x);
    f32 invScaleY = NearlyEqual(Scale.y, 0.0f) ? 0.0f : (1.0f/this->Scale.y);
    f32 invScaleZ = NearlyEqual(Scale.z, 0.0f) ? 0.0f : (1.0f/this->Scale.z);
    shu::vec3f invScaleXYZ = shu::Vec3f(invScaleX, invScaleY, invScaleZ);

    shu::vec3f invScale = invRotation * invScaleXYZ;

    return invScale;
}

shu::vec3f
shoora_body::LocalToWorldSpace(const shu::vec3f &PointLS) const
{
    // NOTE: This is the 2d version of the model matrix we calculate using Shu::TRS()
    shu::vec3f Scaled = this->Scale * PointLS;
    shu::vec3f Rotated = shu::QuatRotateVec(this->Rotation, Scaled);
    shu::vec3f Translated = Rotated + this->Position;

    return Translated;
}

shu::mat3f
shoora_body::GetInverseInertiaTensorWS() const
{
    shu::mat3f Result;

    shu::mat3f RotationMatrix = Rotation.ToMat3f();
    Result = ((RotationMatrix*InverseInertiaTensor)*RotationMatrix.Transposed());

    return Result;
}

shu::vec3f
shoora_body::GetCenterOfMassLS() const
{
    shu::vec3f Result = this->Shape->GetCenterOfMass();
    return Result;
}

shu::vec3f
shoora_body::GetCenterOfMassWS() const
{
    auto CenterLS = this->Shape->GetCenterOfMass();
    CenterLS *= this->Scale;
    shu::vec3f Result = this->Position + shu::QuatRotateVec(this->Rotation, CenterLS);
    return Result;
}

b32
shoora_body::CheckIfClicked(const shu::vec2f &ClickedWorldPos)
{
    b32 Result = false;
    if (this->Shape->Type == shoora_mesh_type::CIRCLE)
    {
        shu::vec2f l = ClickedWorldPos - shu::ToVec2(this->Position);
        u32 Radius = this->Shape->GetDim().x;
        Result = (l.SqMagnitude() < (Radius*Radius));
    }
    else if(this->Shape->Type == RECT_2D)
    {
        shu::rect2d rect = shu::rect2d(Position.x, Position.y, this->Shape->GetDim().x, this->Shape->GetDim().y);
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
    b32 Result = (SHU_ABSOLUTE(this->InvMass - 0.0f) < FLT_EPSILON);
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
shoora_body::ApplyImpulseLinear(const shu::vec3f &LinearImpulse)
{
    if(this->IsStatic()) {
        return;
    }

    this->LinearVelocity += LinearImpulse * this->InvMass;
}

void
shoora_body::ApplyImpulseAngular(const shu::vec3f &AngularImpulse)
{
    if(this->IsStatic()) {
        return;
    }

    // NOTE: This Impulse is in WS, so we need the inertia tensor also in WS.
    this->AngularVelocity += AngularImpulse * this->GetInverseInertiaTensorWS();

    // NOTE: Clamping the angular velocity due to performance issues if not done.
    const f32 MaxAngularSpeed = 30.0f;
    if(this->AngularVelocity.SqMagnitude() > MaxAngularSpeed*MaxAngularSpeed)
    {
        this->AngularVelocity.Normalize();
        this->AngularVelocity *= MaxAngularSpeed;
    }
}

void
shoora_body::ApplyImpulseAtPoint(const shu::vec3f &Impulse, const shu::vec3f &ImpulsePointWS)
{
    if(this->IsStatic()) { return; }

    ApplyImpulseLinear(Impulse);

    // NOTE: Impulse is change in momentum
    // L = I*w = r X p (Angular momentum = Inertia * angular velocity = distVector Cross linear momentum)
    // dL = I*dw = r X J(impulse that was passed here) - [Change in angular momentum / Angular Impulse = Inertia times change in angular velocity].
    // dw = inverseInertia * (r X J) - Here inverseInertia is a tensor matrix in World Space.
    shu::vec3f Center = GetCenterOfMassWS();
    shu::vec3f R = ImpulsePointWS - Center;
    shu::vec3f AngularImpulse = R.Cross(Impulse);

    ApplyImpulseAngular(AngularImpulse);
}

void
shoora_body::AddForce(const shu::vec3f &Force)
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
    this->SumForces = shu::Vec3f(0.0f);
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
    this->LinearVelocity += this->Acceleration * deltaTime;

    // alpha = Tau / Moment of Inertia
#if 0
    this->AngularAcceleration = this->SumTorques * InvI;
#endif

    ClearForces();
    ClearTorques();
}

void
shoora_body::Update(const f32 deltaTime)
{
    if(IsStatic()) {
        return;
    }

    this->Position += this->LinearVelocity * deltaTime;

    // NOTE: The Angular Velocity stored in this body is around its center of mass which may not be its Position.
    shu::vec3f CenterOfMassWS = GetCenterOfMassWS();
    shu::vec3f CMToPos = this->Position - CenterOfMassWS;

    // NOTE: This is the internal torque caused by precession(The Tennis Racket Problem/Intermediate Axes theorem).
    // IMPORTANT: NOTE: Did not understand this. Research more on this.
    // T(Torque) = I*Alpha = w X (I*w)
    shu::mat3f RotationMatrix = Rotation.ToMat3f();
    shu::mat3f InertiaTensorWS = ((RotationMatrix * this->InertiaTensor) * RotationMatrix.Transposed());
    shu::vec3f w = this->AngularVelocity;
    shu::vec3f InternalTorqueVector = (w.Cross(w * InertiaTensorWS));
    // IMPORTANT: NOTE: This torque formula is given here. The cross product gives this.
    // Taken from: https://en.wikipedia.org/wiki/Tennis_racket_theorem#Theory
#if 0
    InternalTorqueVector.x = -(InertiaTensorWS.m22 - InertiaTensorWS.m11)*w.z*w.y;
    InternalTorqueVector.y = -(InertiaTensorWS.m00 - InertiaTensorWS.m22)*w.x*w.z;
    InternalTorqueVector.z = -(InertiaTensorWS.m11 - InertiaTensorWS.m00)*w.x*w.y;
#endif
    shu::vec3f Alpha = InternalTorqueVector*InertiaTensorWS.Inverse();
    this->AngularVelocity += Alpha * deltaTime;

    // Update Rotation Quaternion
    shu::vec3f dAngle = this->AngularVelocity * deltaTime;
    shu::quat dq = shu::QuatAngleAxisDeg(dAngle.Magnitude()*RAD_TO_DEG, this->AngularVelocity);
    this->Rotation = (dq * Rotation);
    shu::QuatNormalize(this->Rotation);

    // NOTE: We are doing this because the quaternion(rotation) is around the center of mass not the position of
    // the body. This will handle cases where the center of mass of the body is not the same as its position.
    // Also, we are multiplying the change in orientation dq not the whole orientation Q(this->Rotation).
    this->Position = CenterOfMassWS + shu::QuatRotateVec(dq, CMToPos);
}

void
shoora_body::KeepInView(const shu::rect2d &ViewBounds, f32 DampFactor)
{
    shu::vec2f boundX = shu::Vec2f(ViewBounds.x - ViewBounds.width / 2, ViewBounds.x + ViewBounds.width / 2);
    shu::vec2f boundY = shu::Vec2f(ViewBounds.y - ViewBounds.height / 2, ViewBounds.y + ViewBounds.height / 2);

    shu::vec2f Dim = this->Shape->GetDim().xy;
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
shoora_body::DrawWireframe(const shu::mat4f &model, f32 thickness, u32 color)
{
    shoora_mesh_filter *mesh = this->Shape->MeshFilter;
    shoora_mesh_type Type = this->Shape->Type;

    if(Type == shoora_mesh_type::CIRCLE)
    {
        for (i32 i = 1; i < mesh->VertexCount; ++i)
        {
            shu::vec3f pos0 = mesh->Vertices[i - 1].Pos;
            shu::vec3f pos1 = mesh->Vertices[i].Pos;

            shu::vec2f p0 = (model * pos0).xy;
            shu::vec2f p1 = (model * pos1).xy;
            shoora_graphics::DrawLine2D(p0, p1, color, 2.5f);
        }

        shu::vec2f p0 = (model * mesh->Vertices[mesh->VertexCount - 1].Pos).xy;
        shu::vec2f p1 = (model * mesh->Vertices[1].Pos).xy;
        shoora_graphics::DrawLine2D(p0, p1, color, thickness);
    }
    else if (Type == shoora_mesh_type::POLYGON_2D || Type == shoora_mesh_type::RECT_2D)
    {
        ASSERT(mesh->VertexCount >= 3);
        auto *Poly = (shoora_shape_polygon *)this->Shape;
        auto *WorldVertices = Poly->WorldVertices;

        // NOTE: No need to calculate TRS here since that's already been done in the physics loop for polygons.
        for (i32 i = 0; i < mesh->VertexCount; ++i)
        {
            shu::vec2f p0 = WorldVertices[i].xy;
            shu::vec2f p1 = WorldVertices[(i+1) % mesh->VertexCount].xy;
            shoora_graphics::DrawLine2D(p0, p1, color, thickness);
        }
    }
}

void
shoora_body::Draw()
{
    shoora_mesh *mesh = (shoora_mesh *)this->Shape->MeshFilter;
    if (mesh != nullptr)
    {
        shoora_mesh_info Info = mesh->GetInfo();
        shoora_graphics::Draw(Info.IndexCount, Info.IndexOffset, Info.VertexOffset);
    }
}
