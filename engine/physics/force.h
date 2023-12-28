#if !defined(FORCE_H)

#include <defines.h>
#include <math/math.h>
#include "body.h"

struct force
{
    static Shu::vec2f GenerateDragForce(const shoora_body *Body, f32 DragCoefficient);
    static Shu::vec2f GenerateFrictionForce(const shoora_body *Body, f32 FrictionCoefficient);
    static Shu::vec2f GenerateGravitationalForce(const shoora_body *A, const shoora_body *B, f32 G,
                                                 f32 minDistance, f32 maxDistance);
    // Anchor: anchor position where the spring is attached to a wall or where the spring starts basically.
    // RestLength: The length from the anchor where the body will be in equilibirum with the spring.
    // k: Spring constant.
    static Shu::vec2f GenerateSpringForce(const shoora_body *p, const Shu::vec2f &Anchor, f32 RestLength,
                                          f32 k);
    static Shu::vec2f GenerateSpringForce(const shoora_body *pA, const shoora_body *pB, f32 RestLength,
                                          f32 k);
};

#define FORCE_H
#endif // FORCE_H