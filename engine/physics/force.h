#if !defined(FORCE_H)

#include <defines.h>
#include <math/math.h>
#include "particle.h"

struct force
{
    static Shu::vec2f GenerateDragForce(const shoora_particle *Particle, f32 DragCoefficient);
    static Shu::vec2f GenerateFrictionForce(const shoora_particle *Particle, f32 FrictionCoefficient);
};

#define FORCE_H
#endif // FORCE_H