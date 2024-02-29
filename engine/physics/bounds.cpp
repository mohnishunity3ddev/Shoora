#include "bounds.h"
#include <renderer/vulkan/graphics/vulkan_graphics.h>
#include <utils/utils.h>

const shoora_bounds &
shoora_bounds::operator=(const shoora_bounds &other)
{
    this->Mins = other.Mins;
    this->Maxs = other.Maxs;
    return *this;
}

void
shoora_bounds::Clear()
{
    this->Mins = shu::Vec3f(SHU_FLOAT_MAX);
    this->Maxs = shu::Vec3f(SHU_FLOAT_MIN);
}

b32
shoora_bounds::DoesIntersect(const shoora_bounds &other) const
{
    b32 Result = true;
    if((this->Maxs.x < other.Mins.x) || (this->Maxs.y < other.Mins.y) || (this->Maxs.z < other.Mins.z)) {
        Result = false;
    }
    else if((other.Maxs.x < this->Mins.x) || (other.Maxs.y < this->Mins.y) || (other.Maxs.z < this->Mins.z)) {
        Result = false;
    }

    return Result;
}

void
shoora_bounds::Expand(const shu::vec3f *Pts, const i32 Num)
{
    for(i32 i = 0; i < Num; ++i)
    {
        this->Expand(Pts[i]);
    }
}

void
shoora_bounds::Expand(const shu::vec3f &V)
{
    if(V.x < this->Mins.x) { this->Mins.x = V.x; }
    if(V.y < this->Mins.y) { this->Mins.y = V.y; }
    if(V.z < this->Mins.z) { this->Mins.z = V.z; }

    if(V.x > this->Maxs.x) { this->Maxs.x = V.x; }
    if(V.y > this->Maxs.y) { this->Maxs.y = V.y; }
    if(V.z > this->Maxs.z) { this->Maxs.z = V.z; }
}

void
shoora_bounds::Expand(const shoora_bounds &Bounds)
{
    this->Expand(Bounds.Mins);
    this->Expand(Bounds.Maxs);
}

void
shoora_bounds::Draw()
{
    shu::vec3f v000 = shu::Vec3f(Mins.x, Mins.y, Mins.z);
    shu::vec3f v100 = shu::Vec3f(Mins.x + WidthX(), Mins.y, Mins.z);
    shu::vec3f v110 = shu::Vec3f(Mins.x + WidthX(), Mins.y + WidthY(), Mins.z);
    shu::vec3f v010 = shu::Vec3f(Mins.x, Mins.y + WidthY(), Mins.z);
    shu::vec3f v001 = shu::Vec3f(Mins.x, Mins.y, Maxs.z);
    shu::vec3f v101 = shu::Vec3f(Maxs.x, Mins.y, Maxs.z);
    shu::vec3f v111 = shu::Vec3f(Maxs.x, Maxs.y, Maxs.z);
    shu::vec3f v011 = shu::Vec3f(Mins.x, Maxs.y, Maxs.z);
    shoora_graphics::DrawCubeWireframe(v000, v100, v110, v010, v001, v101, v111, v011, colorU32::Green, .05f);
}
