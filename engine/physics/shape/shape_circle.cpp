#include "shape.h"

// NOTE: Circle
shoora_shape_circle::shoora_shape_circle(f32 Radius) : shoora_shape(shoora_mesh_type::CIRCLE)
{
    this->Radius = Radius;
    this->mCenterOfMass = shu::Vec3f(0.0f);
}

shoora_shape_circle::~shoora_shape_circle() { LogUnformatted("shoora_shape_circle destructor called!\n"); }

shu::mat3f
shoora_shape_circle::InertiaTensor() const
{
    shu::mat3f Result = shu::Mat3f(0.5f * this->Radius * this->Radius);
    return Result;
}

shu::vec3f
shoora_shape_circle::GetDim() const
{
    shu::vec3f Result = shu::Vec3f(this->Radius, this->Radius, 1.0f);
    return Result;
}

shoora_mesh_type
shoora_shape_circle::GetType() const
{
    return shoora_mesh_type::CIRCLE;
}

shoora_bounds
shoora_shape_circle::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shoora_bounds
shoora_shape_circle::GetBounds() const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shu::vec3f
shoora_shape_circle::Support(const shu::vec3f &Direction, const shu::vec3f &Position, const shu::quat &Orientation,
                             const f32 Bias) const
{
    // TODO): TO BE IMPLEMENTED
    shu::vec3f Result;
    return Result;
}
