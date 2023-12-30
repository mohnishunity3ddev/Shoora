#if !defined(BODY_H)

#include "defines.h"
#include <math/math.h>
#include <mesh/primitive/geometry_primitive.h>
#include <mesh/mesh_filter.h>
#include <physics/shape.h>

struct shoora_body
{
    Shu::vec3f Position;
    Shu::vec3f Scale;

    Shu::vec3f Color;

    shoora_shape *Shape;

    Shu::vec3f Velocity;
    Shu::vec3f Acceleration;
    f32 Mass;
    f32 InvMass;
    Shu::vec3f SumForces;

    // Angular motion
    f32 Rotation;
    f32 AngularVelocity;
    f32 AngularAcceleration;
    f32 InvI;
    Shu::vec3f SumTorques;

    b32 CheckIfClicked(const Shu::vec2f &ClickedWorldPos);
    void KeepInView(const Shu::rect2d &ViewBounds, f32 DampFactor);

    void Initialize(const Shu::vec3f &Color, const Shu::vec2f &InitPos, f32 Mass, shoora_shape *Shape);
    void IntegrateLinear(f32 DeltaTime);
    void AddForce(const Shu::vec2f &Force);
    void ClearForces();

    void IntegrateAngular(f32 DeltaTime);
    void ClearTorques();
};

#define BODY_H
#endif // BODY_H