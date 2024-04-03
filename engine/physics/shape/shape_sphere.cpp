#include "shape.h"

// NOTE: Sphere
shoora_shape_sphere::shoora_shape_sphere(f32 Radius) : shoora_shape(shoora_mesh_type::SPHERE)
{
    this->Radius = Radius;
    this->mCenterOfMass = shu::Vec3f(0.0f);
}

shoora_shape_sphere::~shoora_shape_sphere() { LogUnformatted("shoora_shape_sphere destructor called!\n"); }

shu::mat3f
shoora_shape_sphere::InertiaTensor() const
{
    shu::mat3f Result = shu::Mat3f(0.4f * this->Radius * this->Radius);
    return Result;
}

shu::vec3f
shoora_shape_sphere::GetDim() const
{
    shu::vec3f Result = shu::Vec3f(this->Radius, this->Radius, this->Radius);
    return Result;
}

shoora_mesh_type
shoora_shape_sphere::GetType() const
{
    return shoora_mesh_type::SPHERE;
}

shoora_bounds
shoora_shape_sphere::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    shoora_bounds Result;
    Result.Mins = Pos - shu::Vec3f(this->Radius);
    Result.Maxs = Pos + shu::Vec3f(this->Radius);
    return Result;
}

shoora_bounds
shoora_shape_sphere::GetBounds() const
{
    shoora_bounds Result;
    Result.Mins = shu::Vec3f(-this->Radius);
    Result.Maxs = shu::Vec3f(this->Radius);
    return Result;
}

shu::vec3f
shoora_shape_sphere::SupportPtWorldSpace(const shu::vec3f &Direction, const shu::vec3f &Position,
                             const shu::quat &Orientation, const f32 Bias) const
{
    shu::vec3f n = shu::Normalize(Direction);

    shu::vec3f SupportPoint = Position + n * (this->Radius + Bias);

    return SupportPoint;
}
