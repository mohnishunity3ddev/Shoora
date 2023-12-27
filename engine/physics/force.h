#if !defined(FORCE_H)

#include <defines.h>
#include <math/math.h>
#include "particle.h"

struct force
{
    static Shu::vec2f GenerateDragForce(const shoora_particle *Particle, f32 DragCoefficient);
    static Shu::vec2f GenerateFrictionForce(const shoora_particle *Particle, f32 FrictionCoefficient);
    static Shu::vec2f GenerateGravitationalForce(const shoora_particle *A, const shoora_particle *B, f32 G,
                                                 f32 minDistance, f32 maxDistance);
    // Anchor: anchor position where the spring is attached to a wall or where the spring starts basically.
    // RestLength: The length from the anchor where the particle will be in equilibirum with the spring.
    // k: Spring constant.
    static Shu::vec2f GenerateSpringForce(const shoora_particle *p, const Shu::vec2f &Anchor, f32 RestLength,
                                          f32 k);
    static Shu::vec2f GenerateSpringForce(const shoora_particle *pA, const shoora_particle *pB, f32 RestLength,
                                          f32 k);
};

#define FORCE_H
#endif // FORCE_H