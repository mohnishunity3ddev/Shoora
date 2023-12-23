#if !defined(PARTICLE_H)

#include "defines.h"
#include <math/math.h>
#include <mesh/primitive/geometry_primitive.h>
#include <mesh/mesh_filter.h>

struct shoora_particle
{
    Shu::vec3f Position;
    f32 Size;

    Shu::vec3f Velocity;
    Shu::vec3f Acceleration;

    shoora_mesh_filter *MeshFilter;
    shoora_primitive_type PrimitiveType;

    f32 Mass;
};

#define PARTICLE_H
#endif // PARTICLE_H