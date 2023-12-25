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
    Shu::vec3f Color;

    shoora_mesh_filter *MeshFilter;
    shoora_primitive_type PrimitiveType;

    f32 Mass;
    f32 InvMass;
    Shu::vec3f SumForces;

    void Initialize(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Size, f32 Mass,
                    const shoora_primitive_type Type, shoora_mesh_filter *MeshFilter);
    void Integrate(f32 DeltaTime);
    void AddForce(const Shu::vec2f &Force);
    void ClearForces();
};

#define PARTICLE_H
#endif // PARTICLE_H