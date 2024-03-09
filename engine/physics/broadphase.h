#if !defined(BROADPHASE_H)

#include <defines.h>
#include <containers/dynamic_array.h>
#include "body.h"

struct pseudo_body
{
    i32 Id;
    f32 Value;
    b32 IsMin;
};

struct collision_pair
{
    i32 A;
    i32 B;

    b32
    operator==(const collision_pair &Rhs) const
    {
        b32 Result = (((A == Rhs.A) && (B == Rhs.B)) || ((A == Rhs.B) && (B == Rhs.A)));
        return Result;
    }

    b32
    operator!=(const collision_pair &Rhs) const
    {
        b32 Result = !(*this == Rhs);
        return Result;
    }
};

struct broad_phase
{
    broad_phase() = delete;
    static void BroadPhase(const shoora_body *Bodies, const i32 BodyCount, collision_pair *FinalPairs,
                           i32 &PairCount, const f32 deltaTime);
};

#define BROADPHASE_H
#endif // BROADPHASE_H