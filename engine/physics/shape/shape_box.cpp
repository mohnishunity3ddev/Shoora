#include "shape.h"

// NOTE: Box Stuff
shoora_shape_box::shoora_shape_box(u32 Width, u32 Height) : shoora_shape_polygon(shoora_mesh_type::RECT_2D)
{
    this->Width = Width;
    this->Height = Height;
    this->mCenterOfMass = shu::Vec3f(0.0f);
}

shoora_shape_box::~shoora_shape_box() { LogUnformatted("shoora_shape_box destructor called!\n"); }

shu::mat3f
shoora_shape_box::InertiaTensor() const
{
    shu::mat3f Result = shu::Mat3f((this->Width * this->Width + this->Height * this->Height) / 12.0f);
    return Result;
}

shu::vec3f
shoora_shape_box::GetDim() const
{
    shu::vec3f Result = shu::Vec3f(this->Width, this->Height, 1.0f);
    return Result;
}

shoora_mesh_type
shoora_shape_box::GetType() const
{
    return shoora_mesh_type::RECT_2D;
}

shoora_bounds
shoora_shape_box::GetBounds(const shu::vec3f &Pos, const shu::quat &Orientation) const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shoora_bounds
shoora_shape_box::GetBounds() const
{
    // TODO)): Not implemented yet.
    shoora_bounds Result;
    return Result;
}

shu::vec3f
shoora_shape_box::Support(const shu::vec3f &Direction, const shu::vec3f &Position, const shu::quat &Orientation,
                          const f32 Bias) const
{
    // TODO): TO BE IMPLEMENTED
    shu::vec3f Result;
    return Result;
}