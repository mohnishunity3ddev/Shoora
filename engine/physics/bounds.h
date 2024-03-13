#if !defined(BOUNDS_H)

#include <defines.h>
#include <math/math.h>

struct shoora_bounds
{
    shoora_bounds() { this->Clear(); }
    shoora_bounds(const shoora_bounds &other) : Mins(other.Mins), Maxs(other.Maxs) {}
    shoora_bounds(shu::vec3f Min, shu::vec3f Max) : Mins(Min), Maxs(Max) {}
    const shoora_bounds &operator=(const shoora_bounds &other);
    ~shoora_bounds() = default;

    void Clear();
    b32 DoesIntersect(const shoora_bounds &other) const;
    void Expand(const shu::vec3f *Pts, const i32 Num);
    void Expand(const shu::vec3f &V);
    void Expand(const shoora_bounds &Bounds);

    f32 WidthX() const { f32 Result = Maxs.x - Mins.x; return Result; }
    f32 WidthY() const { f32 Result = Maxs.y - Mins.y; return Result; }
    f32 WidthZ() const { f32 Result = Maxs.z - Mins.z; return Result; }

    void Draw();

    shu::vec3f Mins;
    shu::vec3f Maxs;
};

#define BOUNDS_H
#endif // BOUNDS_H