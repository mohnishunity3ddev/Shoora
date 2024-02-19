#include "bounds.h"

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
    this->Mins = Shu::Vec3f(SHU_FLOAT_MAX);
    this->Maxs = Shu::Vec3f(SHU_FLOAT_MIN);
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
shoora_bounds::Expand(const Shu::vec3f *Pts, const i32 Num)
{
    for(i32 i = 0; i < Num; ++i)
    {
        this->Expand(Pts[i]);
    }
}

void
shoora_bounds::Expand(const Shu::vec3f &V)
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