#include "broadphase.h"
#include <math/math.h>
#include <utils/sort/sort.h>

b32
ComparePseudoBodies(const pseudo_body &A, const pseudo_body &B)
{
    b32 Result = (A.Value <= B.Value);
    return Result;
}

// Sort the bodies based on their bounds along a given axis.
void
SortBodiesBounds(const shoora_body *Bodies, const i32 BodyCount, pseudo_body *SortedArray, const f32 DeltaTime)
{
    Shu::vec3f Axis = Shu::Vec3f(1, 1, 1);
    Axis.Normalize();

    for (i32 i = 0; i < BodyCount; ++i)
    {
        const shoora_body &Body = Bodies[i];
        shoora_bounds Bounds = Body.Shape->GetBounds(Body.Position, Body.Rotation);

        // Expand the bounds by the body's Linear Velocity.
        Bounds.Expand(Bounds.Mins + Body.LinearVelocity*DeltaTime);
        Bounds.Expand(Bounds.Maxs + Body.LinearVelocity*DeltaTime);

        f32 Epsilon = 0.01f;
        Bounds.Expand(Bounds.Mins - Axis*Epsilon);
        Bounds.Expand(Bounds.Mins + Axis*Epsilon);
        
        SortedArray[i*2 + 0].Id = i;
        SortedArray[i*2 + 0].Value = Axis.Dot(Bounds.Mins);
        SortedArray[i*2 + 0].IsMin = true;

        SortedArray[i*2 + 1].Id = i;
        SortedArray[i*2 + 1].Value = Axis.Dot(Bounds.Maxs);
        SortedArray[i*2 + 1].IsMin = false;
    }

    QuicksortRecursive(SortedArray, 0, BodyCount*2, ComparePseudoBodies);
}

void
BuildPairs(shoora_dynamic_array<collision_pair> &CollisionPairs, const pseudo_body *SortedBodies,
           const i32 SortedBodyCount)
{
    CollisionPairs.Clear();

    // At this point, the bodies should be sorted. We build collision Pairs now.
    for(i32 i = 0; i < (SortedBodyCount*2); ++i)
    {
        const pseudo_body &A = SortedBodies[i];
        if(!A.IsMin) { continue; }

        collision_pair Pair;
        Pair.A = A.Id;

        for (i32 j = (i+1); j < (SortedBodyCount*2); ++j)
        {
            const pseudo_body &B = SortedBodies[j];
            // If we have hit the end of the a element,then we are done creating pairs with a.
            if (B.Id == A.Id) { break; }
            if (!B.IsMin) { continue; }

            Pair.B = B.Id;
            CollisionPairs.push_back(Pair);
        }
    }
}

void
SweepAndPrune1D(const shoora_body *Bodies, const i32 BodyCount, shoora_dynamic_array<collision_pair> &FinalPairs,
                const f32 DeltaTime)
{
    pseudo_body *SortedBodies = (pseudo_body *)_alloca(sizeof(pseudo_body) * BodyCount * 2);

    SortBodiesBounds(Bodies, BodyCount, SortedBodies, DeltaTime);
    BuildPairs(FinalPairs, SortedBodies, BodyCount);
}

void
broad_phase::BroadPhase(const shoora_body *Bodies, const i32 BodyCount,
                        shoora_dynamic_array<collision_pair> &FinalPairs, const f32 deltaTime)
{
    FinalPairs.Clear();
    SweepAndPrune1D(Bodies, BodyCount, FinalPairs, deltaTime);
}
