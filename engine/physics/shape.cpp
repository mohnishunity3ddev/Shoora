#include "shape.h"

shoora_shape::shoora_shape(shoora_primitive_type Type)
{
    this->Type = Type;
    this->Primitive = shoora_primitive_collection::GetPrimitive(Type);
}

void
shoora_shape::Draw(const VkCommandBuffer &cmdBuffer)
{
    this->Primitive->Draw(cmdBuffer);
}

shoora_shape_circle::shoora_shape_circle(u32 Radius)
    : shoora_shape(shoora_primitive_type::CIRCLE)
{

    this->Radius = Radius;
}

f32
shoora_shape_circle::GetMomentOfInertia() const
{
    f32 Result = 0.5f * this->Radius * this->Radius;
    return Result;
}

Shu::vec3f
shoora_shape_circle::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Radius, this->Radius, 1.0f);
    return Result;
}

shoora_shape_box::shoora_shape_box(u32 Width, u32 Height)
    : shoora_shape(shoora_primitive_type::RECT_2D)

{
    this->Width = Width;
    this->Height = Height;
}

f32
shoora_shape_box::GetMomentOfInertia() const
{
    f32 Result = (this->Width*this->Width + this->Height*this->Height) / 12.0f;
    return Result;
}

Shu::vec3f
shoora_shape_box::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Width, this->Height, 1.0f);
    return Result;
}

shoora_shape_triangle::shoora_shape_triangle(u32 Base, u32 Height)
    : shoora_shape(shoora_primitive_type::TRIANGLE)
{
    this->Base = Base;
    this->Height = Height;
}

f32
shoora_shape_triangle::GetMomentOfInertia() const
{
    f32 b = this->Base;
    f32 h = this->Height;
    f32 Result = (b * (h*h*h)) / 36.0f;
    return Result;
}

Shu::vec3f
shoora_shape_triangle::GetDim() const
{
    Shu::vec3f Result = Shu::Vec3f(this->Base, this->Height, 1.0f);
    return Result;
}