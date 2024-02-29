#include "broadphase.h"
#include <math/math.h>
#include <utils/sort/sort.h>

b32
ComparePseudoBodies(const pseudo_body &A, const pseudo_body &B)
{
    b32 Result = (A.Value <= B.Value);
    return Result;
}

// NOTE: Sort the bodies based on their bounds along a given axis.
// WE can't for example, choose the z-axis since that will not be of any use. every rigidbody in the same zPos, not
// colliding, will still be considered as colliding, and we will get the worst case and not limit the number of
// collisions in the broadphase which defeats the purpose of doing broadphase collision detection.
void
SortBodiesBounds(const shoora_body *Bodies, const i32 BodyCount, pseudo_body *SortedArray, const f32 DeltaTime)
{
    shu::vec3f Axis = shu::Vec3f(1, 1, 1);
    Axis.Normalize();

    for (i32 i = 0; i < BodyCount; ++i)
    {
        const shoora_body &Body = Bodies[i];
        shoora_bounds Bounds = Body.Shape->GetBounds(Body.Position, Body.Rotation);

        // Expand the bounds by the body's Linear Velocity otherwise the work on continuous collision detection
        // will be of no use.
        // We are doing this since CCD involves detecting collision inside the frame itself, so the bounds have to
        // take into account how much the bodies will move in the current frame. We cant consider they being at
        // rest for the current physics frame(tick). The movement of the body is its velocity multiplied by
        // deltaTime.
        Bounds.Expand(Bounds.Mins + Body.LinearVelocity*DeltaTime);
        Bounds.Expand(Bounds.Maxs + Body.LinearVelocity*DeltaTime);

        // Adding some kind of epsilon/small value to account for rounding errors in floats.
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
BuildPairs(shoora_dynamic_array<collision_pair> &CollisionPairs, const pseudo_body *SortedPseudoBodies,
           const i32 SortedPseudoBodyCount)
{
    CollisionPairs.Clear();

    // At this point, the bodies should be sorted. We build collision Pairs now.
    for(i32 i = 0; i < SortedPseudoBodyCount; ++i)
    {
        const pseudo_body &A = SortedPseudoBodies[i];
        if(!A.IsMin) { continue; }

        collision_pair Pair;
        Pair.A = A.Id;

        // NOTE:
        // What we are doing here is - we add a pair to collisionPairs if for the current body A, we go through
        // the list again, adding bodies to it in a separate pair till we see it again(its maxs). all the bodies
        // that we added to a pair with A(the current body) came before A's max, meaning they are inside A's bound,
        // so it is a potential collision since their bounds are overlapping.
        for (i32 j = (i+1); j < SortedPseudoBodyCount; ++j)
        {
            const pseudo_body &B = SortedPseudoBodies[j];
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
    pseudo_body *SortedPseudoBodies = (pseudo_body *)_alloca(sizeof(pseudo_body) * BodyCount * 2);

    SortBodiesBounds(Bodies, BodyCount, SortedPseudoBodies, DeltaTime);

    // SortedArray of Pseudobodies is twice the number of bodies passed in here. Since, you have two entries for
    // each body. One is the dot product of its Bounds.Min with Axis, the other is the dot product of its
    // Bounds.Max with chosen Axis.
    i32 SortedPseudoBodyCount = BodyCount * 2;
    BuildPairs(FinalPairs, SortedPseudoBodies, SortedPseudoBodyCount);
}

void
broad_phase::BroadPhase(const shoora_body *Bodies, const i32 BodyCount,
                        shoora_dynamic_array<collision_pair> &FinalPairs, const f32 deltaTime)
{
    FinalPairs.Clear();
    SweepAndPrune1D(Bodies, BodyCount, FinalPairs, deltaTime);
}
